#include "info.h"
#include <QDebug>
#include <QSysInfo>

Info::Info()
{
    QSysInfo info;
    qDebug() << info.buildAbi() << info.buildCpuArchitecture() << info.currentCpuArchitecture()
             << info.kernelType() << info.kernelVersion() << info.machineHostName()
             << info.prettyProductName() << info.productType() << info.productVersion();
}

Info::~Info()
{

}
