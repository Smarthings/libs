#ifndef LOGS_H
#define LOGS_H

#include <QObject>

class Logs : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString error READ error WRITE setError NOTIFY errorChanged)

public:
    explicit Logs(QObject *parent = nullptr);
    QString error() { return m_error; }
    void setError(QString err);

signals:
    void errorChanged();

public slots:
    void printError();

protected:
    QString m_error;
};

#endif // LOGS_H
