#include "TcpComM.h"

TcpComM::TcpComM(QHostAddress address, quint16 tcp_port, quint32 timeout_ms): TcpAddress(address), TcpPort(tcp_port), TcpClientTimeoutMs(timeout_ms)
{
    this->server = new QTcpServer(this);
    this->server->setMaxPendingConnections(1);

    // whenever a user connects, it will emit signal
    connect(this->server, SIGNAL(newConnection()), this, SLOT(newConnection()));

    if(!this->server->listen(this->TcpAddress, this->TcpPort))
    {
        qDebug().nospace().noquote() << "[TCP SERVER] TCP server could not start on "<<address.toString()<<":"<<tcp_port;
    }
    else
    {
        qDebug().nospace().noquote()<< "[TCP SERVER] TCP server started on "<<address.toString()<<":"<<tcp_port;
    }

    if( this->TcpClientTimeoutMs > 0 )
    {

        this->ClientTimeoutTimer = new QTimer();

        // Connect the timeout timer
        connect(this->ClientTimeoutTimer, SIGNAL(timeout()), this, SLOT(clientTimeoutReached()));
    }
}

TcpComM::~TcpComM()
{
    if( this->client != Q_NULLPTR )
    {
        if (this->client->isOpen())
            this->client->close();
        this->client = Q_NULLPTR;
    }

    if( this->server != Q_NULLPTR)
    {
        delete this->server;
        this->server = Q_NULLPTR;
        qDebug().nospace().noquote()<< "[TCP SERVER] TCP server stopped";
    }

    // Delete timer if it was created
    if( this->ClientTimeoutTimer != Q_NULLPTR )
    {
        delete this->ClientTimeoutTimer;
        this->ClientTimeoutTimer = Q_NULLPTR;
    }
}

void TcpComM::newConnection()
{
    // Client cannot stay connected more than one second

    if(!this->client || !this->client->isOpen())
    {
        // need to grab the socket
        this->client = this->server->nextPendingConnection();

        connect(this->client, SIGNAL(disconnected()),this, SLOT(clientDisconnected()));
        connect(this->client, SIGNAL(readyRead()),this, SLOT(clienDataAvailable()));

        emit this->TcpClientConnected(this->client);

        // Start a timer and disconnect client if it does not send data within 1s
        if( this->TcpClientTimeoutMs > 0 )
        {
            this->ClientTimeoutTimer->start(this->TcpClientTimeoutMs);
            this->ClientTimeoutTimer->blockSignals(false);
        }
    }
    else
    {
        this->server->nextPendingConnection()->close();
    }
}

void TcpComM::clientDisconnected()
{
    this->ClientTimeoutTimer->blockSignals(true);
    this->ClientTimeoutTimer->stop();

    emit this->TcpClientDisconnected(this->client);
    this->client = Q_NULLPTR;
}

void TcpComM::clienDataAvailable()
{
    emit this->TcpClientDataReception(this->client, this->client->readAll());

    if( this->TcpClientTimeoutMs > 0 )
    {
        this->ClientTimeoutTimer->start(this->TcpClientTimeoutMs);
    }
}

void TcpComM::clientTimeoutReached()
{
    // Timer was stopped, yet it still emits this. Make sure it is not processed in this case
    if( !this->ClientTimeoutTimer->isActive() )
        return;

    this->ClientTimeoutTimer->stop();
    qDebug().nospace().noquote()<< "[TCP SERVER] Close "<<this->client->localAddress().toString()<<":"<<this->client->localPort()<<" due to timout greater than "<<this->TcpClientTimeoutMs<<" ms";
    this->client->write("Timeout reached, bye!");
    this->client->disconnectFromHost();
}
