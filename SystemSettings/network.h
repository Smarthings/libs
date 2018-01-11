#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QList>
#include <QJsonObject>
#include "logs.h"
#include "databasesettings.h"

class Network : public Logs
{
    Q_OBJECT

public:
    explicit Network();
    ~Network();

    /**
     * @brief configure network settings
     * @param QJsonObject data : data in JSON format
     */
    bool setAddress(QJsonObject &data);
    /**
     * @brief writes the interfaces file with the stored data
     */
    bool writeInterfacesConf();
    /**
     * @brief starts the network interface
     */
    void startInterface(QString interface);
    /**
     * @brief stop network interface
     */
    void stopInterface(QString interface);

signals:

public slots:

protected slots:

protected:
    /**
     * @brief get interface settings list
     * @param QStringList fields : list of field names of network table
     * @param QString where : string with the where of query
     */
    void getAddress(QStringList &fields, QString where);

private:
    QStringList m_interface;

    //const QString m_interfaces_file = "/etc/network/interfaces";
    const QString m_interfaces_file = "/tmp/interfaces";
    const QString m_content_interfaces_file = "# interfaces(5) file used by ifup(8) and ifdown(8)\n\nauto lo\niface lo inet loopback\n";
    const QStringList m_default_interfaces = {"wlan0", "eth0"};
    const QString m_table = "Network";
    DatabaseSettings *db = new DatabaseSettings(m_table);

    QList<QJsonObject> m_list_network_address;
    QStringList m_fields = {"id", "interface", "method", "status", "ip_version", "address", "mask", "gateway"};
};

#endif // NETWORK_H
