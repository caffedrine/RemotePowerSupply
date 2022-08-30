#ifndef TCPCOMM_H
#define TCPCOMM_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>

class TcpComM: public QObject
{
        Q_OBJECT
    public:
        TcpComM(QHostAddress address, quint16 tcp_port, quint32 client_timeout_ms = 30000);
        ~TcpComM();

    signals:

    public slots:
        void newConnection();
        void clientDisconnected();
        void clienDataAvailable();
        void clientTimeoutReached();

    private:
        QTimer *ClientTimeoutTimer = Q_NULLPTR;

        quint32 TcpClientTimeoutMs;
        QHostAddress TcpAddress;
        quint16 TcpPort;

        QTcpServer *server = Q_NULLPTR;
        QTcpSocket *client = Q_NULLPTR;

    signals:
        void TcpClientConnected(QTcpSocket *client);
        void TcpClientDisconnected(QTcpSocket *client);
        void TcpClientDataReception(QTcpSocket *client, QByteArray bytes);
};

#endif // TCPCOMM_H
