#pragma once

#include "WifiNetworkModel.h"
#include "IWifiBackend.h"

#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QHash>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QVariant>
#include <QVariantMap>

class IwdAgent;

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QAbstractListModel>
#include <QtQml/qqml.h>

class WifiController final : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    QML_UNCREATABLE("Singleton")

    Q_PROPERTY(QString backendName READ backendName NOTIFY backendNameChanged)
    Q_PROPERTY(bool supported READ supported NOTIFY supportedChanged)
    Q_PROPERTY(bool readOnly READ readOnly NOTIFY readOnlyChanged)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QString currentSsid READ currentSsid NOTIFY currentSsidChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString infoMessage READ infoMessage NOTIFY infoMessageChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QString unsupportedReason READ unsupportedReason NOTIFY unsupportedReasonChanged)
    Q_PROPERTY(QAbstractItemModel *networks READ networks CONSTANT)

public:
    using ConnectionSettingsMap = QMap<QString, QVariantMap>;

    explicit WifiController(QObject *parent = nullptr);

    QString backendName() const;
    bool supported() const;
    bool readOnly() const;
    bool available() const;
    bool enabled() const;
    bool busy() const;
    bool scanning() const;
    QString currentSsid() const;
    QString statusText() const;
    QString infoMessage() const;
    QString errorMessage() const;
    QString unsupportedReason() const;
    QAbstractItemModel *networks();

    Q_INVOKABLE void refreshState();
    Q_INVOKABLE void refreshNetworks(bool rescan = false);
    Q_INVOKABLE void setEnabled(bool enabled);
    Q_INVOKABLE void disconnectCurrent();
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password = QString());
    Q_INVOKABLE void clearMessages();

signals:
    void backendNameChanged();
    void supportedChanged();
    void readOnlyChanged();
    void availableChanged();
    void enabledChanged();
    void busyChanged();
    void scanningChanged();
    void currentSsidChanged();
    void statusTextChanged();
    void infoMessageChanged();
    void errorMessageChanged();
    void unsupportedReasonChanged();

private slots:
    void handleNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void handleManagerPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void handleDevicePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void handleAccessPointAdded(const QDBusObjectPath &accessPointPath);
    void handleAccessPointRemoved(const QDBusObjectPath &accessPointPath);
    void handleDeviceAdded(const QDBusObjectPath &devicePath);
    void handleDeviceRemoved(const QDBusObjectPath &devicePath);
    void handleIwdInterfacesAdded(const QDBusObjectPath &objectPath, const QDBusArgument &interfacesAndProperties);
    void handleIwdInterfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);
    void handleNewConnection(const QDBusObjectPath &connectionPath);
    void handleConnectionRemoved(const QDBusObjectPath &connectionPath);

private:
    friend class IwdAgent;

    void detectBackend();
    void refreshStateInternal();
    void refreshNetworksInternal(bool rescan, bool triggeredBySignal);
    void reloadSavedConnections();
    void reconnectDeviceSignals(const QString &devicePath);
    void disconnectDeviceSignals();
    void ensureIwdAgentRegistered();
    void unregisterIwdAgent();
    void setIwdAgentRegisteredState(bool registered);
    void setIwdAgentRegistrationError(const QString &error);
    QString takeIwdPassphraseForNetwork(const QString &networkPath);

    QVariant getProperty(const QString &service, const QString &path, const QString &interfaceName, const QString &propertyName) const;
    bool setProperty(const QString &service, const QString &path, const QString &interfaceName, const QString &propertyName, const QVariant &value, QString *errorMessage = nullptr) const;
    QDBusMessage callMethod(const QString &service, const QString &path, const QString &interfaceName, const QString &methodName, const QList<QVariant> &arguments = {}) const;

    void setBackendName(const QString &backendName);
    void setSupported(bool supported);
    void setReadOnly(bool readOnly);
    void setAvailable(bool available);
    void setEnabledState(bool enabled);
    void setBusyState(bool busy);
    void setScanningState(bool scanning);
    void setCurrentSsid(const QString &currentSsid);
    void setStatusText(const QString &statusText);
    void setInfoMessage(const QString &infoMessage);
    void setErrorMessage(const QString &errorMessage);
    void setUnsupportedReason(const QString &unsupportedReason);
    void updateBusyState();
    void updateStatusText();
    void clearUnsupportedState(const QString &backendName, bool available, const QString &reason);
    void clearSupportedState();

    QString errorTextForReply(const QDBusMessage &reply, const QString &fallback) const;
    bool activateSavedConnection(const QString &ssid, const QString &accessPointPath);
    bool addAndActivateConnection(const QString &ssid, const QString &accessPointPath, const QString &password, bool secure);

    WifiNetworkModel m_networks;
    QHash<QString, QString> m_savedConnectionsBySsid;
    void setBackend(IWifiBackend *backend);

    IWifiBackend *m_backend = nullptr;
    QString m_infoMessage;
    QString m_errorMessage;
    QTimer m_stateRefreshTimer;
};
