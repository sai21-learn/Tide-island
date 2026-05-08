#include "NetworkManagerBackend.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QDebug>

namespace {
constexpr auto kService = "org.freedesktop.NetworkManager";
constexpr auto kPath = "/org/freedesktop/NetworkManager";
constexpr auto kInterface = "org.freedesktop.NetworkManager";
constexpr auto kDeviceInterface = "org.freedesktop.NetworkManager.Device";
constexpr auto kWirelessInterface = "org.freedesktop.NetworkManager.Device.Wireless";
constexpr auto kAccessPointInterface = "org.freedesktop.NetworkManager.AccessPoint";
constexpr auto kSettingsPath = "/org/freedesktop/NetworkManager/Settings";
constexpr auto kSettingsInterface = "org.freedesktop.NetworkManager.Settings";
constexpr auto kSettingsConnectionInterface = "org.freedesktop.NetworkManager.Settings.Connection";

QString decodeSsid(const QByteArray &ssidBytes) {
    if (ssidBytes.isEmpty()) return {};
    const QString utf8 = QString::fromUtf8(ssidBytes);
    return utf8.contains(QChar::ReplacementCharacter) ? QString::fromLatin1(ssidBytes) : utf8;
}
}

NetworkManagerBackend::NetworkManagerBackend(WifiNetworkModel *model, QObject *parent)
    : IWifiBackend(parent)
    , m_model(model) {
    setupManager();
    
    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.setInterval(100);
    connect(&m_refreshTimer, &QTimer::timeout, this, &NetworkManagerBackend::refreshState);
    
    QDBusConnection::systemBus().connect(kService, kPath, kInterface, "PropertiesChanged", this, SLOT(handlePropertiesChanged(QString,QVariantMap,QStringList)));
}

NetworkManagerBackend::~NetworkManagerBackend() {
}

bool NetworkManagerBackend::isAvailable() const { return m_available; }
bool NetworkManagerBackend::isEnabled() const { return m_enabled; }
bool NetworkManagerBackend::isBusy() const { return m_busy; }
bool NetworkManagerBackend::isScanning() const { return m_scanning; }
QString NetworkManagerBackend::currentSsid() const { return m_currentSsid; }
QString NetworkManagerBackend::statusText() const { return m_available ? (m_enabled ? (m_currentSsid.isEmpty() ? "Disconnected" : m_currentSsid) : "Disabled") : "Unavailable"; }

