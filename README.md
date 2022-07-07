# pcpos-payments-protocols
PC Pos Protocols using C++

## How it Works

It's just part of the protocole. You must write your own connection part (using tcp or serial port).

When your app connected to the pos, You can generate binary strings using below codes:

```c++
#include "sadadpacket.h"

SadadPacket pkt;
pkt.setPrice(100000); // Rials
pkt.setOrder_id(1234);
std::vector<int> data = pkt.data(); // It's a vector of int to send to the device
```

And to decode Strings readed from device:

```c++
#include "sadadpacket.h"

std::vector<int> data;
// Fill data using string readed from pc-pos

SadadPacket pkt;
SadadPacket::fromData(pkt, data);

std::cout << pkt.card_number() << "\n"
          << pkt.responce_code() << "\n";
```

