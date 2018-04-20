#include "network.h"
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <iostream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

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

bool Network::writeInterfacesConf()
{
    QFile write_interfaces(m_interfaces_file);
    if (!write_interfaces.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        setError(QString("writeInterfacesConf error: %1").arg(write_interfaces.errorString()));
        return false;
    }
    else
    {
        QTextStream out(&write_interfaces);
        out << m_content_interfaces_file << '\n';

        QStringList interfaces;
        for (const auto &item : m_list_network_address)
        {
            QString interface = item.value("interface").toString();
            if (interface != "")
            {
                interfaces.append(item.value("interface").toString());

                out << "auto " << interface << '\n';
                if (item.value("method").toString() == "manual")
                {
                    out << "iface " << interface << (QString(item.value("ip_version").toString()).toInt() == 4? " inet" : " inet6") << " static\n";
                    out << "\taddress " << item.value("address").toString() << '\n';
                    out << "\tnetmask " << item.value("mask").toString() << '\n';
                    out << "\tgateway " << item.value("gateway").toString() << '\n';
                    out << '\n';
                }
                else
                {
                    out << "iface " << interface << " inet dhcp\n";
                    out << '\n';
                }
            }
        }
        for (const auto &item : m_default_interfaces)
        {
            if (!interfaces.contains(item))
            {
                out << "auto " << item << '\n';
                out << "iface " << item << " inet dhcp\n\n";
            }
        }
        write_interfaces.close();
    }
    return true;
}

void Network::startInterface(QString interface)
{
    if (interface.isNull() || interface == "")
    {
        setError(QString("startInterface error: Interface is null"));
        return;
    }
    QString command = QString("/bin/bash \"ifup %1\"").arg(interface);
    QProcess start_interface;
    start_interface.start(command);

    if (!start_interface.waitForFinished())
        setError(QString("startInterface error: %1").arg(start_interface.errorString()));
    start_interface.close();
}

void Network::stopInterface(QString interface)
{
    if (interface.isNull() || interface == "")
    {
        setError(QString("stopInterface error: Interface is null"));
        return;
    }
    QString command = QString("/bin/bash \"ifdown %1\"").arg(interface);
    QProcess start_interface;
    start_interface.start(command);

    if (!start_interface.waitForFinished())
        setError(QString("stopInterface error: %1").arg(start_interface.errorString()));
    start_interface.close();
}

void Network::abstractInterface(QString interface)
{
    QString command = QString("ip addr show %1").arg(interface).trimmed();
    QProcess *process_addr = new QProcess();
    process_addr->start(command);

    QRegularExpression inet("inet (\\w+.\\w+.\\w+.\\w+)");
    QRegularExpression broadcast("brd (\\w+.\\w+.\\w+.\\w+)");

    if (!process_addr->waitForFinished())
        setError(QString("abstractInfo error: %1").arg(process_addr->errorString()));
    else
    {
        while (!process_addr->atEnd()) {
            QString line = process_addr->readLine();

            QRegularExpressionMatch match_inet = inet.match(line);
            if (match_inet.hasMatch())
                qDebug() << "IP" << match_inet.captured().replace("inet", "").trimmed();
            QRegularExpressionMatch match_broadcast = broadcast.match(line);
            if (match_broadcast.hasMatch())
                qDebug() << "Broadcast" << match_broadcast.captured().replace("brd", "").trimmed();
        }
    }

    process_addr->close();
    delete process_addr;
}
