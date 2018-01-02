#include "databasesettings.h"
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>


DatabaseSettings::DatabaseSettings(QObject *parent) :
    QObject(parent), m_database(QSqlDatabase::addDatabase("QSQLITE"))
{

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
        createTables();
}

void DatabaseSettings::createTables()
{
    QDirIterator etc(QString("%1/sql").arg(m_etc), QDirIterator::Subdirectories);
    while (etc.hasNext())
    {
        etc.next();
        if (etc.fileInfo().isFile())
        {
            if (etc.fileInfo().completeSuffix() == "sql")
            {
                QString table = etc.fileInfo().baseName();
                QSqlQuery query_check_table(QString("SELECT id as count FROM %1 LIMIT 1").arg(table));
                if (!query_check_table.record().count())
                {
                    QString file_name = etc.fileInfo().filePath();
                    QFile sql_file(file_name);
                    if (!sql_file.open(QIODevice::ReadOnly))
                    {
                        m_error = QString("createTables Error: %1 [%2]").arg(sql_file.errorString()).arg(file_name);
                        emit errorChanged();
                        continue;
                    }
                    QSqlQuery sql_create_table(sql_file.readAll());
                    if (!sql_create_table.exec())
                    {
                        m_error = QString("createTables Error: %1").arg(sql_create_table.lastError().text());
                        emit errorChanged();
                    }
                }
            }
        }
    }
}
