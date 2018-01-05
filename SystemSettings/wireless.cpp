#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFile>
#include <QDebug>
#include <iostream>
#include <QSqlRecord>
#include <QSqlField>
#include "wireless.h"
#include "databasesettings.h"

Wireless::Wireless() :
    timer(new QTimer)
{
    connect(&scan_wireless, SIGNAL(finished(int)), this, SLOT(parseScanWireless(int)));
    connect(timer, SIGNAL(timeout()), this, SLOT(scanWireless()), Qt::UniqueConnection);
    timer->start(m_scan_time);

    db->openDatabaseSettings();
    getSettings(m_fields, "");
}

Wireless::~Wireless()
{
    delete db;
    delete timer;
}

void Wireless::startWlan()
{
    if (m_interface == "")
        return;

    QString command = QString("/bin/sh \"ifup %1\"").arg(m_interface);
    QProcess start_wlan;
    start_wlan.start(command);

    if (!start_wlan.waitForFinished())
        setError(QString("startWlan Error: %1").arg(start_wlan.errorString()));
    start_wlan.close();
}

void Wireless::stopWlan()
{
    if (m_interface == "")
        return;

    QString command = QString("/bin/sh -c \"ifdown %1\"").arg(m_interface);
    QProcess stop_wlan;
    stop_wlan.start(command);

    if (!stop_wlan.waitForFinished())
        setError(QString("stopWlan Error: %1").arg(stop_wlan.errorString()));
    stop_wlan.close();
}

void Wireless::setNetworkWireless(QJsonObject data)
{
    busyIndicator(true);
    QString command = QString("wpa_passphrase \"%1\" \"%2\"")
            .arg(data.value("ESSID").toString())
            .arg(data.value("password").toString());

    if (!validateFieldsNetworkWireless(data))
    {
        setError(QString("setNetworkWireless Error: Campo ESSID ou password não informado"));
        return;
    }
    if (data.value("ESSID").toString() == "" || data.value("password").toString() == "")
    {
        setError(QString("setNetworkWireless Error: ESSID ou senha em branco"));
        return;
    }

    QProcess write_network_wireless;
    write_network_wireless.start(command);

    if (!write_network_wireless.waitForFinished())
        setError(QString("setNetworkWireless Error: %1").arg(write_network_wireless.errorString()));
    else
    {
        QString contentWpaSupplicant;
        QRegularExpression errorMsg("(Passphrase must be.*)");
        QRegularExpression sharpPsk("(#psk=.*)");
        QRegularExpression psk("(psk=.*)");

        while (!write_network_wireless.atEnd())
        {
            QString line = write_network_wireless.readLine();

            QRegularExpressionMatch match_errorMsg = errorMsg.match(line);
            if (match_errorMsg.hasMatch())
            {
                setError(QString("stopWlan Error: %1").arg(match_errorMsg.captured()));
                break;
            }

            QRegularExpressionMatch match_sharpPsk = sharpPsk.match(line);
            if (match_sharpPsk.hasMatch())
            {
                if (data.value("password").toString() ==
                        QString(match_sharpPsk.captured().replace("\"", "").split("=")[1]))
                    continue;
            }

            QRegularExpressionMatch match_psk = psk.match(line);
            if (match_psk.hasMatch())
                saveSettings({{"ssid", data.value("ESSID").toString()}, {"pass_crypt", QString(match_psk.captured()).split("=")[1]}});

            contentWpaSupplicant.append(line);
        }
        write_network_wireless.close();
        if (writeWpaSupplicant(contentWpaSupplicant))
        {
            stopWlan();
            startWlan();
        }
        getSSID(m_interface);
    }

    busyIndicator(false);
}

bool Wireless::validateFieldsNetworkWireless(QJsonObject &data)
{
    if (data.contains("ESSID") || data.contains("password"))
        return true;
    return false;
}

bool Wireless::writeWpaSupplicant(QString data)
{
    QFile file_wpa_supplicant(m_wpa_supplicant);
    if (!file_wpa_supplicant.open(QIODevice::WriteOnly))
    {
        setError(QString("writeWpaSupplicant Error: %1").arg(file_wpa_supplicant.errorString()));
        return false;
    }
    if (!file_wpa_supplicant.write(data.toUtf8(), data.length()))
    {
        setError(QString("writeWpaSupplicant Error: %1").arg(file_wpa_supplicant.errorString()));
        file_wpa_supplicant.close();
        return false;
    }
    file_wpa_supplicant.close();
    if (file_wpa_supplicant.isOpen())
    {
        setError(QString("writeWpaSupplicant Error: %1").arg(file_wpa_supplicant.errorString()));
        return false;
    }
    return true;
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

bool Wireless::forgetWirelessNetwork(quint32 id)
{
    bool query = db->remove(id);
    getSettings(m_fields, "");
    return query;
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
            {
                m_network_list.append(obj.toVariantMap());
                obj = {};
            }

            QRegularExpressionMatch match_SSID = reg_SSID.match(line);
            if (match_SSID.hasMatch())
            {
                QStringList split = match_SSID.captured().replace("\"", "").split(":");
                obj.insert(split.at(0), split.at(1));
                QStringList savedPass = checkSaved(split.at(1));
                if (savedPass.count() > 0)
                {
                    obj.insert("id", savedPass.at(0));
                    obj.insert("saved", true);
                }
                if (split.at(1) == m_wifi_connected)
                    obj.insert("connected", true);
            }

            QRegularExpressionMatch match_CHANNEL = reg_CHANNEL.match(line);
            if (match_CHANNEL.hasMatch())
            {
                QStringList split = match_CHANNEL.captured().replace("\"", "").split(":");
                obj.insert(split.at(0), split.at(1));
            }

            QRegularExpressionMatch match_ENCRIPTION = reg_ENCRIPTION.match(line);
            if (match_ENCRIPTION.hasMatch())
            {
                QStringList split = match_ENCRIPTION.captured().replace("\"", "").replace(" key", "").split(":");
                obj.insert(split.at(0), split.at(1));
            }

            QRegularExpressionMatch match_QUALITY = reg_QUALITY.match(line);
            if (match_QUALITY.hasMatch())
            {
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
        scan_wireless.close();
    }
    m_busy_scan = false;
}

void Wireless::busyIndicator(bool status)
{
    m_busy = status;
    emit busyChanged();
}

bool Wireless::saveSettings(QJsonObject data)
{
    bool query = db->save(data);
    getSettings(m_fields, "");
    return query;
}

void Wireless::getSettings(QStringList fields,  QString where)
{
    m_list_settings_saved = db->get(fields, where);
}

bool Wireless::deleteSettings(quint32 id)
{
    bool query = db->remove(id);
    getSettings(m_fields, "");
    return query;
}

const QStringList Wireless::checkSaved(QString ssid)
{
    QStringList list;
    for (const auto &essid : m_list_settings_saved)
    {
        if (essid.value("ssid").toString() == ssid)
        {
            list << essid.value("id").toString() << essid.value("ssid").toString();
            return list;
        }
    }
    return list;
}

const QString Wireless::getSSID(QString iface)
{
    QString command = QString("iwconfig %1").arg(iface);
    QProcess process;
    process.start(command);

    if (!process.waitForFinished())
        setError(QString("getSSID Error: %1").arg(process.errorString()));
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
