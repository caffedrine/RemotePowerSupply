#include "SerialComM.h"

SerialComM::SerialComM()
{
}

SerialComM::~SerialComM()
{
    if(this->isOpen())
	{
        this->disconnect();
		delete pSerialPort;
		pSerialPort = Q_NULLPTR;
	}
}

QString SerialComM::getLastError()
{
	return this->lastError;
}

void SerialComM::setLastError(QString error)
{
	this->lastError = error;
}

bool SerialComM::connect(QString portName, BaudRate baudRate)
{
	//Make sure we are not already connected
    if(this->isOpen())
	{
        this->setLastError("Port must be gracefully closed before any reconnect attempt!");
        return false;
	}

	if(portName.isEmpty())	//If you pass an empty name, pSerialPort will crash!
	{
        this->setLastError("Don't ever send me an empty name again! Empty names crash me :(!!!");
		return false;
	}

	//Create a new instance
	pSerialPort = new QSerialPort();

	//Connecting signals to slots
	QObject::connect(pSerialPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(connectionStatusChanged(QSerialPort::SerialPortError)));
    QObject::connect(pSerialPort, SIGNAL(readyRead()), this, SLOT(readyRead()));


    /* Set serial port name */
    pSerialPort->setPortName(portName);

    try
	{
		//Open RW Sessions
        pSerialPort->open(QIODevice::ReadWrite);
        /* Set baudrate */
        pSerialPort->setBaudRate(baudRate);

		//Check if connection succeed
        if(this->isOpen())
		{            
			//Clean buffers to throw away garbage from the wires - but it is now working :(
			pSerialPort->flush();
			pSerialPort->clear();
			return true;
		}
		else
		{
			this->setLastError(pSerialPort->errorString());
			return false;
		}
	}
	catch(...)
	{
		this->setLastError("Fatal error when called serialPort.open() function");
		return false;
	}
}

bool SerialComM::disconnect()
{
    if(pSerialPort == Q_NULLPTR)
		return true;

	pSerialPort->close();
    pSerialPort->disconnect();

    if(this->isOpen())
        return false;

    delete pSerialPort;
    pSerialPort = Q_NULLPTR;

    return true;
}

void SerialComM::connectionStatusChanged(QSerialPort::SerialPortError errNo)
{
    if(errNo == 0)
    {
        emit connectionStatusChanged(true);
    }
    else
    {
        emit connectionStatusChanged(false);
    }
}

