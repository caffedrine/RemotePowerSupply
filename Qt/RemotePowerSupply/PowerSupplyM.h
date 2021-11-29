#ifndef POWERSUPPLYM_H
#define POWERSUPPLYM_H

#include <QObject>
#include <QDebug>

#include "SerialComM.h"

typedef enum
{
    POWER_OFF,
    POWER_ON,
    POWER_INVALID
} power_supply_state_t;

class PowerSupplyM: QObject
{
        Q_OBJECT
    public:
        PowerSupplyM(SerialComM *serialComHandler);
        bool PowerON();
        bool PowerOFF();
        power_supply_state_t GetPowerState();

    private:
        SerialComM *serial = Q_NULLPTR;

};

#endif // POWERSUPPLYM_H