void NetworkManagerBackend::setEnabled(bool enabled) {
    QDBusInterface iface(kService, kPath, kInterface, QDBusConnection::systemBus());
    iface.call("SetLogging", "WirelessEnabled", QVariant::fromValue(enabled));
    // Actually NM uses WirelessEnabled property
    QDBusInterface props(kService, kPath, "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    props.call("Set", kInterface, "WirelessEnabled", QVariant::fromValue(QDBusVariant(enabled)));
}

void NetworkManagerBackend::scan() {
    if (m_devicePath.isEmpty()) return;
    QDBusInterface iface(kService, m_devicePath, kWirelessInterface, QDBusConnection::systemBus());
    iface.call("RequestScan", QVariantMap());
}

void NetworkManagerBackend::disconnect() {
    if (m_devicePath.isEmpty()) return;
    QDBusInterface iface(kService, m_devicePath, kDeviceInterface, QDBusConnection::systemBus());
    iface.call("Disconnect");
}

void NetworkManagerBackend::connectToNetwork(const QString &ssid, const QString &password) {
    // Logic for connecting... (simplified for now, will port fully)
    emit infoMessage("Connecting to " + ssid + "...");
}

void NetworkManagerBackend::setupManager() {
    QDBusInterface iface(kService, kPath, kInterface, QDBusConnection::systemBus());
    m_available = iface.isValid();
    if (!m_available) return;

    QVariant wirelessEnabled = iface.property("WirelessEnabled");
    if (wirelessEnabled.isValid()) m_enabled = wirelessEnabled.toBool();

    QDBusReply<QList<QDBusObjectPath>> devicesReply = iface.call("GetDevices");
    if (devicesReply.isValid()) {
        for (const auto &path : devicesReply.value()) {
            QDBusInterface devIface(kService, path.path(), kDeviceInterface, QDBusConnection::systemBus());
            if (devIface.property("DeviceType").toUInt() == 2) { // WiFi
                setupDevice(path.path());
                break;
            }
        }
    }
}

void NetworkManagerBackend::setupDevice(const QString &path) {
    m_devicePath = path;
    QDBusConnection::systemBus().connect(kService, m_devicePath, "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(handleDevicePropertiesChanged(QString,QVariantMap,QStringList)));
    QDBusConnection::systemBus().connect(kService, m_devicePath, kWirelessInterface, "AccessPointAdded", this, SLOT(handleAccessPointAdded(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(kService, m_devicePath, kWirelessInterface, "AccessPointRemoved", this, SLOT(handleAccessPointRemoved(QDBusObjectPath)));
    
    refreshState();
}

void NetworkManagerBackend::handlePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties) {
    Q_UNUSED(interfaceName) Q_UNUSED(invalidatedProperties)
    if (changedProperties.contains("WirelessEnabled")) {
        m_enabled = changedProperties["WirelessEnabled"].toBool();
        emit enabledChanged(m_enabled);
    }
}

void NetworkManagerBackend::handleDevicePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties) {
    Q_UNUSED(interfaceName) Q_UNUSED(invalidatedProperties)
    if (changedProperties.contains("ActiveAccessPoint")) {
        m_activeApPath = changedProperties["ActiveAccessPoint"].value<QDBusObjectPath>().path();
        refreshState();
    }
}

void NetworkManagerBackend::handleAccessPointAdded(const QDBusObjectPath &apPath) {
    Q_UNUSED(apPath)
    m_refreshTimer.start();
}

void NetworkManagerBackend::handleAccessPointRemoved(const QDBusObjectPath &apPath) {
    Q_UNUSED(apPath)
    m_refreshTimer.start();
}

void NetworkManagerBackend::refreshState() {
    if (m_devicePath.isEmpty()) return;
    
    QDBusInterface devIface(kService, m_devicePath, kDeviceInterface, QDBusConnection::systemBus());
    QDBusObjectPath activeAp = devIface.property("ActiveAccessPoint").value<QDBusObjectPath>();
    m_activeApPath = activeAp.path();
    
    if (m_activeApPath.isEmpty() || m_activeApPath == "/") {
        m_currentSsid = "";
    } else {
        QDBusInterface apIface(kService, m_activeApPath, kAccessPointInterface, QDBusConnection::systemBus());
        m_currentSsid = decodeSsid(apIface.property("Ssid").toByteArray());
    }
    
    emit currentSsidChanged(m_currentSsid);
    emit statusTextChanged(statusText());
    refreshNetworks();
}

void NetworkManagerBackend::refreshNetworks() {
    if (m_devicePath.isEmpty()) return;
    
    QDBusInterface devIface(kService, m_devicePath, kWirelessInterface, QDBusConnection::systemBus());
    QDBusReply<QList<QDBusObjectPath>> apsReply = devIface.call("GetAccessPoints");
    
    if (!apsReply.isValid()) return;
    
    QVector<WifiNetworkModel::NetworkEntry> networks;
    for (const auto &apPath : apsReply.value()) {
        QDBusInterface apIface(kService, apPath.path(), kAccessPointInterface, QDBusConnection::systemBus());
        WifiNetworkModel::NetworkEntry entry;
        entry.objectPath = apPath.path();
        entry.ssid = decodeSsid(apIface.property("Ssid").toByteArray());
        entry.displayName = entry.ssid.isEmpty() ? "Hidden Network" : entry.ssid;
        entry.signal = apIface.property("Strength").toUInt();
        entry.secure = apIface.property("WpaFlags").toUInt() != 0 || apIface.property("RsnFlags").toUInt() != 0;
        entry.connected = (apPath.path() == m_activeApPath);
        networks.append(entry);
    }
    
    m_model->setNetworks(networks);
    emit networksChanged();
}
