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

class PowerSupplyM: public QObject
{
        Q_OBJECT
    public:
        PowerSupplyM(SerialComM *serialComHandler);
        bool PowerON();
        bool PowerOFF();
        bool RequestPowerState();

    private:
        SerialComM *serial = Q_NULLPTR;
        bool SendPacket(quint16 packetType, QByteArray packetBytes);

    private slots:
        void OnPacketReceived(quint16 packetType, QByteArray packetBytes);

    signals:
        void PowerStatusChanged(power_supply_state_t newState);

};

#endif // POWERSUPPLYM_H
