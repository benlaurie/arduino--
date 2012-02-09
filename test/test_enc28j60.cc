#include "enc28j60.h"

ENC28J60<Pin::B2> ether;
template<class Pin> byte ENC28J60<Pin>::Enc28j60Bank;
template<class Pin> uint16_t ENC28J60<Pin>::NextPacketPtr;

int main(int argc, char **argv)
    {
    ether.PhyWrite(0, 0);
    ether.Init(0);
    ether.PacketSend(0, 0);
    ether.PacketReceive(0, 0);
    }
