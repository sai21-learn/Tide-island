#pragma once

#include <QObject>
#include <QStringList>
#include <QtQml/qqml.h>

class Logger : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QStringList logs READ logs NOTIFY logsChanged)

public:
    explicit Logger(QObject *parent = nullptr);
    static Logger *instance();

    QStringList logs() const;

    Q_INVOKABLE void info(const QString &message);
    Q_INVOKABLE void warn(const QString &message);
    Q_INVOKABLE void error(const QString &message);

signals:
    void logsChanged();
    void newLog(const QString &message);

private:
    void addLog(const QString &level, const QString &message);
    QStringList m_logs;
};
