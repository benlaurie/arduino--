#include "arduino++.h"

// Note: with avr-gcc 4.5.1 this DOES NOT WORK when compiled with -O3
// and not -Os

#include "serial.h"  // FIXME: should not need to depend on this.

template <class Pin> class Button
    {
public:
    void SetID(const byte bits[64])
	{
	for (byte n = 0; n < 8; ++n)
	    for (byte b = 0; b < 8; ++b)
		id_[n] = (id_[n] << 1) | bits[(7-n)*8 + 7 - b];
	}
    bool operator==(const Button &other) const
	{
	for (byte n = 0; n < 8; ++n)
	    if (id_[n] != other.id_[n])
		return false;
	return true;
	}

    void Reset() const;
    void OutByte(byte b) const;
    byte InByte() const;
    void Select() const;
    void GetTemperature();
    void Dump(_Serial *serial) const;

    enum Command
	{
	SEARCH_ROM = 0xf0,
	READ_ROM = 0x33,
	MATCH_ROM = 0x55,
	SKIP_ROM = 0xcc,

	// DS18B20
	CONVERT_T = 0x44,
	WRITE_SCRATCHPAD = 0x4e,
	READ_SCRATCHPAD = 0xbe,
	RECALL_EE = 0xb8,
	READ_POWER_SUPPLY = 0xb4,
	};
	
private:
    byte id_[8];
    uint16_t temperature_;  // as read from the device.
    };

template <class Pin> class Buttons
    {
public:
    Buttons() : num_(0) {}
    static void Init()
	{
	Pin::clear();
	Pin::modeInput();
	}
    static void Reset();
    static void OutBit(bool bit);
    static void OutByte(byte b);
    static byte InBit();
    static byte InByte();
    bool Scan();
    void Add(const byte bits[64])
	{
	if (num_ >= MAX_BUTTONS)
	    return;
	Button<Pin> b;
	b.SetID(bits);
	for (byte n = 0; n < num_; ++n)
	    if (b == buttons_[n])
		return;
	buttons_[num_++].SetID(bits);
	}
    void GetTemperatures();
    // return true if any device is using parasite power
    static bool GetParasites()
	{
	Reset();
	OutByte(Button<Pin>::SKIP_ROM);
	OutByte(Button<Pin>::READ_POWER_SUPPLY);
	byte b = InBit();
	return b == 0;
	}
    Button<Pin> &operator[](unsigned n) { return buttons_[n]; }
    void Dump(_Serial *serial) const
	{
	for (byte b = 0; b < num_; ++b)
	    buttons_[b].Dump(serial);
	}
private:
    static const int MAX_BUTTONS = 10;
    Button<Pin> buttons_[MAX_BUTTONS];
    byte num_;
    };

template <class Pin> void Button<Pin>::Reset() const { Buttons<Pin>::Reset(); }
template <class Pin> void Button<Pin>::OutByte(byte b) const
    { Buttons<Pin>::OutByte(b); }
template <class Pin> byte Button<Pin>::InByte() const
    { return Buttons<Pin>::InByte(); }

template <class Pin> void Buttons<Pin>::Reset()
    {
    Pin::clear();
    Pin::modeOutput();
    _delay_us(480);
    Pin::modeInput();
    _delay_us(480);
    }

template <class Pin> void Buttons<Pin>::OutBit(bool bit)
    {
    if (bit)
	{
	Pin::clear();
	Pin::modeOutput();
	_delay_us(1);
	Pin::modeInput();  // width of low is 1.25 us
	_delay_us(59);
	}
    else
	{
	Pin::clear();
	Pin::modeOutput();
	_delay_us(60);
	Pin::modeInput();  // width of low is 60.4375 us
	}
    // there is supposed to be a 1 us rest between reads and/or
    // writes. Since function call overhead is 1 us, just assume we're
    // going to get that.

    // If we measure from the start of this bit (in a byte) to the
    // start of the next, we get:
    // 0: 61.75 us
    // 1: 61.5625 us
    // To do any better, we need sub-us delays...
    }

template <class Pin> void Buttons<Pin>::OutByte(byte d)
    {
    byte n;

    for(n = 8; n != 0; n--)
	{
	OutBit((d & 0x01) == 1);  // test least sig bit
	d = d >> 1; // now the next bit is in the least sig bit position.
	}
    }

