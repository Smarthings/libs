#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include "./logs.h"

class System : public Logs
{
    Q_OBJECT
public:
    explicit System();

signals:

public slots:
    /**
     * @brief restartSystem
     */
    void restartSystem();
};

#endif // SYSTEM_H
