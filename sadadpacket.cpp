#include "sadadpacket.h"

#include <string>
#include <QString>

#define POP_VALUE(NAME, LIST) \
    if (LIST.size() == 0) return false; \
    int NAME = LIST.front(); \
    LIST.pop_front()

#define EXTRACT_NUMBER(NAME, LIST, LIMIT) \
    int NAME; \
    if (!extractNumber(LIST, LIMIT, NAME)) \
        return false

#define EXTRACT_UNICODE(NAME, LIST, LIMIT) \
    std::string NAME; \
    if (!extractUnicode(LIST, LIMIT, NAME)) \
        return false

#define SIZE_LIMIT 0x03
#define VERSION_LIMIT 0x03
#define TYPE_LIMIT 0x01
#define AMOUNT_LIMIT 0x0d
#define INVOICE_NUMBER_LIMIT 0x0a
#define FUNCTION_CODE_LIMIT 0x04
#define MESSAGE_TYPE_LIMIT 0x04
#define PROCESSING_CODE_LIMIT 0x06
#define BITMAP_LIMIT 0x01
#define OPTIONAL_LIMIT 0x01

SadadPacket::SadadPacket(PacketType type, Version version)
    : m_type(type)
    , m_version(version)
{

}

bool SadadPacket::fromData(SadadPacket &pkt, const PacketResult &data)
{
    std::list<int> list{data.begin(), data.end()};

    POP_VALUE(stx, list);
    if (stx != (int)PacketType::STX)
        return false;

    EXTRACT_NUMBER(msg_length, list, SIZE_LIMIT);
    EXTRACT_NUMBER(version, list, VERSION_LIMIT);

    POP_VALUE(type, list);

    pkt.m_type = (PacketType)type;
    switch (type)
    {
    case (int)PacketType::REQUEST:
        break;
    case (int)PacketType::RESPONSE:
    {
        EXTRACT_UNICODE(code, list, 2);
        pkt.m_responce_code = std::atoi(code.c_str());

        extractNull(list, 4);

        EXTRACT_UNICODE(price, list, 12);
        pkt.m_price = std::atoi(price.c_str());

        extractNull(list, 2);

        EXTRACT_UNICODE(card, list, 16);
        pkt.m_card_number = card;
    }
        break;
    case (int)PacketType::ACK:
    case (int)PacketType::NACK:
    {
        POP_VALUE(etx, list);
        if (etx != (int)PacketType::ETX)
            return false;
    }
        break;
    }

    return true;
}

SadadPacket::PacketResult SadadPacket::data()
{
    switch ((int)m_type)
    {
    case (int)PacketType::REQUEST:
        return processRequest();
        break;
    case (int)PacketType::ACK:
    case (int)PacketType::NACK:
    case (int)PacketType::INM:
        return processAck();
        break;
    }

    return {};
}

SadadPacket::PacketResult SadadPacket::processRequest()
{
    SadadPacket::PacketResult msg;
    append(msg, (char)m_version, VERSION_LIMIT);
    appendBit(msg, (int)m_type);
    append(msg, m_price, AMOUNT_LIMIT, true);
    fill(msg, 0x20, INVOICE_NUMBER_LIMIT);
    fill(msg, 0x20, FUNCTION_CODE_LIMIT);
    append(msg, m_message_type, MESSAGE_TYPE_LIMIT);
    append(msg, m_processing_code, PROCESSING_CODE_LIMIT);
//    appendBit(msg, m_bitmap);

    if (m_sale_id)
    {
        appendBit(msg, 0x82);
        const auto sale_id = std::to_string(m_sale_id);
        append(msg, sale_id.size(), 3);
        append(msg, m_sale_id, sale_id.size());
    }
    else
    {
        appendBit(msg, 0x90);
        const auto order_id = std::to_string(m_order_id);
        append(msg, order_id.size(), 3);
        append(msg, m_order_id, order_id.size());

    }

    for (const auto n: {0x30, 0x31, 0x32, 0x35, 0x2e, 0x33, 0x2e, 0x31, 0x38, 0x35, 0x2e, 0x31, 0x33, 0x33, 0x36})
        msg.push_back(n);

    SadadPacket::PacketResult buf;
    append(buf, (int)msg.size() + 2, SIZE_LIMIT);
    for (const auto m: msg)
        buf.push_back(m);

    SadadPacket::PacketResult res;
    res.push_back( (int)PacketType::STX );

    for (const auto m: buf)
        res.push_back(m);

    res.push_back( (int)PacketType::ETX );
    res.push_back( calcLrc(buf) );

    return res;
}

