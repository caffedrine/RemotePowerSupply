#include "Arduino.h"
#include <GpioBase.h>
#include <LED.h>

#define PIN_PWR_SUPPLY_CONTROL		8u
#define PIN_PWR_SUPPLY_READ_STATUS	A0

class PowerSupplyManager
{
public:
	uint16_t POWER_TRANSITION_TIMEOUT_MS = 600;

	typedef enum
	{
		POWER_OFF,
		POWER_ON,
		POWER_INVALID,
	}power_supply_state_t;

	PowerSupplyManager(uint8_t PinControl, uint8_t PinReadStatus)
	{
		this->PwrControl = new Drivers::GpioBase(PinControl, OUTPUT);
		this->PwrStatus = new Drivers::GpioBase(PinReadStatus, INPUT);
	}

	~PowerSupplyManager()
	{
		if (this->PwrStatus != nullptr)
		{
			delete this->PwrStatus;
		}

		if (this->PwrControl != nullptr)
		{
			delete this->PwrControl;
		}
	}

	bool PowerON()
	{
		if( this->CommandState == CMD_STATE_WAIT_PWR_ON)
			return true;
		else if( this->CommandState != CMD_STATE_IDLE)
			return false;

		// If already powered on, then just return positive
		if( this->PowerSupplyState == POWER_ON )
			return true;

		// Trigger the relay to enable the power supply
		this->PwrControl->Write(HIGH);

		this->CommandStartTimestamp = millis();
		this->CommandState = CMD_STATE_WAIT_PWR_ON;

		return true;
	}

	bool PowerOFF()
	{
		if( this->CommandState == CMD_STATE_WAIT_PWR_OFF)
			return true;
		else if( this->CommandState != CMD_STATE_IDLE)
			return false;

		// If already powered off, then just return positive
		if( this->PowerSupplyState == POWER_OFF )
			return true;

		// Trigger the relay to disable the power supply
		this->PwrControl->Write(HIGH);

		this->CommandStartTimestamp = millis();
		this->CommandState = CMD_STATE_WAIT_PWR_OFF;

		return true;
	}

	bool PowerToggle()
	{
		if (this->PowerSupplyState == POWER_ON)
		{
			return this->PowerOFF();
		}
		else
		{
			return this->PowerON();
		}
	}

	power_supply_state_t GetPowerSupplyState()
	{
		return this->PowerSupplyState;
	}

	void SetNotificationCallbackOnStateChange(void (*func_ptr)(power_supply_state_t))
	{
		this->OnStateChangeCbkFunc = func_ptr;
	}

	void MainFunction()
	{
		// Read power status on every cycle
		this->ReadPowerSupplyState();

		// Handle waiting for power transitions
		if( this->CommandState == CMD_STATE_WAIT_PWR_OFF )
		{
			// Timeout occured or power transited?
			if( (millis() - this->CommandStartTimestamp >= this->POWER_TRANSITION_TIMEOUT_MS) ||
					this->PowerSupplyState == POWER_OFF)
			{
				this->CommandState = CMD_STATE_IDLE;
				this->PwrControl->Write(LOW);
			}
		}
		else if( this->CommandState == CMD_STATE_WAIT_PWR_ON )
		{
			// Timeout occured or power transited?
			if( (millis() - this->CommandStartTimestamp >= this->POWER_TRANSITION_TIMEOUT_MS) ||
					this->PowerSupplyState == POWER_ON)
			{
				this->CommandState = CMD_STATE_IDLE;
				this->PwrControl->Write(LOW);
			}
		}
	}

private:
	typedef enum
	{
		CMD_STATE_IDLE,
		CMD_STATE_WAIT_PWR_ON,
		CMD_STATE_WAIT_PWR_OFF
	}command_state_t;

	// State machine used to process commands send to power supply
	command_state_t CommandState = CMD_STATE_IDLE;
	// Store the time when command started (needed for timeout)
	uint64_t CommandStartTimestamp = 0;

	// Record power supply state in this variable
	power_supply_state_t PowerSupplyState = POWER_INVALID;

	// Pins handlers
	// Create structures with pins used
	Drivers::GpioBase *PwrStatus = nullptr;
	Drivers::GpioBase *PwrControl = nullptr;

	// Store last read analogic value from power supply
	uint16_t PwrLastAnalogicRead = 600;

	// Callback that can be set to be triggered when power state changes
	typedef void(*OnStateChangeCbk)(power_supply_state_t);
	OnStateChangeCbk OnStateChangeCbkFunc = nullptr;

