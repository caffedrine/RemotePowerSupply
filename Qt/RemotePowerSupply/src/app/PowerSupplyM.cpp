#include "PowerSupplyM.h"

PowerSupplyM::PowerSupplyM(SerialComM *serialComHandler): serial(serialComHandler)
{
    // connect serial connection connection signals
    connect(this->serial, SIGNAL(packetReceived(quint16, QByteArray)), this, SLOT(OnPacketReceived(quint16, QByteArray)));
}

bool PowerSupplyM::PowerON()
{
    if( this->serial == Q_NULLPTR)
        return false;

    return this->serial->SendPacket(1, QString("PWR_ON").toUtf8());
}

bool PowerSupplyM::PowerOFF()
{
    if( this->serial == Q_NULLPTR)
        return false;

    return this->serial->SendPacket(1, QString("PWR_OFF").toUtf8());
}

bool PowerSupplyM::RequestPowerState()
{
    if( this->serial == Q_NULLPTR)
        return false;

    return this->serial->SendPacket(1, QString("?").toUtf8());
}

power_supply_state_t PowerSupplyM::ReadPowerState()
{
    return this->StatePowerSupply;
}

void PowerSupplyM::OnPacketReceived(quint16 packetType, QByteArray packetBytes)
{
    if( packetType == SerialComM::PKT_TYPE_ACK )        // Packet type 0 is ACK
    {
        // do nothing here
    }
    else if( packetType == SerialComM::PKT_TYPE_DATA)     // Packet type 1 is answer
    {
        // Convert to string response
        QString response = QString(packetBytes);
        if( response == "ON" )
        {
            this->StatePowerSupply = POWER_ON;
            emit this->PowerStatusChanged(POWER_ON);
        }
        else if( response == "OFF" )
        {
            this->StatePowerSupply = POWER_OFF;
            emit this->PowerStatusChanged(POWER_OFF);
        }
        else
        {
            this->StatePowerSupply = POWER_INVALID;
            qWarning().nospace().noquote() << "[PS MANAGER] Invalid packet answer received: " << response;
            emit this->PowerStatusChanged(POWER_INVALID);
        }
    }
    else if ( packetType == SerialComM::PKT_TYPE_ERROR )
    {

    }
    else // Unexpected packet type
    {
        qWarning().nospace().noquote() << "[PS MANAGER] Unexpected packet type '"<<packetType<<"' with payload: " << packetBytes;
    }
}


