#ifndef INFO_H
#define INFO_H

#include <QObject>
#include <QList>
#include <QJsonObject>
#include "logs.h"

class Info : public Logs
{
    Q_OBJECT

public:
    explicit Info();
    ~Info();

signals:

public slots:
    void restartSystem();
};

#endif // INFO_H
