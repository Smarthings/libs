#ifndef DATABASESETTINGS_H
#define DATABASESETTINGS_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

class DatabaseSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
public:
    explicit DatabaseSettings(QObject *parent = nullptr);

signals:
    void errorChanged();

public slots:
    QString error() { return m_error; }

protected:
    void createTables();

private:
    const QString m_database_file = "/tmp/SystemSettings.sql";
    QSqlDatabase m_database;

    QString m_error = "";
    const QString m_etc = "/etc/HomeThings";
};

#endif // DATABASESETTINGS_H
