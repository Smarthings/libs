#include "databasesettings.h"
#include <QDebug>
#include <QFile>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>


DatabaseSettings::DatabaseSettings(QString table, QObject *parent) :
    QObject(parent), m_database(QSqlDatabase::addDatabase("QSQLITE"))
{
    m_database_file = QString("%1/%2.sql").arg(m_path_database).arg(table);
    m_table = table;
}

DatabaseSettings::~DatabaseSettings()
{
    if (m_database.isOpen())
        m_database.close();
}

void DatabaseSettings::openDatabaseSettings()
{
    m_database.setDatabaseName(m_database_file);
    if (!m_database.open())
    {
        m_error = QString("DatabaseSettings Error: %1").arg(m_database.lastError().text());
        emit errorChanged();
    }
    else
        createTable();
}

void DatabaseSettings::createTable()
{
    QSqlQuery query_check_table(QString("SELECT id as count FROM %1 LIMIT 1").arg(m_table));
    if (!query_check_table.record().count())
    {
        QFile sql_file(QString("%1/sql/%2.sql").arg(m_etc).arg(m_table));
        if (!sql_file.open(QIODevice::ReadOnly))
        {
            m_error = QString("createTable error: %1 [%2]").arg(sql_file.errorString()).arg(m_table);
            emit errorChanged();
            return;
        }
        QSqlQuery sql_create_table(sql_file.readAll());
        if (!sql_create_table.exec())
        {
            m_error = QString("createTable error: %1").arg(sql_create_table.lastError().text());
            emit errorChanged();
        }
    }
}

bool DatabaseSettings::save(QJsonObject data)
{
    QString insert_query = QString("INSERT INTO %1").arg(m_table);
    QString fields_name = "(", fields_value = "VALUES (";
    QStringList prepare_values;
    QSqlQuery query;

    for (const auto &field : data.keys())
        prepare_values.append(QString(":%1").arg(field));
    fields_name.append(data.keys().join(", ")).append(")");
    fields_value.append(prepare_values.join(", ").append(")"));

    query.prepare(QString("%1 %2 %3").arg(insert_query).arg(fields_name).arg(fields_value));
    qint32 i = 0;
    for (const auto &field : data.keys())
    {
        query.bindValue(i, data.value(field).toString());
        ++i;
    }

    if (!query.exec())
    {
        m_error = QString("save error: %1").arg(query.lastError().text());
        emit errorChanged();
        return false;
    }
    return true;
}

bool DatabaseSettings::remove(quint32 id)
{
    QSqlQuery query;
    query.prepare(QString("DELETE FROM %1 WHERE id=:id").arg(m_table));
    query.bindValue(0, id);
    if (!query.exec())
    {
        m_error = QString("remove error: %1").arg(query.lastError().text());
        emit errorChanged();
        return false;
    }
    return (query.numRowsAffected())? true : false;
}

QList<QJsonObject> DatabaseSettings::get(QStringList fields, QString where)
{
    QList<QJsonObject> list;
    QSqlQuery query;
    query.prepare(QString("SELECT %1 FROM %2 %3").arg(fields.join(", ")).arg(m_table).arg(where));
    if (!query.exec())
    {
        m_error = QString("get error: %1").arg(query.lastError().text());
        emit errorChanged();
        return list;
    }

    while (query.next())
    {
        QJsonObject row;
        for (const auto &field : fields)
            row.insert(field, query.value(field).toString());
        list << row;
    }
    return list;
}
