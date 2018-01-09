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

    bool setAddress(QJsonObject &data);

signals:

public slots:

protected slots:
    void getAddress(QStringList &fields, QString where);

private:
    QStringList m_interface;

    //const QString m_interfaces_file = "/etc/network/interfaces";
    const QString m_interfaces_file = "/tmp/interfaces";
    const QString m_table = "Network";
    DatabaseSettings *db = new DatabaseSettings(m_table);

    QList<QJsonObject> m_list_network_address;
    QStringList m_fields = {"id", "interface", "method", "status", "ip_version", "address", "mask", "gateway"};
};

#endif // NETWORK_H
