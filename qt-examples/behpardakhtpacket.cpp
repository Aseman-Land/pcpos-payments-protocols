#include "behpardakhtpacket.h"

#include <string>

#define POP_VALUE(NAME, LIST) \
    if (LIST.size() == 0) return false; \
    int NAME = LIST.front(); \
    LIST.pop_front()

#define EXTRACT_NUMBER(NAME, LIST, LIMIT) \
    int NAME; \
    if (!extractNumber(LIST, LIMIT, NAME)) \
        return false

#define SIZE_LIMIT 0x04

BehpardakhtPacket::BehpardakhtPacket()
{

}

bool BehpardakhtPacket::fromData(BehpardakhtPacket &pkt, const PacketResult &data)
{
    auto bytes = toArray(data).toStdString();
    bytes = bytes.substr(4, bytes.size()-6);

    pkt.m_responce_code = 1;
    pkt.m_price = 0;

    for (uint i=0; i<bytes.size()-1;)
    {
        auto code = bytes[i];
        auto size = bytes[i+1];

        if (code == (char)0xc0) // status
        {
            pkt.m_responce_code = std::atoi(bytes.substr(i+2, size).c_str());
        }
        if (code == (char)0xb0) // price
        {
            std::string price_str;
            for (const auto ch: bytes.substr(i+2, size))
                price_str += QByteArray(ch + QByteArray()).toHex().toStdString();

            pkt.m_price = std::atoi(price_str.c_str());
        }

        i += 2 + size;
    }

    return pkt.m_responce_code == 0 && pkt.m_price;
}

BehpardakhtPacket::PacketResult BehpardakhtPacket::data()
{
    return processRequest();
}

BehpardakhtPacket::PacketResult BehpardakhtPacket::processRequest()
{
    BehpardakhtPacket::PacketResult body;
    appendBit(body, 0xb0);
    appendBCD(body, m_price);

    if (m_order_id)
    {
        appendBit(body, 0xb1);
        appendBCD(body, m_order_id);
    }

    appendBit(body, 0xa2);
    appendBCD(body, 1);

    appendBit(body, 0xa4);
    append(body, 5263, -1);

    BehpardakhtPacket::PacketResult res;
    append(res, (int)body.size(), SIZE_LIMIT);

    for (const auto m: body)
        res.push_back(m);

    for (const auto b: calcCrc(body))
        res.push_back(b);

    return res;
}

const std::string &BehpardakhtPacket::card_number() const
{
    return m_card_number;
}

int BehpardakhtPacket::responce_code() const
{
    return m_responce_code;
}

int BehpardakhtPacket::price() const
{
    return m_price;
}

void BehpardakhtPacket::setPrice(int newPrice)
{
    m_price = newPrice;
}

int BehpardakhtPacket::order_id() const
{
    return m_order_id;
}

void BehpardakhtPacket::setOrder_id(int newOrder_id)
{
    m_order_id = newOrder_id;
}

void BehpardakhtPacket::append(BehpardakhtPacket::PacketResult &msg, int v, int length_limit, bool right_pad)
{
    const auto value = std::to_string(v);
    const auto value_size = (int)value.size();

    if (length_limit == -1)
    {
        appendBit(msg, value_size);
        length_limit = value_size;
    }

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

void BehpardakhtPacket::appendBCD(PacketResult &msg, int value)
{
    auto str = std::to_string(value);
    if (str.size() % 2 == 1)
        str = '0' + str;

    appendBit(msg, str.size()/2);
    for (const auto c: QByteArray::fromHex(str.c_str()))
        appendBit(msg, c);
}

void BehpardakhtPacket::appendBit(PacketResult &msg, int bit)
{
    msg.push_back(bit);
}

void BehpardakhtPacket::fill(PacketResult &msg, int c, int limit)
{
    for (int i=0; i<limit; i++)
        msg.push_back(c);
}

BehpardakhtPacket::PacketResult BehpardakhtPacket::calcCrc(const PacketResult &buf)
{
#define CRC16 0x8005
    QByteArray bytes = toArray(buf);
    const uint8_t *data = reinterpret_cast<uint8_t *>(bytes.data());
    uint16_t size = bytes.size();

    uint16_t out = 0;
    int bits_read = 0, bit_flag;

    /* Sanity check: */
    if(data == NULL)
        return {};

    while(size > 0)
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;
        if(bits_read > 7)
        {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if(bit_flag)
            out ^= CRC16;

    }

    // item b) "push out" the last 16 bits
    int i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if(bit_flag)
            out ^= CRC16;
    }

    // item c) reverse the bits
    uint16_t crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return {(int)(crc % 256), (int)(crc / 256)};
}

bool BehpardakhtPacket::extractNumber(std::list<int> &list, int limit, int &r)
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

bool BehpardakhtPacket::extractNull(std::list<int> &list, int limit)
{
    while (limit && list.size())
    {
        list.pop_front();
        limit--;
    }
    return true;
}

QByteArray BehpardakhtPacket::toArray(const BehpardakhtPacket::PacketResult &list)
{
    QByteArray res;
    for (const auto l: list)
        res += (uchar)l;
    return res;
}

BehpardakhtPacket::PacketResult BehpardakhtPacket::toVector(const QByteArray &bytes)
{
    BehpardakhtPacket::PacketResult res;
    for (const auto b: bytes)
        res.push_back(b);
    return res;
}
