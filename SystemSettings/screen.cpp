#include "screen.h"
#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QRegularExpressionMatch>
#include <QRegularExpression>

Screen::Screen()
{
    getBrightness();
    getRotation();
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

void Screen::setRotation(quint16 _rotation)
{
    QString lcd_rotate = QString("lcd_rotate=%1").arg(_rotation? 2 : 0);
    QFile file(m_file_config);
    if (!file.open(QIODevice::ReadWrite))
        setError(QString("setRotation error: %1").arg(file.errorString()));
    else
    {
        QString file_content = file.readAll();
        QRegularExpression find_lcd_rotate("(lcd_rotate=\\d)");
        QRegularExpressionMatch match_lcd_rotate = find_lcd_rotate.match(file_content);
        if (match_lcd_rotate.hasMatch())
        {
            file_content.replace(find_lcd_rotate, lcd_rotate);
        }
        else
        {
            file_content.append(QString("\n%1\n").arg(lcd_rotate));
        }

        file.resize(0);
        file.seek(0);
        file.write(file_content.toUtf8());

        file.flush();
    }
    file.close();
}

void Screen::getBrightness()
{
    QFile file(m_file_brightness);
    if (!file.open(QIODevice::ReadOnly))
        setError(QString("getBrightness error: %1").arg(file.errorString()));
    else
    {
        m_brightness = file.readAll().trimmed().toUInt();
        file.flush();
        emit brightnessChanged();
    }
    file.close();
}

void Screen::getRotation()
{
    QFile file(m_file_config);
    if (!file.open(QIODevice::ReadOnly))
        setError(QString("getRotation error: %1").arg(file.errorString()));
    else
    {
        while (!file.atEnd()) {
            QString line = file.readLine().trimmed();
            QRegularExpression lcd_rotate("(lcd_rotate=\\d)");
            QRegularExpressionMatch match_lcd_rotate = lcd_rotate.match(line);
            if (match_lcd_rotate.hasMatch())
            {
                m_rotation = match_lcd_rotate.captured().split("=")[1].toUInt();
                emit rotationChanged();
                break;
            }
        }
        file.flush();
    }
    file.close();
}
