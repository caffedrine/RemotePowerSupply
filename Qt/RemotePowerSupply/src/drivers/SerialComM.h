#ifndef SERIALCONN_H
#define SERIALCONN_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QList>
#include <QByteArray>
#include <QDebug>
#include <QTime>

class SerialComM: public QObject
{
	Q_OBJECT

public:
	typedef QSerialPort::BaudRate BaudRate;	//let user choose his own boudrate

    typedef enum
    {
        PKT_TYPE_ACK = 0,
        PKT_TYPE_DATA = 1,
        PKT_TYPE_ERROR = 2,
    } serial_comm_packet_type_t;

    typedef enum
    {
        WAIT_START_TAG,
        WAIT_LENGTH,
        WAIT_PACKET_TYPE,
        WAIT_PACKET_PAYLOAD
    } serial_comm_packet_read_state_t;

    typedef struct
    {
        uint16_t Length;
        uint16_t Type;
        QByteArray Payload;
    }serial_comm_packet_t;

    SerialComM();
    ~SerialComM();

private:
	//Serial port handler
	QSerialPort *pSerialPort = Q_NULLPTR;

	//Store last error
	QString lastError;

public:
    qint64 ReadTimeoutMs;
    serial_comm_packet_read_state_t ReadState = WAIT_START_TAG;
    QByteArray ReadDataBuffer;
    serial_comm_packet_t CurrPacket;

	bool connect(QString portName, BaudRate baudRate);
	bool disconnect();

	static QList<QString> getSerialPorts();
	bool isOpen();
    bool SendPacket(quint16 packetType, QByteArray packetBytes);
	//Other methods
	QString getLastError();

private:
	void setLastError(QString error);

private slots:
	void connectionStatusChanged(QSerialPort::SerialPortError errNo);
    void readyRead();

signals:
    void packetReceived(quint16 packetType, QByteArray packeBytes);
	void connectionStatusChanged(bool connected);
};

#endif // SERIALCONN_H
