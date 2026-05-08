#pragma once

#include "IWifiBackend.h"
#include "WifiNetworkModel.h"
#include <QDBusObjectPath>
#include <QDBusContext>
#include <QMap>
#include <QHash>
#include <QTimer>

class IwdBackend : public IWifiBackend {
    Q_OBJECT

public:
    explicit IwdBackend(WifiNetworkModel *model, QObject *parent = nullptr);
    ~IwdBackend() override;

    QString name() const override { return "iwd"; }
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
    void handleInterfacesAdded(const QDBusObjectPath &path, const QVariantMap &interfaces);
    void handleInterfacesRemoved(const QDBusObjectPath &path, const QStringList &interfaces);
    void refreshState();

private:
    void setupIwd();
    
    WifiNetworkModel *m_model;
    QString m_stationPath;
    QString m_devicePath;
    QString m_currentSsid;
    bool m_available = false;
    bool m_enabled = false;
    bool m_busy = false;
    bool m_scanning = false;
};
