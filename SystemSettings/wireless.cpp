#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFile>
#include <QDebug>
#include <iostream>
#include <QSqlRecord>
#include <QSqlField>
#include <QJsonArray>
#include "wireless.h"

Wireless::Wireless() :
    timer(new QTimer)
{
    connect(scan_wireless, SIGNAL(finished(int)), this, SLOT(parseScanWireless(int)));
    connect(timer, SIGNAL(timeout()), this, SLOT(scanWireless()), Qt::UniqueConnection);
    timer->start(m_scan_time);
    getWifiSaved();
}

Wireless::~Wireless()
{
    //delete db;
    delete timer;
    delete scan_wireless;
}

void Wireless::startWlan()
{
    if (m_interface == "")
        return;

    QString command = QString("/bin/sh \"ifup %1\"").arg(m_interface);
    QProcess *start_wlan = new QProcess();
    start_wlan->start(command);

    if (!start_wlan->waitForFinished())
        setError(QString("startWlan Error: %1").arg(start_wlan->errorString()));
    start_wlan->close();
    getSSID(m_interface);
}

void Wireless::stopWlan()
{
    if (m_interface == "")
        return;

    QString command = QString("/bin/sh -c \"ifdown %1\"").arg(m_interface);
    QProcess *stop_wlan = new QProcess();
    stop_wlan->start(command);

    if (!stop_wlan->waitForFinished())
        setError(QString("stopWlan Error: %1").arg(stop_wlan->errorString()));
    stop_wlan->close();
    delete stop_wlan;
    getSSID(m_interface);
}

bool Wireless::setNetworkWireless(QJsonObject data)
{
    busyIndicator(true);

    if (!validateFieldsNetworkWireless(data))
    {
        setError(QString("setNetworkWireless Error: Campos obrigatórios não informados"));
        return false;
    }

    if (!data.value("Saved").toBool() && data.value("Encryption").toBool() && data.value("ESSID").toString() == "" && data.value("Password").toString() == "")
    {
        setError(QString("setNetworkWireless Error: ESSID ou senha em branco"));
        return false;
    }

    if (!data.value("Saved").toBool() && !m_list_settings_saved.contains(data.value("ESSID").toString()))
    {
        QProcess *getHashPassword = new QProcess();
        getHashPassword->start(QString("wpa_passphrase \"%1\" \"%2\"").arg(data.value("ESSID").toString()).arg(data.value("Password").toString()));

        if (!getHashPassword->waitForFinished())
        {
            setError(QString("setNetworkWireless error: %1").arg(getHashPassword->errorString()));
            getHashPassword->close();
            return false;
        }

        QRegularExpression errorMsg("(Passphrase must be.*)");
        QRegularExpression sharpPsk("(#psk=.*)");
        QRegularExpression psk("(psk=.*)");
        while (!getHashPassword->atEnd()) {
            QString line = getHashPassword->readLine();

            QRegularExpressionMatch match_errorMsg = errorMsg.match(line);
            if (match_errorMsg.hasMatch())
            {
                setError(QString("setNetworkWireless Error: %1").arg(match_errorMsg.captured()));
                getHashPassword->close();
                return false;
            }

            QRegularExpressionMatch match_sharpPsk = sharpPsk.match(line);
            if (match_sharpPsk.hasMatch())
            {
                if (data.value("Password").toString() == QString(match_sharpPsk.captured().replace("\"", "").split("=")[1]))
                    continue;
            }

            QRegularExpressionMatch match_psk = psk.match(line);
            if (match_psk.hasMatch())
            {
                m_list_settings_saved.insert(data.value("ESSID").toString(), QJsonObject({{"Password", QString(match_psk.captured()).replace("psk=", "").trimmed()}, {"Encryption", data.value("Encryption").toBool()}}));
                break;
            }
        }

        getHashPassword->close();
        delete getHashPassword;
    }

    if (writeWpaSupplicant(data))
    {
        stopWlan();
        startWlan();
    }

    busyIndicator(false);
    return true;
}

bool Wireless::forgetWirelessNetwork(QJsonObject data)
{
    busyIndicator(true);
    if (!data.contains("ESSID"))
    {
        setError(QString("forgetWirelessNetwork error: Campos obrigatório não informado"));
        return false;
    }

    bool hasKey = m_list_settings_saved.contains(data.value("ESSID").toString());
    m_list_settings_saved.remove(data.value("ESSID").toString());

    busyIndicator(false);
    return (hasKey && !m_list_settings_saved.contains(data.value("ESSID").toString()) && writeWpaSupplicant(data));
}

bool Wireless::validateFieldsNetworkWireless(QJsonObject data)
{
    if (data.contains("ESSID") && data.contains("Password") && data.contains("Encryption") && data.contains("Saved"))
        return true;
    return false;
}

bool Wireless::writeWpaSupplicant(QJsonObject &data)
{
    QString content_wpa_supplicant = "";
    for (auto essid: m_list_settings_saved.keys())
    {
        if (m_list_settings_saved[essid].toObject().value("Encryption").toBool())
            content_wpa_supplicant.append(QString("network={\n\tssid=\"%1\"\n\tpsk=\"%2\"\n\tpriority=%3\n}\n")
                                          .arg(essid)
                                          .arg(m_list_settings_saved.value(essid).toObject().value("Password").toString())
                                          .arg((essid == data.value("ESSID").toString())? 2 : 1));
        else
            content_wpa_supplicant.append(QString("network={\n\tssid=\"%1\"\n\tkey_mgmt=NONE\n\tpriority=%2\n}\n")
                                          .arg(essid)
                                          .arg((essid == data.value("ESSID").toString())? 2 : 1));
    }

    QFile file_wpa_supplicant(m_wpa_supplicant);
    if (!file_wpa_supplicant.open(QIODevice::WriteOnly))
    {
        setError(QString("writeWpaSupplicant Error: %1").arg(file_wpa_supplicant.errorString()));
        return false;
    }
    if (!file_wpa_supplicant.write(content_wpa_supplicant.toUtf8(), content_wpa_supplicant.length()))
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
    getWifiSaved();

    return true;
}

