#include "enc28j60.h"

ENC28J60<Pin::B2> ether;

int main(int argc, char **argv)
    {
    ether.PhyWrite(0, 0);
    ether.Init(0);
    ether.PacketSend(0, 0);
    ether.PacketReceive(0, 0);
    }
