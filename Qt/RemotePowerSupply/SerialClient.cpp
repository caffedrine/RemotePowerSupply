#include "SerialClient.h"

SerialClient::SerialClient()
{
}

SerialClient::~SerialClient()
{
    if(this->isOpen())
	{
        this->disconnect();
		delete pSerialPort;
		pSerialPort = Q_NULLPTR;
	}
}

QString SerialClient::getLastError()
{
	return this->lastError;
}

void SerialClient::setLastError(QString error)
{
	this->lastError = error;
}

bool SerialClient::connect(QString portName, BaudRate baudRate)
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
	QObject::connect(pSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(connectionStatusChanged(QSerialPort::SerialPortError)));
	QObject::connect(pSerialPort, SIGNAL(readyRead()), this, SIGNAL(readyRead()));


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

bool SerialClient::disconnect()
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

void SerialClient::connectionStatusChanged(QSerialPort::SerialPortError errNo)
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

qint64 SerialClient::write(QByteArray writeData)
{
    if(!this->isOpen())
    {
        this->setLastError("Port is not open for writing!");
        return -1;
    }

    if(writeData.length() == 0)
    {
        this->setLastError("Invalid 0 length data to send!");
        return -1;
    }

    if( this->isOpen() )
	{
        return pSerialPort->write( writeData );
	}
	else
	{
		this->setLastError(pSerialPort->errorString());
		return -1;
	}
}

QList<QString> SerialClient::getSerialPorts()
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

QString SerialClient::readString()
{
    if(!this->isOpen())
    {
        this->setLastError("Port is not oppened for reading!");
        return "-1";
    }

    /* Read data */
    QByteArray chunk;
    static QByteArray recvBytes;

    if(pSerialPort->readChannelCount() <= 0)
        return "" ;

    /* Receive a chunk and then validate it */
    chunk = pSerialPort->readAll();
    if(chunk.length() <= 0)
        return "";

    /* Copy recv chunk in the static recvBuffer */
    for(int i=0; i < chunk.length(); i++)
    {
            recvBytes.append(chunk.at(i));
    }

    /* Store data until a complete line is received */
    bool foundNewline = false;
    for(int i=0; i < chunk.length(); i++)
    {
        if(chunk.at(i) == '\0')
        {
            foundNewline = true;
            break;
        }
    }
    if(!foundNewline)
        return "";

    /* Store recv line and clear recv buffer for further readings */
    QString line = QString( recvBytes );

    recvBytes.clear();

    /* Return line */
    return line;
}

QString SerialClient::readLine()
{
    if(!this->isOpen())
    {
        this->setLastError("Port is not oppened for reading!");
        return "-1";
    }

    /* Read data */
    QByteArray chunk;
    static QByteArray recvBytes;

    if(pSerialPort->readChannelCount() <= 0)
        return "" ;

    /* Receive a chunk and then validate it */
    chunk = pSerialPort->readLine();
    if(chunk.length() <= 0)
        return "";

    /* Copy recv chunk in the static recvBuffer */
    for(int i=0; i < chunk.length(); i++)
    {
        if(chunk.at(i) != '\x00')           /* Ignore string termination */
            recvBytes.append(chunk.at(i));
    }

    /* Store data until a complete line is received */
    bool foundNewline = false;
    for(int i=0; i < chunk.length(); i++)
    {
        if(chunk.at(i) == '\n')
        {
            foundNewline = true;
            break;
        }
    }
    if(!foundNewline)
        return "";

    /* Store recv line and clear recv buffer for further readings */
    QString line = QString( recvBytes );

    //qDebug() << recvBytes;

    recvBytes.clear();

    /* Return line */
    return line;
}

bool SerialClient::isOpen()
{
    if(this->pSerialPort == Q_NULLPTR || this->pSerialPort == 0)
        return false;

	if(pSerialPort->isOpen() && pSerialPort->isWritable() && pSerialPort->isReadable())
		return true;
	return false;
}

qint64 SerialClient::read(char *buffer, qint64 maxLen)
{
    if(!this->isOpen())
    {
        this->setLastError("Port is not oppened for reading!");
        return -1;
    }

    qint64 readBytes = pSerialPort->read(buffer, maxLen);
    return readBytes;
}