// Note that reading is just like writing a 1. Except you read as well :-)
template <class Pin> byte Buttons<Pin>::InBit()
    {
    ::Pin::C4::modeOutput();
    ::Pin::C4::set();  // .125 us
    ::Pin::C4::clear();  // .125 us
    ::Pin::C4::set();  // .125 us

    Pin::clear();
    Pin::modeOutput();
    // This should be 1 us ideally: the longer it is, the less time
    // the bus has to rise to show us a 1. But since 1us is the
    // minimum, 2 for safety.
    _delay_us(1);
    ::Pin::C4::clear();

    Pin::modeInput();  // this ends a down pulse of width 1.375 us with
    // debugging, should be 1.25 us without debugging.

    // Really we should delay this read until as near to 15 us after
    // we pull the bus down as we dare so we don't get caught out by a
    // slow rise. FIXME when I have a logic analyser.
    _delay_us(13);

    ::Pin::C4::set();  // this occurs at 14.25 us after the bus goes low,
		// with debugging. Without it should be 14 us exactly
		// from dropping the bus to reading
    byte b = Pin::read();

    _delay_us(46);  // and this brings the total to 60.375 us with
		    // debugging, should be 60 us exactly without(!).

    ::Pin::C4::clear();

    return b;
    }

template <class Pin> byte Buttons<Pin>::InByte()
    {
    byte d = 0;

    for (byte n = 0; n < 8; n++)
	{
	byte b = InBit();
        d = (d >> 1) | (b << 7);
	}
    return d;
    }

template <class Pin> void Button<Pin>::Select() const
    {
    Reset();
    OutByte(MATCH_ROM);
    for (byte n = 7; n >= 0; --n)
	OutByte(id_[n]);
    }

template <class Pin> void Buttons<Pin>::GetTemperatures()
    {  
    Reset();
    // Select all
    OutByte(Button<Pin>::SKIP_ROM);
    // perform temperature conversion, strong pullup for one sec (750
    // ms, surely?)
    OutByte(Button<Pin>::CONVERT_T);
    // since we provide power rather than strong pullup, we could do
    // something else for 750ms
    // delay(750);
    // Since we're not using parasitic power, we can instead do...
    while(!InBit())
	;

    for (byte n = 0; n < num_; ++n)
	buttons_[n].GetTemperature();
    }

template <class Pin> void Button<Pin>::GetTemperature()
    {
    Select();
    OutByte(READ_SCRATCHPAD);

    byte LowByte = InByte();
    byte HighByte = InByte();
    temperature_ = ((uint16_t)HighByte << 8) + LowByte;
    }

/* Calculate the CRC X^8+X^5+X^4+1. I think the reason we actually use
   8, 4 and 3 is coz we are doing the last 3 terms, and they are reversed,
   bitwise, i.e. we are doing (8-0), (8-4) and (8-5). What happened to the 8,
   I dunno, except, since we immediately shift right, it disappears???
   This could be complete cobblers, of course.
   In fact, coz we do it in a 1 byte field, and do the right shift first, it
   looks even stranger, coz we now use 7, 3, 2, which is a long way from the
   original.
*/
class iBLabCRC8
    {
    byte m_ucCRC;
public:
    iBLabCRC8()
	{ m_ucCRC=0; }
    void Bit(int nBit)
	{
	//assert(!(nBit&~1));
	nBit &= 1;
	nBit ^= m_ucCRC&1;
	m_ucCRC >>= 1;
	if(nBit)
	    m_ucCRC ^= 0x8c;
//	m_ucCRC^=(nBit << 7)|(nBit << 3)|(nBit << 2);
	}
    void Bits(unsigned un,unsigned nBits)
	{
	for(unsigned n=0 ; n < nBits ; ++n)
	    {
	    Bit(un&1);
	    un>>=1;
	    }
	}
    void Byte(unsigned un)
	{ Bits(un,8); }
    bool OK() const
	{ return !m_ucCRC; }
    byte Value() const
	{ return m_ucCRC; }
    };

template <class Pin> bool Buttons<Pin>::Scan()
    {
    static byte ucBits[64];
    int nLastConflict;
    int nConflict=-1;

    for( ; ; )
	{
	iBLabCRC8 crc;

	Reset();

	nLastConflict=nConflict;
	nConflict=0;
	OutByte(Button<Pin>::SEARCH_ROM);
	for(int n=0 ; n < 64 ; ++n)
	    {
	    byte b1 = InBit();
	    byte b2 = InBit();

	    if(b1 == 1 && b2 == 1)
		{
		return false;
		}
	    else if(b1 == 0 && b2 == 1)
		{
		ucBits[n]=0;
		}
	    else if(b1 == 1 && b2 == 0)
		{
		ucBits[n]=1;
		}
	    else
		{
		if(n == nLastConflict)
		    {
		    ucBits[n]=1;
		    }
		else if(n > nLastConflict)
		    {
		    ucBits[n]=0;
		    nConflict=n;
		    }
		else if(ucBits[n] == 0)
		    {
		    nConflict=n;
		    }
		else
		    {
		    }
		}
	    crc.Bit(ucBits[n]);
	    OutBit(ucBits[n]);
	    }

	if(crc.OK())
	    {
	    Add(ucBits);
	    }
		
	if(!crc.OK())
	    return false;

	if(!nConflict)
	    break;
	}

    return true;
    }
