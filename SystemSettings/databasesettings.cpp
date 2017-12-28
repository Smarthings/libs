#include "databasesettings.h"
#include <QDebug>
#include <QDirIterator>

DatabaseSettings::DatabaseSettings(QObject *parent) :
    QObject(parent), m_database(QSqlDatabase::addDatabase("QSQLITE"))
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
        qDebug() << etc.next();
    }
}