	void ReadPowerSupplyState()
    {

		power_supply_state_t prevState = this->PowerSupplyState;

		// Update analogic val
		this->ReadPwrAnalogicVal();

		// Thresholds measured empirically
		if (this->PwrLastAnalogicRead < 550)
			this->PowerSupplyState = POWER_ON;
		else if( this->PwrLastAnalogicRead >  650 )
			this->PowerSupplyState = POWER_OFF;

		// Trigger callback in case was set
		if( (OnStateChangeCbkFunc != nullptr) && (prevState != this->PowerSupplyState) )
		{
			OnStateChangeCbkFunc(this->PowerSupplyState);
		}
    }

	void ReadPwrAnalogicVal()
	{
		uint16_t readVal = this->PwrStatus->ReadAnalog();
		if( readVal > this->PwrLastAnalogicRead)
			this->PwrLastAnalogicRead++;
		else if( readVal < this->PwrLastAnalogicRead )
			this->PwrLastAnalogicRead--;
	}
};

class SerialCommManager
{
public:
	SerialCommManager(HardwareSerial *srl, unsigned long baud)
	{
		this->serial = srl;
		this->serial->begin(baud);
	}

	virtual ~SerialCommManager()
	{

	}

	void SetCallbackOnCommandReceived(void(*func_ptr)(uint8_t *, uint16_t))
	{
		this->OnDataReceivedCbkFunc = func_ptr;
	}

	void SendPacket(String DataToBeSend, uint8_t packet_type)
	{
		String packet = "#";
		packet += String(DataToBeSend.length());
		packet += "@";
		packet += String(packet_type);
		packet += "$";
		packet += DataToBeSend;
		this->serial->println(packet);
	}

	void MainFunction()
	{
		// Timeout occurred for current packet?
		if( this->ReadingState !=  STATE_READ_WAIT_FIRST_TOKEN)
		{
			if ( millis() - this->ReadPacketStartTimestamp > this->ReadPacketTimeoutMs )
			{
				this->ReadingState = STATE_READ_WAIT_FIRST_TOKEN;
				Serial.println("Timeout");
			}
		}

		// Handle states only when we receive serial data
		if( this->serial->available() > 0 )
		{
			if( this->ReadingState == STATE_READ_WAIT_FIRST_TOKEN )       // #1
			{
				// Read first byte and check whether it is start symbol
				uint8_t readChar;
				this->serial->readBytes(&readChar, 1);
				if( readChar == '#')
				{
					this->ReadingState = STATE_WAIT_LENGTH;
					this->ReadPacketStartTimestamp = millis();

					// Reset timeout
					this->ReadPacketTimeoutMs = sizeof(this->readBytes) * this->ReadByteTimeoutMs;
				}
			}
			else if( this->ReadingState == STATE_WAIT_LENGTH )			  // #2
			{
				// At least two bytes are needed for get full length
				if( this->serial->available() >= 2 )
				{
					uint8_t lenMsb, lenLsb;

					this->serial->readBytes((char *)&lenMsb, 1);
					this->serial->readBytes((char *)&lenLsb, 1);

					// Build read length
					this->readLength = ((uint16_t)((uint16_t)lenMsb << 8)) | ((uint16_t)lenLsb);

					if( this->readLength > sizeof(readBytes) )
					{
						this->readLength = sizeof(readBytes);
					}

					// Calculate timeout based on total number of bytes that shall arrive
					this->ReadPacketTimeoutMs =  this->readLength * this->ReadByteTimeoutMs;

					this->readBytesIdx = 0;
					this->ReadingState = STATE_WAIT_PAYLOAD;

					//this->t1 = millis();
				}
			}
			else if( this->ReadingState == STATE_WAIT_PAYLOAD )			  // #3
			{
				this->serial->setTimeout(this->ReadPacketTimeoutMs);

				uint16_t bytesAvailable = (uint16_t)this->serial->available(); // can be safely converted as > 0  check was previously done
				uint16_t read_bytes_so_far = this->readBytesIdx;
				uint16_t remaining_buffer_size = sizeof(this->readBytes) - read_bytes_so_far;
				uint16_t remaining_bytes_to_be_read = this->readLength - read_bytes_so_far;

				if(remaining_bytes_to_be_read > remaining_buffer_size)
					remaining_bytes_to_be_read = remaining_buffer_size;

				uint16_t bytes_to_read_now = bytesAvailable >= remaining_bytes_to_be_read ? remaining_bytes_to_be_read : bytesAvailable;

//				Serial.print("Total bytes available: ");
//				Serial.println(bytesAvailable, DEC);
//				Serial.print("Bytes to read in this round: ");
//				Serial.println(bytes_to_read_now, DEC);


				uint16_t chunk_read_size = serial->readBytes(&this->readBytes[this->readBytesIdx], bytes_to_read_now);
				this->readBytesIdx += chunk_read_size;

				// All the bytes were already read?
				if( this->readBytesIdx >= this->readLength )
				{
					// Trigger reception event
					this->OnPacketRead(this->readBytes, this->readLength);

					// Reset state machine
					this->ReadingState = STATE_READ_WAIT_FIRST_TOKEN;
					this->readBytesIdx = 0;
					this->readLength = 0;

//					Serial.println("Time elapsed2: ");
//					Serial.println( (int)(millis() - this->t1), DEC);
				}
			}
		}
	}

protected:
	virtual void OnPacketRead(uint8_t *readBytes, uint16_t readLength)
	{
		if( this->OnDataReceivedCbkFunc != nullptr )
		{
			this->OnDataReceivedCbkFunc(readBytes, readLength);
		}
	}

private:
	HardwareSerial *serial;

