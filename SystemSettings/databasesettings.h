#ifndef DATABASESETTINGS_H
#define DATABASESETTINGS_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QJsonObject>
#include <QList>

class DatabaseSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    explicit DatabaseSettings(QString table, QObject *parent = nullptr);
    ~DatabaseSettings();
    QList<QJsonObject> get(QStringList fields, QString where);
    bool save(QJsonObject data);
    bool remove(quint32 id);

signals:
    void errorChanged();

public slots:
    QString error() { return m_error; }
    void openDatabaseSettings();

protected:
    void createTable();

private:
    QSqlDatabase m_database;
    QString m_error = "";

    const QString m_etc = "/etc/HomeThings";
    const QString m_path_database = "/tmp";
    QString m_database_file;
    QString m_table;
};

#endif // DATABASESETTINGS_H
