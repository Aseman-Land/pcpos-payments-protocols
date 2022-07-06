#ifndef BEHPARDAKHTPOS_H
#define BEHPARDAKHTPOS_H

#include "pcposmanager.h"
#include "behpardakhtpacket.h"

#include <QTcpSocket>
#include <QPointer>

class BehpardakhtPos : public AbstractPCPos<BehpardakhtPos>
{
    Q_OBJECT
public:
    BehpardakhtPos(QObject *parent = nullptr);
    virtual ~BehpardakhtPos();

    void start(qint32 price, std::function<void (Status)> callback);
    void stop();
    bool running() const {
        return mSocket;
    }
    QString displayName() const;

protected:
    static QByteArray toArray(const BehpardakhtPacket::PacketResult &list);
    static BehpardakhtPacket::PacketResult toVector(const QByteArray &bytes);

private:
    QPointer<QTcpSocket> mSocket;
    bool mOnPay = false;
};

#endif // BEHPARDAKHTPOS_H
