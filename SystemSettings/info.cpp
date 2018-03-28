#include "info.h"
#include <QProcess>
#include <QDebug>
#include <QSysInfo>

Info::Info()
{
    QSysInfo info;
    qDebug() << info.buildAbi() << info.buildCpuArchitecture() << info.currentCpuArchitecture()
             << info.kernelType() << info.kernelVersion() << info.machineHostName()
             << info.prettyProductName() << info.productType() << info.productVersion();
}

void Info::restartSystem()
{
    QProcess process;
    process.start("/bin/bash -c \"reboot\"");
    if (!process.waitForFinished())
        setError("restartSystem error: reiniciar o sistema");
    process.close();
}

Info::~Info()
{

}
