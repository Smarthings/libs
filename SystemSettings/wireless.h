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
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QJsonObject info READ info NOTIFY infoChanged)
    Q_PROPERTY(QString interface READ interface WRITE setInterface NOTIFY interfaceChanged)
    Q_PROPERTY(QList<QVariant> network_list READ network_list NOTIFY network_listChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    explicit Wireless(QObject *parent = nullptr);
    /**
     * @brief starts the network interface
     */
    void startWlan();
    /**
     * @brief stop network interface
     */
    void stopWlan();
    /**
     * @brief set network wireless. Receives the ESSID of the network and the password and run the wpa_passphrase command to obtain the output to the file wpa_supplicant.conf
     * @param QJSonObject data : data in JSON format with network ESSID and password
     */
    void setNetworkWireless(QJsonObject data);
    /**
     * @brief validate ESSID fields and password
     * @param QJsonData data : fields = ESSID, password
     */
    bool validateFieldsNetworkWireless(QJsonObject &data);
    /**
     * @brief Write the contents of the wpa_passphrase() command in the wpa_supplicant file
     * @param QString data : content of wpa_passphrase() command
     * @return return status of writing wpa_supplicant.conf file
     */
    bool writeWpaSupplicant(QString data);
    /**
     * @brief returns a list with available networks
     * @return
     */
    QList<QVariant> network_list() { return m_network_list; }
    /**
     * @brief returns the status of busy
     */
    bool busy() { return m_busy; }
    /**
     * @brief returns the error message
     * @return
     */
    QString error() { return m_error; }

Q_SIGNALS:
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
    /**
     * @brief emits the signal that the status has been updated
     */
    void busyChanged();
    /**
     * @brief emits the signal that some error has occurred
     */
    void errorChanged();

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

protected slots:
    /**
     * @brief Abstracts the iwlist command data and creates a list of available wireless networks. Emits the signal network_listChanged.
     * @param int status : parameter sent by the signal "finished (int)" when the process terminates execution
     */
    void parseScanWireless(int status);
    /**
     * @brief changes the m_status to busy and issues the busyChanged()
     * @param bool status
     */
    void busyIndicator(bool status);

private:
    QString m_interface = "";

    QTimer *timer;
    const qint32 m_scan_time = 2500;

    QJsonObject m_info;
    QString m_wifi_connected = "off/any";
    QString m_error = "";

    QProcess scan_wireless;
    QList<QVariant> m_network_list;

    bool m_busy_scan = false;
    bool m_busy = false;

    const QString m_wpa_supplicant = "/tmp/wpa_supplicant.conf";
};

#endif // WIRELESS_H