	typedef enum
	{
		STATE_READ_WAIT_FIRST_TOKEN,
		STATE_WAIT_LENGTH,
		STATE_WAIT_PAYLOAD,
	}packet_read_state_t;

	packet_read_state_t ReadingState = STATE_READ_WAIT_FIRST_TOKEN;

	// Buffers used to store data read
	uint16_t readLength = 0;
	uint16_t readBytesIdx = 0;
	uint8_t readBytes[256];

	// Remember when packet read started
	uint64_t ReadPacketStartTimestamp = 0;

	// Timeout to wait for a single byte
	uint64_t ReadByteTimeoutMs = 4;

	// Timeout to wait for a single packet
	uint64_t ReadPacketTimeoutMs = sizeof(this->readBytes) * 4;

	// Callback that can be set to be triggered when complete data is received
	typedef void(*OnDataReceivedCbk)(uint8_t *, uint16_t);
	OnDataReceivedCbk OnDataReceivedCbkFunc = nullptr;

	// Debugging time consumption
	uint64_t t1 = 0, t2 = 0;
};

// Func prototypes
void SerialOnCommandReceived(uint8_t *data, uint16_t data_len);
void PowerSupplyOnStateChange(PowerSupplyManager::power_supply_state_t new_state);

// Create a handler for power supply
Drivers::LED led(13); // debugging led
PowerSupplyManager powerSupply(PIN_PWR_SUPPLY_CONTROL, PIN_PWR_SUPPLY_READ_STATUS);
SerialCommManager comm(&Serial, 9600);

void setup()
{
	// Init serial
	Serial.begin(9600);

	// Set communication callback notification
	comm.SetCallbackOnCommandReceived(&SerialOnCommandReceived);

	// Attach callback notification to power supply to get notifications when power state changes
	powerSupply.SetNotificationCallbackOnStateChange(&PowerSupplyOnStateChange);
}

void SerialOnCommandReceived(uint8_t *data, uint16_t data_len)
{
	// Convert to string and check whether it is a valid command
	String serial_command = "";
	for( uint16_t i = 0; i < data_len; i++ )
		serial_command += String((char)data[i]);


	if( serial_command == "PWR_ON" )
	{
		comm.SendPacket( powerSupply.PowerON()?"1":"0", 0);
	}
	else if( serial_command == "PWR_OFF")
	{
		comm.SendPacket( powerSupply.PowerOFF()?"1":"0", 0);
	}
	else if (serial_command == "?")
	{
		comm.SendPacket( (powerSupply.GetPowerSupplyState() == PowerSupplyManager::POWER_ON) ? "ON" : "OFF", 1);
	}
}

void PowerSupplyOnStateChange(PowerSupplyManager::power_supply_state_t new_state)
{
	comm.SendPacket( (new_state == PowerSupplyManager::POWER_ON)? "ON" : "OFF", 1);
}

// The loop function is called in an endless loop
uint64_t prevMillis;
void loop()
{
	// Handle main function of power supply
	powerSupply.MainFunction();

	// Toggle power supply
	comm.MainFunction();

//	if( millis() - prevMillis >= 200 )
//	{
//		prevMillis = millis();
//		Serial.print("Power status: ");
//		Serial.println(powerSupply.GetPowerSupplyState());
//	}
}
