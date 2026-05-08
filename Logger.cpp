#include "Logger.h"
#include <QDateTime>
#include <QDebug>

static Logger *s_instance = nullptr;

Logger::Logger(QObject *parent) : QObject(parent) {
    s_instance = this;
}

Logger *Logger::instance() {
    return s_instance;
}

QStringList Logger::logs() const {
    return m_logs;
}

void Logger::info(const QString &message) { addLog("INFO", message); }
void Logger::warn(const QString &message) { addLog("WARN", message); }
void Logger::error(const QString &message) { addLog("ERROR", message); }

void Logger::addLog(const QString &level, const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString formatted = QString("[%1] %2: %3").arg(timestamp, level, message);
    
    m_logs.prepend(formatted);
    if (m_logs.size() > 100) m_logs.removeLast();
    
    qDebug() << formatted;
    emit logsChanged();
    emit newLog(formatted);
}
