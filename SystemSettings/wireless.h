#ifndef WIRELESS_H
#define WIRELESS_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QJsonObject>
#include <QProcess>
#include <QTimer>

class Wireless : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QJsonObject info READ info NOTIFY infoChanged)
    Q_PROPERTY(QString interface READ interface WRITE setInterface NOTIFY interfaceChanged)
    Q_PROPERTY(QList<QVariant> network_list READ network_list NOTIFY network_listChanged)

public:
    explicit Wireless(QObject *parent = nullptr);

signals:
    /**
     * @brief emits the signal that the info variable has been updated
     */
    void infoChanged();
    /**
     * @brief emits the signal that the variable containing the list of available networks has been updated
     */
    void network_listChanged();
    /**
     * @brief emits the signal that the interface name variable has been updated
     */
    void interfaceChanged();

public slots:
    /**
     * @brief returns a JSON with the interface information
     */
    const QJsonObject info();
    /**
     * @brief return name of interface
     */
    const QString interface () { return m_interface; }
    /**
     * @brief get network SSID
     * @param QString iface : interface name
     */
    const QString getSSID(QString iface);
    /**
     * @brief defines the interface name
     * @param QString iface : interface name
     */
    void setInterface(QString iface);
    /**
     * @brief abstracts data from the network interface
     */
    void abstractInfo();
    /**
     * @brief scan the wireless network. It is executed by a time in construct
     */
    void scanWireless();
    /**
     * @brief network_list
     * @return
     */
    QList<QVariant> network_list() { return m_network_list; }

protected slots:
    /**
     * @brief Abstracts the iwlist command data and creates a list of available wireless networks. Emits the signal network_listChanged.
     * @param int status : parameter sent by the signal "finished (int)" when the process terminates execution
     */
    void parseScanWireless(int status);

private:
    QString m_interface = "";

    QTimer *timer;
    const qint32 m_scan_time = 2500;

    QJsonObject m_info;
    QString m_wifi_connected = "off/any";

    QProcess scan_wireless;
    bool m_busy_scan = false;
    QList<QVariant> m_network_list;
};

#endif // WIRELESS_H
