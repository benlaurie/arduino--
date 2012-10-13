// -*- mode: c++; indent-tabs-mode: nil; -*-
/*
  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 27 December 2011 by Ben Laurie (RingBuffer)
  Modified 5 February 2012 by Lars Immisch (template remix)  
*/

#ifndef SERIAL_H
#define SERIAL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <avr/interrupt.h>
#include <compat/deprecated.h>  // for sbi, cbi

template <byte rx_buffer_size> class RingBuffer
    {
 public:
    void store(byte c)
        {
        byte i = (_head + 1) % rx_buffer_size;

        // if we should be storing the received character into the location
        // just before the tail (meaning that the head would advance to the
        // current location of the tail), we're about to overflow the buffer
        // and so we don't write the character or advance the head.
        if (i != _tail)
            {
            _buffer[_head] = c;
            _head = i;
            }
        }
    int available(void) const
        { return (rx_buffer_size + _head - _tail) % rx_buffer_size; }
    int peek(void) const
        {
        if (_head == _tail)
            return -1;
        else
            return _buffer[_tail];
        }
    int read(void)
        {
        // if the head isn't ahead of the tail, we don't have any characters
        if (_head == _tail)
            return -1;
        else
            {
            byte c = _buffer[_tail];
            _tail = (_tail + 1) % rx_buffer_size;
            return c;
            }
        }
    void flush(void)
        {
        // don't reverse this or there may be problems if the RX interrupt
        // occurs after reading the value of rx_buffer_head but before writing
        // the value to rx_buffer_tail; the previous value of rx_buffer_head
        // may be written to rx_buffer_tail, making it appear as if the buffer
        // were full, not empty.
        _head = _tail;
        }

 private:
    unsigned char _buffer[rx_buffer_size];
    volatile byte _head;
    volatile byte _tail;
    };

template <byte ubrrh, byte ubrrl, byte ucsra, byte ucsrb, byte udr,
  byte rxen, byte txen, byte rxcie, byte udre, byte u2x, byte size>
    class HardwareSerial : public RingBuffer<size>
    {
 public:

    static const byte ucsrbmask = (1 << rxen) | (1 << txen) | (1 << rxcie);

    void begin(long baud)
        {
        uint16_t baud_setting;
        bool use_u2x = true;

#if F_CPU == 16000000UL
        // hardcoded exception for compatibility with the bootloader shipped
        // with the Duemilanove and previous boards and the firmware on the 8U2
        // on the Uno and Mega 2560.
        if (baud == 57600)
            use_u2x = false;
#endif

try_again:
       if (use_u2x)
            {
            _SFR_IO8(ucsra) = 1 << u2x;
            baud_setting = (F_CPU / 4 / baud - 1) / 2;
            }
        else
            {
            _SFR_IO8(ucsra) = 0;
            baud_setting = (F_CPU / 8 / baud - 1) / 2;
            }

       if ((baud_setting > 4095) && use_u2x)
           {
           use_u2x = false;
           goto try_again;
           }

        // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
        _SFR_IO8(ubrrh) = baud_setting >> 8;
        _SFR_IO8(ubrrl) = baud_setting;

        _SFR_IO8(ucsrb) |= ucsrbmask;
        _SFR_IO8(ucsrb) &= ~udre;        
        }

    void end()
        { 
        _SFR_IO8(ucsrb) &= ~(ucsrbmask | (1 << udre));
        RingBuffer<size>::flush();
        }

    void write(uint8_t c)
        {
        while (!(_SFR_IO8(ucsra) & (1 << udre)))
            ;
        _SFR_IO8(udr) = c;
        }
    
    void writeHex(byte b) 
        { HexWriter<HardwareSerial>::write(this, b); }
    void writeHex(uint16_t i) 
        { HexWriter<HardwareSerial>::write(this, i); }
    void writeHex(const byte *b, byte n)
        { HexWriter<HardwareSerial>::write(this, b, n); }
    void write(const char *str)
        { StringWriter<HardwareSerial>::write(this, str); }
    void write_P(const char *str)
        { StringWriter<HardwareSerial>::write_P(this, str); }
    void writeDecimal(uint32_t d, byte digits = 1)
	{ DecimalWriter<HardwareSerial>::write(this, d, digits); }
};

// typedefs for serial classes

#if defined(__AVR_ATmega8__)
typedef HardwareSerial<NUBRRH, NUBRRL, NUCSRA, NUCSRB, NUDR, 
  RXEN, TXEN, RXCIE, UDRE, U2X, 128> _Serial;
#else
typedef HardwareSerial<NUBRR0H, NUBRR0L, NUCSR0A, NUCSR0B, NUDR0, 
  RXEN0, TXEN0, RXCIE0, UDRE0, U2X0, 128> _Serial;
#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
typedef HardwareSerial<NUBRR1H, NUBRR1L, NUCSR1A, NUCSR1B, NUDR1, 
  RXEN1, TXEN1, NRXCIE1, UDRE1, U2X1, 128> _Serial;
typedef HardwareSerial<NUBRR2H, NUBRR2L, NUCSR2A, NUCSR2B, NUDR2, 
  RXEN2, TXEN2, NRXCIE2, UDRE2, U2X2, 128> _Serial2;
typedef HardwareSerial<NUBRR3H, NUBRR3L, NUCSR3A, NUCSR3B, NUDR3, 
  RXEN3, TXEN3, NRXCIE3, UDRE3, U2X3, 128> _Serial3;
#endif

// Preinstantiate Objects //////////////////////////////////////////////////////

_Serial Serial;

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

SIGNAL(SIG_USART0_RECV)
    {
    unsigned char c = UDR0;
    Serial.store(c);
    }

SIGNAL(SIG_USART1_RECV)
    {
    unsigned char c = UDR1;
    Serial1.store(c);
    }

SIGNAL(SIG_USART2_RECV)
    {
    unsigned char c = UDR2;
    Serial2.store(c);
    }

SIGNAL(SIG_USART3_RECV)
    {
    unsigned char c = UDR3;
    Serial3.store(c);
    }

#else

#if defined(__AVR_ATmega8__)
SIGNAL(SIG_UART_RECV)
#else
SIGNAL(USART_RX_vect)
#endif
    {
#if defined(__AVR_ATmega8__)
    unsigned char c = UDR;
#else
    unsigned char c = UDR0;
#endif
    Serial.store(c);
    }

#endif

// Observer for star slaves (star.h)
class SerialSlaveObserver
    {
public:
    static void cantSend() { Serial.write('.'); }
    static void gotPacket(byte id, byte type, byte length, const byte *data)
	{
	Serial.write("Got packet, id: ");
	Serial.writeDecimal(id);
	Serial.write(" type: ");
	Serial.writeDecimal(type);
	Serial.write(" data: ");
	Serial.writeHex(data, length);
	Serial.write("\r\n");
	}
    static void protocolError(byte id, byte type, byte length, const byte *data)
	{
	Serial.write("PROTOCOL ERROR\r\n");
	}
    static void sentPacket(byte id, byte type, byte length, const byte *data)
	{
	Serial.write("Sent packet, id: ");
	Serial.writeDecimal(id);
	Serial.write(" type: ");
	Serial.writeDecimal(type);
	Serial.write(" data: ");
	Serial.writeHex(data, length);
	Serial.write("\r\n");
	}
    static void canSend() {}
    };

#endif  // ndef SERIAL_H
