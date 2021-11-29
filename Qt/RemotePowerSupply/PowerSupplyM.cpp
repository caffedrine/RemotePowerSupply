#include "PowerSupplyM.h"

PowerSupplyM::PowerSupplyM(SerialComM *serialComHandler): serial(serialComHandler)
{

}

bool PowerSupplyM::PowerON()
{
    if( !this->serial )
        return false;

    return true;
}

bool PowerSupplyM::PowerOFF()
{
    if( !this->serial )
        return false;

    return true;
}

power_supply_state_t PowerSupplyM::GetPowerState()
{
    if( !this->serial )
        return POWER_INVALID;

    return POWER_INVALID;
}