void SerialComM::readyRead()
{
    ///
    /// Packet format:
    /// #<length>@<packet_type>$<payload>
    ///
    ///

    // Check timeout for one packet read
    if( (this->ReadState != WAIT_START_TAG) && (600 < (QDateTime::currentMSecsSinceEpoch() - this->ReadTimeoutMs)) )
    {
        qDebug().nospace().noquote() << "[SERIAL ComM] Packet read timout reached";
        this->ReadState = WAIT_START_TAG;
    }

    // Read serial data and append to current buffer
    this->ReadDataBuffer += this->pSerialPort->readAll();
    //qDebug().nospace().noquote()<< "SERIAL RECV: " << this->pSerialPort->readAll();

    // Check state. Wait for a fragmented packet or for a new packet
    if( this->ReadState == WAIT_START_TAG )
    {
        // Read start token and remove all data form buffe runtil reaching it
        if(this->ReadDataBuffer.indexOf('#') >= 0)
        {
            this->ReadDataBuffer = this->ReadDataBuffer.mid( this->ReadDataBuffer.indexOf('#') + 1 );

            // token found, advance the state and start timeout
            this->ReadState = WAIT_LENGTH;
            this->ReadTimeoutMs = QDateTime::currentMSecsSinceEpoch();
        }
        else
        {
            // Data is junk since start token is not found, so discard it
            this->ReadDataBuffer = QByteArray();
            qWarning().nospace().noquote() << "[SERIAL ComM] Junk data discarded on phase WAIT_START_TAG";

        }
    }


    if( this->ReadState == WAIT_LENGTH )
    {
        if( this->ReadDataBuffer.indexOf('@') >= 0 )
        {
            this->CurrPacket.Length = this->ReadDataBuffer.left(this->ReadDataBuffer.indexOf('@')).toInt();

            // Remove the bytes already read and proceeed with next state
            this->ReadDataBuffer.remove(0, this->ReadDataBuffer.indexOf('@') + 1);
            this->ReadState = WAIT_PACKET_TYPE;
        }
        else
        {
            qWarning().nospace().noquote() << "[SERIAL ComM] Junk data discarded on phase WAIT_LENGTH";
        }
    }

    if( this->ReadState == WAIT_PACKET_TYPE )
    {
        if( this->ReadDataBuffer.indexOf('$') >= 0 )
        {
            QString typeStr = this->ReadDataBuffer.left(this->ReadDataBuffer.indexOf('$'));
            this->CurrPacket.Type = typeStr.toInt();

            // Remove the bytes already read and proceeed with next state
            this->ReadDataBuffer.remove(0, this->ReadDataBuffer.indexOf('$') + 1);
            this->ReadState = WAIT_PACKET_PAYLOAD;
        }
        else
        {
            qWarning().nospace().noquote() << "[SERIAL ComM] Junk data discarded on phase WAIT_PACKET_TYPE";
        }
    }

    if( this->ReadState == WAIT_PACKET_PAYLOAD )
    {
        if( this->ReadDataBuffer.count() >= this->CurrPacket.Length )
        {
            this->CurrPacket.Payload = this->ReadDataBuffer.left(this->CurrPacket.Length);

            // Remove processed bytes from buffer
            this->ReadDataBuffer.remove(0, this->CurrPacket.Length);

            qDebug().nospace().noquote() << "[SERIAL ComM] RECV PACKET <: @" << this->CurrPacket.Type << "$" << this->CurrPacket.Payload << (this->CurrPacket.Type == ACK ? " (ACK)":"");

            // Emit signal that a packet was received
            emit this->packetReceived(this->CurrPacket.Type, this->CurrPacket.Payload);

            // Reset state
            this->ReadState = WAIT_START_TAG;
        }
        else
        {
            qWarning().nospace().noquote() << "[SERIAL ComM] Junk data discarded on phase WAIT_PACKET_PAYLOAD (length bigger than the one received)";
        }
    }
}

QList<QString> SerialComM::getSerialPorts()
{
	QList<QString> ports;

	//Get serial ports info
	QSerialPortInfo *portsInfo = new QSerialPortInfo;

	//Fetching available ports into a list
	QList<QSerialPortInfo> serialPorts = portsInfo->availablePorts();

	//Filling combo list
	foreach (QSerialPortInfo serialPort, serialPorts)
	{
		ports.push_back(serialPort.portName());
	}

	//clearing pointer we just created
	delete portsInfo;

	return ports;
}

bool SerialComM::isOpen()
{
    if(this->pSerialPort == Q_NULLPTR || this->pSerialPort == 0)
    {
        return false;
    }

	if(pSerialPort->isOpen() && pSerialPort->isWritable() && pSerialPort->isReadable())
    {
		return true;
    }

	return false;
}

bool SerialComM::SendPacket(quint16 packetType, QByteArray packetBytes)
{
    QByteArray packet;

    // Add start token
    packet.append('#');

    // Add packet length
    if( packetBytes.count() < 255 )
    {
        packet.append((char)(0));
        packet.append((char)(packetBytes.count()));
    }
    else
    {
        packet.append((char)(packetBytes.count() >> 8));
        packet.append((char)(packetBytes.count() & 0x00FF ));
    }

    // Add packet
    packet += packetBytes;

    if(  this->pSerialPort->write(packet) )
    {
        qDebug().nospace().noquote() << "[SERIAL ComM] SEND PACKET >: hex(" << packet.toHex(' ') << ")";
        return true;
    }
    else
    {
        qDebug().nospace().noquote() << "[SERIAL ComM] SEND PACKET ERROR >: hex(" << packet.toHex(' ') << ")";
        return false;
    }
}