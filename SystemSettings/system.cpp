#include "system.h"
#include <QProcess>

System::System()
{

}

void System::restartSystem()
{
    QProcess process;
    process.start("/bin/bash -c \"reboot\"");
    if (!process.waitForFinished())
        setError(QString("restartSystem error: %1").arg(process.errorString()));
    process.close();
}
