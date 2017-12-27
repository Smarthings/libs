#ifndef SYSTEMSETTINGS_H
#define SYSTEMSETTINGS_H

#include <QStringList>
#include "systemsettings_global.h"

class SYSTEMSETTINGSSHARED_EXPORT SystemSettings
{

public:
    SystemSettings();
    const QStringList getInterfaces();

protected:
    void discoveryInterfaces();

private:
    QStringList m_interfaces;
};

#endif // SYSTEMSETTINGS_H
