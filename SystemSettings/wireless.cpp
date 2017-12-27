#include "wireless.h"
#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDebug>
#include <iostream>

Wireless::Wireless(QObject *parent) :
    QObject(parent),
    timer(new QTimer)
{
    connect(&scan_wireless, SIGNAL(finished(int)), this, SLOT(parseScanWireless(int)));
    connect(timer, SIGNAL(timeout()), this, SLOT(scanWireless()), Qt::UniqueConnection);
    timer->start(m_scan_time);
}

const QJsonObject Wireless::info()
{
    return m_info;
}

void Wireless::setInterface(QString iface)
{
    m_interface = iface;
    emit interfaceChanged();
}

void Wireless::abstractInfo()
{
    QList<QNetworkInterface> discovery = QNetworkInterface::allInterfaces();
    for (const auto &iface : discovery)
    {
        QJsonObject obj;
        if (iface.name() == m_interface && iface.name() != QString("lo") && iface.name().indexOf("wl") >= 0)
        {
            obj.insert("status", iface.flags().testFlag(QNetworkInterface::IsUp));
            obj.insert("macaddress", iface.hardwareAddress());
            for (const auto &addr : iface.addressEntries())
            {
                if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    QJsonObject ipv4;
                    ipv4.insert("address", addr.ip().toString());
                    ipv4.insert("netmask", addr.netmask().toString());
                    ipv4.insert("broadcast", addr.broadcast().toString());

                    obj.insert("ipv4", ipv4);
                }
                if (addr.ip().protocol() == QAbstractSocket::IPv6Protocol)
                {
                    QJsonObject ipv6;
                    ipv6.insert("address", addr.ip().toString());
                    ipv6.insert("netmask", addr.netmask().toString());
                    ipv6.insert("broadcast", addr.broadcast().toString());

                    obj.insert("ipv6", ipv6);
                }
                obj.insert("ssid", getSSID(std::move(iface.name())));
            }
            m_info.insert(iface.name(), obj);
        }
    }
    emit infoChanged();
}

void Wireless::scanWireless()
{
    if (m_busy_scan || m_interface == "")
        return;
    m_busy_scan = true;

    QString command = QString("iwlist %1 scan").arg(m_interface);
    scan_wireless.start(command);
}

void Wireless::parseScanWireless(int status)
{
    m_network_list.clear();

    QRegularExpression reg_START("(Cell.*- Address: .*)");
    QRegularExpression reg_SSID("(ESSID:.*)");
    QRegularExpression reg_CHANNEL("(Channel:.*)");
    QRegularExpression reg_ENCRIPTION("(Encryption key:.*)");
    QRegularExpression reg_QUALITY("(Quality=.* Signal level=.*)");

    QJsonObject obj;
    if (status == 0)
    {
        while (!scan_wireless.atEnd()) {
            QString line = scan_wireless.readLine();

            QRegularExpressionMatch match_START = reg_START.match(line);
            if (match_START.hasMatch() && obj.length() > 0)
                obj = {};

            QRegularExpressionMatch match_SSID = reg_SSID.match(line);
            if (match_SSID.hasMatch())
            {
                QStringList split = match_SSID.captured().replace("\"", "").split(":");
                obj.insert(split.at(0), split.at(1));
                if (split.at(1) == m_wifi_connected)
                    obj.insert("connected", true);
            }

            QRegularExpressionMatch match_CHANNEL = reg_CHANNEL.match(line);
            if (match_CHANNEL.hasMatch()) {
                QStringList split = match_CHANNEL.captured().replace("\"", "").split(":");
                obj.insert(split.at(0), split.at(1));
            }

            QRegularExpressionMatch match_ENCRIPTION = reg_ENCRIPTION.match(line);
            if (match_ENCRIPTION.hasMatch()) {
                QStringList split = match_ENCRIPTION.captured().replace("\"", "").replace(" key", "").split(":");
                obj.insert(split.at(0), split.at(1));
            }

            QRegularExpressionMatch match_QUALITY = reg_QUALITY.match(line);
            if (match_QUALITY.hasMatch()) {
                QStringList split = match_QUALITY.captured().replace("\"", "").split(" ");
                split = QString(split.at(0)).split("=");
                obj.insert(split.at(0), split.at(1));
            }

            if (scan_wireless.atEnd())
            {
                m_network_list.append(obj.toVariantMap());
                obj = {};
            }
        }
        if (!m_network_list.isEmpty())
            emit network_listChanged();
    }
    scan_wireless.close();
    m_busy_scan = false;
}

const QString Wireless::getSSID(QString iface)
{
    QString command = QString("iwconfig %1").arg(iface);
    QProcess process;
    process.start(command);

    if (!process.waitForFinished())
        qDebug() << command << "Error" << process.errorString();
    else
    {
        QString result = process.readAll();
        QRegularExpression reg_SSID("(ESSID:.*)");
        QRegularExpressionMatch match_SSID = reg_SSID.match(result);
        if (match_SSID.hasMatch())
            return QString(match_SSID.captured()).replace("\"", "").replace(" ", "").split(":").at(1);
    }
    return QString("");
}

