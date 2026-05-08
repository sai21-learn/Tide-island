#include "WifiController.h"
#include "NetworkManagerBackend.h"
#include "IwdBackend.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>

WifiController::WifiController(QObject *parent)
    : QObject(parent) {
    detectBackend();
}

QString WifiController::backendName() const {
    return m_backend ? m_backend->name() : "unsupported";
}

bool WifiController::supported() const {
    return m_backend != nullptr;
}

bool WifiController::readOnly() const {
    return false;
}

bool WifiController::available() const {
    return m_backend && m_backend->isAvailable();
}

bool WifiController::enabled() const {
    return m_backend && m_backend->isEnabled();
}

bool WifiController::busy() const {
    return m_backend && m_backend->isBusy();
}

bool WifiController::scanning() const {
    return m_backend && m_backend->isScanning();
}

QString WifiController::currentSsid() const {
    return m_backend ? m_backend->currentSsid() : "";
}

QString WifiController::statusText() const {
    return m_backend ? m_backend->statusText() : "Unavailable";
}

QString WifiController::infoMessage() const {
    return m_infoMessage;
}

QString WifiController::errorMessage() const {
    return m_errorMessage;
}

QString WifiController::unsupportedReason() const {
    return m_backend ? "" : "No supported Wi-Fi backend (NetworkManager or iwd) found.";
}

QAbstractItemModel *WifiController::networks() {
    return &m_networks;
}

void WifiController::refreshState() {
    if (m_backend) m_backend->scan();
}

void WifiController::refreshNetworks(bool rescan) {
    if (m_backend && rescan) m_backend->scan();
}

void WifiController::setEnabled(bool enabled) {
    if (m_backend) m_backend->setEnabled(enabled);
}

void WifiController::disconnectCurrent() {
    if (m_backend) m_backend->disconnect();
}

void WifiController::connectToNetwork(const QString &ssid, const QString &password) {
    if (m_backend) m_backend->connectToNetwork(ssid, password);
}

void WifiController::clearMessages() {
    m_infoMessage = "";
    m_errorMessage = "";
    emit infoMessageChanged();
    emit errorMessageChanged();
}

void WifiController::detectBackend() {
    auto bus = QDBusConnection::systemBus();
    if (bus.interface()->isServiceRegistered("org.freedesktop.NetworkManager")) {
        setBackend(new NetworkManagerBackend(&m_networks, this));
    } else if (bus.interface()->isServiceRegistered("net.connman.iwd")) {
        setBackend(new IwdBackend(&m_networks, this));
    }
}

void WifiController::setBackend(IWifiBackend *backend) {
    if (m_backend) {
        QObject::disconnect(m_backend, nullptr, this, nullptr);
        m_backend->deleteLater();
    }
    m_backend = backend;
    if (!m_backend) return;

    connect(m_backend, &IWifiBackend::availableChanged, this, &WifiController::availableChanged);
    connect(m_backend, &IWifiBackend::enabledChanged, this, &WifiController::enabledChanged);
    connect(m_backend, &IWifiBackend::busyChanged, this, &WifiController::busyChanged);
    connect(m_backend, &IWifiBackend::scanningChanged, this, &WifiController::scanningChanged);
    connect(m_backend, &IWifiBackend::currentSsidChanged, this, &WifiController::currentSsidChanged);
    connect(m_backend, &IWifiBackend::statusTextChanged, this, &WifiController::statusTextChanged);
    connect(m_backend, &IWifiBackend::infoMessage, this, [this](const QString &msg) {
        m_infoMessage = msg;
        emit infoMessageChanged();
    });
    connect(m_backend, &IWifiBackend::errorMessage, this, [this](const QString &msg) {
        m_errorMessage = msg;
        emit errorMessageChanged();
    });

    emit backendNameChanged();
    emit supportedChanged();
    emit availableChanged();
    emit enabledChanged();
    emit currentSsidChanged();
    emit statusTextChanged();
}
