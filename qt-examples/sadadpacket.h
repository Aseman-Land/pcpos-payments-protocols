#ifndef SADADPACKET_H
#define SADADPACKET_H

#include <vector>
#include <string>
#include <list>

class SadadPacket
{
public:
    typedef std::vector<int> PacketResult;

    enum class PacketType {
        INM = 0xF1,
        EOT = 0x04,
        ACK = 0x06,
        STX = 0x02,
        ETX = 0x03,
        NACK = 0x15,
        REQUEST = 0x19,
        RESPONSE = 0x1A,
    };

    enum class Version {
        KHADAMAT = 300,
        SADAD = 000,
        SADADENCRYPTION = 001,
        SADAD2 = 002,
    };

    enum class ProcessingCode {
        PROCODE_CHARGE = 150000,
        PROCODE_BALANCE = 310000,
        PROCODE_SALE_INQUIRY = 330000,
        PROCODE_SALE = 000000,
        PROCODE_BILL_PAYMENT = 170000,
        PROCODE_SALE_FIXED_DUTY = 220000,
        PROCODE_SALE_IDENTIFIED_INQUIRY = 240000,
        PROCODE_SALE_GOVERNMENT_IDENTIFIED_INQUIRY = 270000,
        PROCODE_SALE_GOVERNMENT_IDENTIFIED_INQUIRY_MULTI_ID = 280000,
    };

    SadadPacket(PacketType type = PacketType::REQUEST, Version version = Version::SADAD);

    static bool fromData(SadadPacket &pkt, const PacketResult &data);

    PacketResult data();

    int price() const;
    void setPrice(int newPrice);

    int order_id() const;
    void setOrder_id(int newOrder_id);

    int bitmap() const;
    void setBitmap(int newBitmap);

    PacketType type() const;
    Version version() const;

    int sale_id() const;
    void setSale_id(int newSale_id);

    int responce_code() const;

    const std::string &card_number() const;

protected:
    static void append(PacketResult &msg, int value, int length_limit, bool right_pad = false);
    static void appendBit(PacketResult &msg, int bit);
    static void fill(PacketResult &msg, int c, int limit);
    static int calcLrc(const PacketResult &Buffer);

    static bool extractNumber(std::list<int> &list, int limit, int &res);
    static bool extractUnicode(std::list<int> &list, int limit, std::string &res);
    static bool extractNull(std::list<int> &list, int limit);

    PacketResult processRequest();
    PacketResult processAck();

private:
    PacketType m_type;
    Version m_version;
    int m_price = 0;
    int m_message_type = 200;
    int m_processing_code = (int)ProcessingCode::PROCODE_SALE;
    int m_bitmap = 0x19;
    int m_order_id = 0;
    int m_sale_id = 0;
    int m_responce_code = -1;
    std::string m_card_number;
};

#endif // SADADPACKET_H