const QJsonObject Wireless::info()
{
    return m_info;
}

void Wireless::setInterface(QString iface)
{
    m_interface = std::move(iface);
    getSSID(m_interface);
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
                    ipv4.insert("gateway", getGatewayIface(iface.name()));

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
    scan_wireless->start(command);
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
        while (!scan_wireless->atEnd()) {
            QString line = scan_wireless->readLine();

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
                if (m_list_settings_saved.contains(split.at(1)))
                    obj.insert("saved", true);
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
                obj.insert(split.at(0), (split.at(1) == "on"? true : false));
            }

            QRegularExpressionMatch match_QUALITY = reg_QUALITY.match(line);
            if (match_QUALITY.hasMatch())
            {
                QStringList split = match_QUALITY.captured().replace("\"", "").split(" ");
                split = QString(split.at(0)).split("=");
                obj.insert(split.at(0), split.at(1));
            }

            if (scan_wireless->atEnd())
            {
                m_network_list.append(obj.toVariantMap());
                obj = {};
            }
        }
        if (!m_network_list.isEmpty())
            emit network_listChanged();
        scan_wireless->close();
    }
    m_busy_scan = false;
}

void Wireless::busyIndicator(bool status)
{
    m_busy = status;
    emit busyChanged();
}

bool Wireless::saveSettings(QJsonObject &data)
{
    return false;
}

void Wireless::getWifiSaved()
{
    QFile *wpa_supplicant_file = new QFile(m_wpa_supplicant);
    if (!wpa_supplicant_file->open(QIODevice::ReadOnly))
        setError(QString("getWifiSaved error: %1").arg(wpa_supplicant_file->errorString()));
    else
    {
        QRegularExpression line_network("(network={)");
        QRegularExpression line_ssid("(ssid=.*)");
        QRegularExpression line_psk("(psk=.*)");
        QRegularExpression line_key_mgmt("(key_mgmt=NONE)");
        QRegularExpression line_end_network("(})");

        QString essid, psk;
        bool encryption = true;

        while (!wpa_supplicant_file->atEnd()) {
            QString line = wpa_supplicant_file->readLine();

            QRegularExpressionMatch match_line_network = line_network.match(line);
            if (match_line_network.hasMatch())
            {
                essid = "";
                psk = "";
                encryption = true;
            }

            QRegularExpressionMatch match_line_ssid = line_ssid.match(line);
            if (match_line_ssid.hasMatch())
                essid = match_line_ssid.captured().replace("ssid=", "").replace("\"", "").trimmed();

            QRegularExpressionMatch match_line_psk = line_psk.match(line);
            if (match_line_psk.hasMatch())
                psk = match_line_psk.captured().replace("psk=", "").replace("\"", "").trimmed();

            QRegularExpressionMatch match_line_key_mgmt = line_key_mgmt.match(line);
            if (match_line_key_mgmt.hasMatch())
                encryption = false;

            QRegularExpressionMatch match_line_end_network = line_end_network.match(line);
            if (match_line_end_network.hasMatch())
                m_list_settings_saved.insert(essid, QJsonObject({{"Password", psk}, {"Encryption", encryption}}));
        }
    }
    wpa_supplicant_file->close();
    delete wpa_supplicant_file;
}

bool Wireless::deleteSettings(quint32 &id)
{
    return false;
}

const QStringList Wireless::checkSaved(QString ssid)
{
    /*QStringList list;
    for (const auto &essid : m_list_settings_saved)
    {
        if (essid.value("ssid").toString() == ssid)
        {
            list << essid.value("id").toString() << essid.value("ssid").toString();
            return list;
        }
    }
    return list;*/
    return {};
}

QString Wireless::getGatewayIface(QString iface)
{
    QString gateway = "";
    QString command = QString("ip route show");
    QProcess *process_addr = new QProcess();
    process_addr->start(command);

    QRegularExpression line_gateway(QString("default via (\\d+.\\d+.\\d+.\\d+) dev %1").arg(iface));
    QRegularExpression get_gateway(QString("(\\d+.\\d+.\\d+.\\d+)"));
    if (!process_addr->waitForFinished())
        setError(QString("getGatewayIface error: %1").arg(process_addr->errorString()));
    else
    {
        while (!process_addr->atEnd()) {
            QString line = process_addr->readLine();
            QRegularExpressionMatch match_gateway = line_gateway.match(line);
            if (match_gateway.hasMatch())
            {
                QRegularExpressionMatch match_get_gateway = get_gateway.match(line);
                if (match_get_gateway.hasMatch())
                    return match_get_gateway.captured(0);
            }
        }
    }

    process_addr->close();

    return gateway;
}

void Wireless::getSSID(QString iface)
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
            m_wifi_connected = QString(match_SSID.captured()).replace("\"", "").trimmed().split(":").at(1);
        else
            m_wifi_connected = "off/any";
    }
    process.close();
    emit connectedChanged();
}
