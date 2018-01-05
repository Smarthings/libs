#include "logs.h"
#include <QDebug>

Logs::Logs(QObject *parent) : QObject(parent)
{
    connect(this, &Logs::errorChanged, this, &Logs::printError);
}

void Logs::setError(QString err)
{
    m_error = err;
    emit errorChanged();
}

void Logs::printError()
{
    qDebug() << m_error;
}
