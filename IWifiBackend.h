#pragma once

#include <QObject>
#include <QString>
#include <QAbstractItemModel>

class IWifiBackend : public QObject {
    Q_OBJECT

public:
    explicit IWifiBackend(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IWifiBackend() = default;

    virtual QString name() const = 0;
    virtual bool isAvailable() const = 0;
    virtual bool isEnabled() const = 0;
    virtual bool isBusy() const = 0;
    virtual bool isScanning() const = 0;
    virtual QString currentSsid() const = 0;
    virtual QString statusText() const = 0;

    virtual void setEnabled(bool enabled) = 0;
    virtual void scan() = 0;
    virtual void disconnect() = 0;
    virtual void connectToNetwork(const QString &ssid, const QString &password) = 0;

signals:
    void availableChanged(bool available);
    void enabledChanged(bool enabled);
    void busyChanged(bool busy);
    void scanningChanged(bool scanning);
    void currentSsidChanged(const QString &ssid);
    void statusTextChanged(const QString &status);
    void infoMessage(const QString &message);
    void errorMessage(const QString &message);
    void networksChanged();
};
