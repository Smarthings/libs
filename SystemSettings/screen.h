#ifndef SCREEN_H
#define SCREEN_H

#include <QObject>

class Screen : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint16 brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    explicit Screen(QObject *parent = nullptr);

signals:
    /**
     * @brief emit signal chage brightness
     */
    void brightnessChanged();
    /**
     * @brief emit signal error
     */
    void errorChanged();

public slots:
    /**
     * @brief get brightness
     */
    quint16 brightness() { return m_brightness; }
    /**
     * @brief setBrightness
     * @param qiont16 _brightness : brightness number
     */
    quint16 setBrightness(quint16 _brightness);
    /**
     * @brief get message error
     */
    QString error() { return m_error; }

private slots:
    /**
     * @brief get brightness from file in system
     */
    void getBrightness();

protected:
    const QString m_file_brightness = "/sys/class/backlight/rpi_backlight/brightness";
    QString m_error = "";
    quint16 m_brightness = 20;
};

#endif // SCREEN_H
