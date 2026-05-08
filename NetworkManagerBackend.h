#pragma once

#include "IWifiBackend.h"
#include "WifiNetworkModel.h"
#include <QDBusObjectPath>
#include <QMap>
#include <QHash>
#include <QTimer>

class NetworkManagerBackend : public IWifiBackend {
    Q_OBJECT

public:
    explicit NetworkManagerBackend(WifiNetworkModel *model, QObject *parent = nullptr);
    ~NetworkManagerBackend() override;

    QString name() const override { return "NetworkManager"; }
    bool isAvailable() const override;
    bool isEnabled() const override;
    bool isBusy() const override;
    bool isScanning() const override;
    QString currentSsid() const override;
    QString statusText() const override;

    void setEnabled(bool enabled) override;
    void scan() override;
    void disconnect() override;
    void connectToNetwork(const QString &ssid, const QString &password) override;

private slots:
    void handlePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void handleDevicePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void handleAccessPointAdded(const QDBusObjectPath &apPath);
    void handleAccessPointRemoved(const QDBusObjectPath &apPath);
    void refreshState();
    void refreshNetworks();

private:
    void setupManager();
    void setupDevice(const QString &path);
    void reloadSavedConnections();
    bool activateSavedConnection(const QString &ssid, const QString &apPath);
    bool addAndActivateConnection(const QString &ssid, const QString &apPath, const QString &password, bool secure);

    WifiNetworkModel *m_model;
    QString m_devicePath;
    QString m_activeApPath;
    QString m_currentSsid;
    bool m_available = false;
    bool m_enabled = false;
    bool m_busy = false;
    bool m_scanning = false;
    QHash<QString, QString> m_savedConnections;
    QTimer m_refreshTimer;
};
