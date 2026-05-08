#include "IwdBackend.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

namespace {
constexpr auto kService = "net.connman.iwd";
constexpr auto kPath = "/";
constexpr auto kObjectManagerInterface = "org.freedesktop.DBus.ObjectManager";
}

IwdBackend::IwdBackend(WifiNetworkModel *model, QObject *parent)
    : IWifiBackend(parent)
    , m_model(model) {
    setupIwd();
    QDBusConnection::systemBus().connect(kService, kPath, kObjectManagerInterface, "InterfacesAdded", this, SLOT(handleInterfacesAdded(QDBusObjectPath,QVariantMap)));
    QDBusConnection::systemBus().connect(kService, kPath, kObjectManagerInterface, "InterfacesRemoved", this, SLOT(handleInterfacesRemoved(QDBusObjectPath,QStringList)));
}

IwdBackend::~IwdBackend() {
}

bool IwdBackend::isAvailable() const { return m_available; }
bool IwdBackend::isEnabled() const { return m_enabled; }
bool IwdBackend::isBusy() const { return m_busy; }
bool IwdBackend::isScanning() const { return m_scanning; }
QString IwdBackend::currentSsid() const { return m_currentSsid; }
QString IwdBackend::statusText() const { return m_available ? (m_enabled ? (m_currentSsid.isEmpty() ? "Disconnected" : m_currentSsid) : "Disabled") : "Unavailable"; }

void IwdBackend::setEnabled(bool enabled) {
    if (m_devicePath.isEmpty()) return;
    QDBusInterface props(kService, m_devicePath, "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    props.call("Set", "net.connman.iwd.Device", "Powered", QVariant::fromValue(QDBusVariant(enabled)));
}

void IwdBackend::scan() {
    if (m_stationPath.isEmpty()) return;
    QDBusInterface iface(kService, m_stationPath, "net.connman.iwd.Station", QDBusConnection::systemBus());
    iface.call("Scan");
}

void IwdBackend::disconnect() {
    if (m_stationPath.isEmpty()) return;
    QDBusInterface iface(kService, m_stationPath, "net.connman.iwd.Station", QDBusConnection::systemBus());
    iface.call("Disconnect");
}

void IwdBackend::connectToNetwork(const QString &ssid, const QString &password) {
    Q_UNUSED(ssid) Q_UNUSED(password)
    emit infoMessage("iwd connection not fully implemented in this refactor yet.");
}

void IwdBackend::setupIwd() {
    QDBusInterface iface(kService, kPath, kObjectManagerInterface, QDBusConnection::systemBus());
    m_available = iface.isValid();
    if (!m_available) return;
    
    refreshState();
}

void IwdBackend::handleInterfacesAdded(const QDBusObjectPath &path, const QVariantMap &interfaces) {
    Q_UNUSED(path) Q_UNUSED(interfaces)
    refreshState();
}

void IwdBackend::handleInterfacesRemoved(const QDBusObjectPath &path, const QStringList &interfaces) {
    Q_UNUSED(path) Q_UNUSED(interfaces)
    refreshState();
}

void IwdBackend::refreshState() {
    // Basic state discovery...
    emit statusTextChanged(statusText());
}
