#include "network.h"
#include <QDebug>
#include <iostream>

Network::Network()
{
    getAddress(m_fields, "");
}

Network::~Network()
{
    delete db;
}

void Network::getAddress(QStringList &fields, QString where)
{
    m_list_network_address = db->get(fields, where);
}

bool Network::setAddress(QJsonObject &data)
{
    bool query = false;
    if (!data.contains("id"))
        query = db->save(data);
    else
    {
        qint32 id = QString("%1").arg(data.value("id").toString()).toInt();
        data.remove("id");
        query = db->update(id, data);
    }
    if (query)
        getAddress(m_fields, QString(""));
    return query;
}
