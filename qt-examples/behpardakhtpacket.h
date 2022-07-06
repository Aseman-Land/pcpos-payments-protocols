#ifndef BEHPARDAKHTPACKET_H
#define BEHPARDAKHTPACKET_H

#include <vector>
#include <string>
#include <list>
#include <QByteArray>

class BehpardakhtPacket
{
public:
    typedef std::vector<int> PacketResult;

    BehpardakhtPacket();

    static bool fromData(BehpardakhtPacket &pkt, const PacketResult &data);

    PacketResult data();

    int price() const;
    void setPrice(int newPrice);

    int order_id() const;
    void setOrder_id(int newOrder_id);

    int responce_code() const;

    const std::string &card_number() const;

protected:
    static void append(BehpardakhtPacket::PacketResult &msg, int value, int length_limit, bool right_pad = false);
    static void appendBCD(BehpardakhtPacket::PacketResult &msg, int value);
    static void appendBit(BehpardakhtPacket::PacketResult &msg, int bit);
    static void fill(BehpardakhtPacket::PacketResult &msg, int c, int limit);
    static PacketResult calcCrc(const PacketResult &Buffer);

    static bool extractNumber(std::list<int> &list, int limit, int &res);
    static bool extractNull(std::list<int> &list, int limit);

    PacketResult processRequest();

    static QByteArray toArray(const PacketResult &list);
    static PacketResult toVector(const QByteArray &bytes);

private:
    int m_price = 0;
    int m_order_id = 0;
    int m_responce_code = -1;
    std::string m_card_number;
};

#endif // BEHPARDAKHTPACKET_H
