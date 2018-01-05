#include "screen.h"
#include <QFile>
#include <QProcess>

Screen::Screen(QObject *parent) : QObject(parent)
{
    getBrightness();
}

quint16 Screen::setBrightness(quint16 _brightness)
{
    QString command = QString("/bin/bash -c \"echo %1 > /%2\"").arg(_brightness).arg(m_file_brightness);
    QProcess process;
    process.start(command);

    if (!process.waitForFinished())
    {
        m_error = QString("setBrightness error: %1").arg(process.errorString());
        emit errorChanged();
    }
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
        m_error = QString("brightness error: %1").arg(brightness_file.errorString());
        emit errorChanged();
        return;
    }
    m_brightness = brightness_file.readLine().toUInt();
    emit brightnessChanged();
}
