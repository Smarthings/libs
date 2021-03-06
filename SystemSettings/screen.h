#ifndef SCREEN_H
#define SCREEN_H

#include <QObject>
#include "logs.h"

class Screen : public Logs
{
    Q_OBJECT
    Q_PROPERTY(quint16 brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(quint16 rotation READ rotation WRITE setRotation NOTIFY rotationChanged)

public:
    explicit Screen();

signals:
    /**
     * @brief emit signal chage brightness
     */
    void brightnessChanged();
    /**
     * @brief rotationChanged
     */
    void rotationChanged();

public slots:
    /**
     * @brief setBrightness
     * @param quint16 _brightness : brightness number
     */
    void setBrightness(quint16 _brightness);
    /**
     * @brief setRotation
     * @param _rotation
     */
    bool setRotation(quint16 _rotation);
    /**
     * @brief get brightness
     */
    quint16 brightness() { return m_brightness; }
    /**
     * @brief rotation
     * @return
     */
    quint16 rotation() { return m_rotation; }

private slots:
    /**
     * @brief get brightness from file in system
     */
    void getBrightness();
    /**
     * @brief getRotation
     */
    void getRotation();

protected:
    const QString m_file_brightness = "/sys/class/backlight/rpi_backlight/brightness";
    const QString m_file_config = "/boot/config.txt";
//    const QString m_file_brightness = "/tmp/brightness";
//    const QString m_file_config = "/tmp/config.txt";

    quint16 m_brightness = 20;
    quint16 m_rotation = 0;
};

#endif // SCREEN_H
