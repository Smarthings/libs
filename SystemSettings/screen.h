#ifndef SCREEN_H
#define SCREEN_H

#include <QObject>
#include "logs.h"

class Screen : public Logs
{
    Q_OBJECT
    Q_PROPERTY(quint16 brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)

public:
    explicit Screen();

signals:
    /**
     * @brief emit signal chage brightness
     */
    void brightnessChanged();

public slots:
    /**
     * @brief get brightness
     */
    quint16 brightness() { return m_brightness; }
    /**
     * @brief setBrightness
     * @param quint16 _brightness : brightness number
     */
    void setBrightness(quint16 _brightness);

private slots:
    /**
     * @brief get brightness from file in system
     */
    void getBrightness();

protected:
    //const QString m_file_brightness = "/sys/class/backlight/rpi_backlight/brightness";
    const QString m_file_brightness = "/tmp/brightness";
    quint16 m_brightness = 20;
};

#endif // SCREEN_H
