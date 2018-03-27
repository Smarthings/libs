#include "screen.h"
#include <QFile>
#include <QProcess>
#include <QDebug>

Screen::Screen()
{
    getBrightness();
}

void Screen::setBrightness(quint16 _brightness)
{
    QString command = QString("/bin/bash -c \"echo %1 > /%2\"").arg(_brightness).arg(m_file_brightness);
    QProcess process;
    process.start(command);

    if (!process.waitForFinished())
        setError(QString("setBrightness error: %1").arg(process.errorString()));
    process.close();
}

void Screen::getBrightness()
{
    QString command = QString("/bin/bash -c \"cat %1\"").arg(m_file_brightness);
    QProcess proccess;
    proccess.start(command);

    if (!proccess.waitForFinished())
        setError(QString("getBrightness error: %1").arg(proccess.errorString()));
    else
    {
        m_brightness = proccess.readLine().trimmed().toUInt();
        emit brightnessChanged();
    }
    proccess.close();
}
