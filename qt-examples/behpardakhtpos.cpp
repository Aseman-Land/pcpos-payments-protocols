#include "behpardakhtpos.h"

BehpardakhtPos::BehpardakhtPos(QObject *parent) : AbstractPCPos<BehpardakhtPos>(parent)
{
    setPort(1024);
}

BehpardakhtPos::~BehpardakhtPos()
{

}

void BehpardakhtPos::start(qint32 price, std::function<void (Status)> callback)
{
    if (mSocket)
        return;

    mOnPay = false;
    auto socket = new QTcpSocket(this);

    mSocket = socket;
    connect(socket, &QTcpSocket::connected, [this, callback, socket, price](){
        callback(Initializing);
        callback(Initializing);

        BehpardakhtPacket pk;
        pk.setPrice(price);
        pk.setOrder_id(321654);

        socket->write(toArray(pk.data()));
        socket->flush();

        mOnPay = true;
        callback(OnThePay);
    });
    connect(socket, &QTcpSocket::readyRead, [this, socket, callback, price](){
        const auto bytes = socket->readAll();

        BehpardakhtPacket pk;
        if (!BehpardakhtPacket::fromData( pk, toVector(bytes) ))
        {
            callback(Failed);
            mSocket->disconnectFromHost();
            return;
        }

        if (pk.price() == price)
            callback(PaidSuccessfully);
        else
            callback(Failed);
        mSocket->disconnectFromHost();
    });
    connect(socket, &QTcpSocket::disconnected, [this, callback, socket](){
        if (mSocket == socket)
            mSocket = Q_NULLPTR;

        socket->deleteLater();
        callback(Finished);
    });

    callback(Connecting);
    socket->connectToHost(hostAddress(), port());
}

void BehpardakhtPos::stop()
{
    if (!mSocket)
        return;

    mSocket->disconnectFromHost();
    mSocket = Q_NULLPTR;
}

QString BehpardakhtPos::displayName() const
{
    return tr("Behpardakht");
}

QByteArray BehpardakhtPos::toArray(const BehpardakhtPacket::PacketResult &list)
{
    QByteArray res;
    for (const auto l: list)
        res += (uchar)l;
    return res;
}

BehpardakhtPacket::PacketResult BehpardakhtPos::toVector(const QByteArray &bytes)
{
    BehpardakhtPacket::PacketResult res;
    for (const auto b: bytes)
        res.push_back(b);
    return res;
}
