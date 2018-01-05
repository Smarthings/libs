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
    else
    {
        m_brightness = _brightness;
        emit brightnessChanged();
    }
    process.close();
}

void Screen::getBrightness()
{
    QFile brightness_file(m_file_brightness);
    if (!brightness_file.open(QIODevice::ReadOnly))
    {
        setError(QString("brightness error: %1").arg(brightness_file.errorString()));
        return;
    }
    m_brightness = brightness_file.readLine().toUInt();
    emit brightnessChanged();
}