SadadPacket::PacketResult SadadPacket::processAck()
{
    SadadPacket::PacketResult msg;
    append(msg, (char)m_version, VERSION_LIMIT);
    appendBit(msg, (int)m_type);

    SadadPacket::PacketResult buf;
    append(buf, (int)msg.size() + 2, SIZE_LIMIT);
    for (const auto m: msg)
        buf.push_back(m);

    SadadPacket::PacketResult res;
    res.push_back( (int)PacketType::STX );

    for (const auto m: buf)
        res.push_back(m);

    res.push_back( (int)PacketType::ETX );
    res.push_back( calcLrc(buf) );

    return res;
}

const std::string &SadadPacket::card_number() const
{
    return m_card_number;
}

int SadadPacket::responce_code() const
{
    return m_responce_code;
}

int SadadPacket::sale_id() const
{
    return m_sale_id;
}

void SadadPacket::setSale_id(int newSale_id)
{
    m_sale_id = newSale_id;
}

int SadadPacket::price() const
{
    return m_price;
}

void SadadPacket::setPrice(int newPrice)
{
    m_price = newPrice;
}

int SadadPacket::order_id() const
{
    return m_order_id;
}

void SadadPacket::setOrder_id(int newOrder_id)
{
    m_order_id = newOrder_id;
}

void SadadPacket::append(SadadPacket::PacketResult &msg, int v, int length_limit, bool right_pad)
{
    const auto value = std::to_string(v);
    const auto value_size = (int)value.size();

    if (right_pad)
    {
        for (int i=0; i<length_limit; i++)
        {
            if (i < value_size)
                msg.push_back( value.at(i) );
            else
                msg.push_back(0x00);
        }
    }
    else
    {
        for (int i = value_size - length_limit; i<value_size; i++)
        {
            if (i >= 0)
                msg.push_back( value.at(i) );
            else
                msg.push_back(0x30);
        }
    }
}

void SadadPacket::appendBit(PacketResult &msg, int bit)
{
    msg.push_back(bit);
}

void SadadPacket::fill(PacketResult &msg, int c, int limit)
{
    for (int i=0; i<limit; i++)
        msg.push_back(c);
}

int SadadPacket::bitmap() const
{
    return m_bitmap;
}

void SadadPacket::setBitmap(int newBitmap)
{
    m_bitmap = newBitmap;
}

int SadadPacket::calcLrc(const PacketResult &buf)
{
    int dwSize = buf.size();
    int lrc = 0;
    for (int i = 0; i < dwSize; i++)
    {
        lrc = (int)(lrc ^ (int)buf.at(i));
    }
    lrc = (int)(lrc ^ 3);
    return (int)(lrc & 0xff);
}

bool SadadPacket::extractNumber(std::list<int> &list, int limit, int &r)
{
    std::string res;
    for (int i=0; i<limit; i++)
    {
        POP_VALUE(v, list);
        res += (char)(v);
    }
    r = std::atoi(res.c_str());
    return true;
}

bool SadadPacket::extractUnicode(std::list<int> &list, int limit, std::string &res)
{
    if (limit*2 > (int)list.size())
        return false;

    QString s;
    for (int i=0; i<limit; i++)
    {
        POP_VALUE(v1, list);
        POP_VALUE(v2, list);

        if (!v1 && !v2)
            continue;

        s += QChar(v1, v2);
    }

    res = s.toStdString();
    return true;
}

bool SadadPacket::extractNull(std::list<int> &list, int limit)
{
    while (limit && list.size())
    {
        list.pop_front();
        limit--;
    }
    return true;
}

SadadPacket::Version SadadPacket::version() const
{
    return m_version;
}

SadadPacket::PacketType SadadPacket::type() const
{
    return m_type;
}
