#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include "logs.h"
#include "databasesettings.h"

class Network : public Logs
{
    Q_OBJECT

public:
    explicit Network();
    ~Network();

signals:

public slots:

protected slots:

private:
    QStringList m_interface;

    const QString m_interfaces_file = "/etc/network/interfaces";
    const QString m_table = "Network";
    DatabaseSettings *db = new DatabaseSettings(m_table);
};

#endif // NETWORK_H
