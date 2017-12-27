#include "systemsettings.h"
#include <QNetworkInterface>
#include <QDebug>

SystemSettings::SystemSettings()
{
    discoveryInterfaces();
}

const QStringList SystemSettings::getInterfaces()
{
    return m_interfaces;
}

void SystemSettings::discoveryInterfaces()
{
    m_interfaces.clear();
    QList<QNetworkInterface> discovery = QNetworkInterface::allInterfaces();
    for (auto &interface : discovery)
    {
        if (interface.name() != "lo")
            m_interfaces.append(interface.name());
    }
}

