#include "PowerSupplyM.h"

PowerSupplyM::PowerSupplyM(SerialComM *serialComHandler): serial(serialComHandler)
{
    // connect serial connection connection signals
    connect(this->serial, SIGNAL(packetReceived(quint16, QByteArray)), this, SLOT(OnPacketReceived(quint16, QByteArray)));
}

bool PowerSupplyM::PowerON()
{
    if( !this->serial )
        return false;

    return this->serial->SendPacket(1, QString("PWR_ON").toUtf8());
}

bool PowerSupplyM::PowerOFF()
{
    if( !this->serial )
        return false;

    return this->serial->SendPacket(1, QString("PWR_OFF").toUtf8());
}

bool PowerSupplyM::RequestPowerState()
{
    if( !this->serial )
        return false;

    return this->serial->SendPacket(1, QString("?").toUtf8());
}

void PowerSupplyM::OnPacketReceived(quint16 packetType, QByteArray packetBytes)
{
    if( packetType == 0x00 )        // Packet type 0 is ACK
    {
        qDebug() << "SERIAL ACK: " << packetBytes;
    }
    else if( packetType == 0x1)     // Packet type 1 is answer
    {
        qDebug() << "Received answer packet: " << packetBytes;

        // Convert to string response
        QString response = QString(packetBytes);
        if( response == "ON" )
        {
            emit this->PowerStatusChanged(POWER_ON);
        }
        else if( response == "OFF" )
        {
            emit this->PowerStatusChanged(POWER_OFF);
        }
        else
        {
            qDebug() << "Invalid packet answer received: " << response;
            emit this->PowerStatusChanged(POWER_INVALID);
        }

    }
    else // Unexpected packet type
    {
        qDebug() << "Unexpected packet type '"<<packetType<<"' with payload: " << packetBytes;
    }
}


