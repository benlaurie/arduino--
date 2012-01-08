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

  Modified 27 December 2011 by Ben Laurie
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <avr/interrupt.h>
#include <compat/deprecated.h>  // for sbi, cbi

template <byte RX_BUFFER_SIZE> class RingBuffer
    {
 public:
    void store(byte c)
	{
	int i = (_head + 1) % RX_BUFFER_SIZE;

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
	{ return (RX_BUFFER_SIZE + _head - _tail) % RX_BUFFER_SIZE; }
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
	    _tail = (_tail + 1) % RX_BUFFER_SIZE;
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
    unsigned char _buffer[RX_BUFFER_SIZE];
    byte _head;
    byte _tail;
    };

class HardwareSerial : public RingBuffer<128>
    {
 private:
    volatile uint8_t *_ubrrh;
    volatile uint8_t *_ubrrl;
    volatile uint8_t *_ucsra;
    volatile uint8_t *_ucsrb;
    volatile uint8_t *_udr;
    byte _ucsrbmask;
    uint8_t _udre;
    uint8_t _u2x;
 public:
    HardwareSerial(volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
		   volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
		   volatile uint8_t *udr, uint8_t rxen, uint8_t txen,
		   uint8_t rxcie, uint8_t udre, uint8_t u2x)
	{
	_ubrrh = ubrrh;
	_ubrrl = ubrrl;
	_ucsra = ucsra;
	_ucsrb = ucsrb;
	_udr = udr;
	_ucsrbmask = (1 << rxen) | (1 << txen) | (1 << rxcie);
	_udre = udre;
	_u2x = u2x;
	}
    void begin(long baud)
	{
	uint16_t baud_setting;
	bool use_u2x;

	// U2X mode is needed for baud rates higher than (CPU Hz / 16)
	if (baud > F_CPU / 16)
	    {
	    use_u2x = true;
	    }
	else
	    {
	    // figure out if U2X mode would allow for a better
	    // connection calculate the percent difference between the
	    // baud-rate specified and the real baud rate for both U2X
	    // and non-U2X mode (0-255 error percent)
	    uint8_t nonu2x_baud_error
		= abs((int)(255 - ((F_CPU / (16 * (((F_CPU / 8 / baud - 1) / 2)
						   + 1)) * 255) / baud)));
	    uint8_t u2x_baud_error
		= abs((int)(255 - ((F_CPU / (8 * (((F_CPU / 4 / baud - 1) / 2)
						  + 1)) * 255) / baud)));
    
	    // prefer non-U2X mode because it handles clock skew better
	    use_u2x = (nonu2x_baud_error > u2x_baud_error);
	    }
  
	if (use_u2x)
	    {
	    *_ucsra = 1 << _u2x;
	    baud_setting = (F_CPU / 4 / baud - 1) / 2;
	    }
	else
	    {
	    *_ucsra = 0;
	    baud_setting = (F_CPU / 8 / baud - 1) / 2;
	    }

	// assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
	*_ubrrh = baud_setting >> 8;
	*_ubrrl = baud_setting;

	*_ucsrb |= _ucsrbmask;
	}
    void end()
	{ *_ucsrb &= ~_ucsrbmask; }
    void write(uint8_t c)
	{
	while (!((*_ucsra) & (1 << _udre)))
	    ;
	
	*_udr = c;
	}
    void writeHex(byte b) { HexWriter<HardwareSerial>::write(this, b); }
    void writeHex(uint16_t i) { HexWriter<HardwareSerial>::write(this, i); }
    void write(const char *str)
	{ StringWriter<HardwareSerial>::write(this, str); }
    void write_P(const char *str)
	{ StringWriter<HardwareSerial>::write_P(this, str); }
};

// Preinstantiate Objects //////////////////////////////////////////////////////

#if defined(__AVR_ATmega8__)
HardwareSerial Serial(&UBRRH, &UBRRL, &UCSRA, &UCSRB, &UDR, RXEN, TXEN, RXCIE, UDRE, U2X);
#else
HardwareSerial Serial(&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UDR0, RXEN0, TXEN0, RXCIE0, UDRE0, U2X0);
#endif

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
HardwareSerial Serial1(&UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UDR1, RXEN1, TXEN1, RXCIE1, UDRE1, U2X1);
HardwareSerial Serial2(&UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UDR2, RXEN2, TXEN2, RXCIE2, UDRE2, U2X2);
HardwareSerial Serial3(&UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UDR3, RXEN3, TXEN3, RXCIE3, UDRE3, U2X3);
#endif

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
