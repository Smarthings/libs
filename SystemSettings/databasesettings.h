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
    /**
     * @brief prepare variable for address of database and table name
     * @param QString table : table name
     */
    explicit DatabaseSettings(QString table, QObject *parent = nullptr);
    ~DatabaseSettings();
    /**
     * @brief get JSON list of query
     * @param QStringList fields : list with fields name
     * @param QString where : string of where for query
     */
    QList<QJsonObject> get(QStringList fields, QString where);
    /**
     * @brief save data
     * @param QJsonObject data : JSON list of data (field : value)
     */
    bool save(QJsonObject data);
    /**
     * @brief remove a value of table
     * @param quint32 id : ID of row to remove
     */
    bool remove(quint32 id);

signals:
    /**
     * @brief emit error signal of query
     */
    void errorChanged();

public slots:
    /**
     * @brief return string of error
     */
    QString error() { return m_error; }
    /**
     * @brief open database. If open, run createTable
     */
    void openDatabaseSettings();

protected:
    /**
     * @brief check if exists the table, else open file with structure of table and create table.
     */
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
