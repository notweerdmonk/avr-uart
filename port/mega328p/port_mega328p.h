/*
 * avr-uart - UART module for AVR microcontrollers
 * Copyright (C) 2026 notweerdmonk
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
*/

#ifndef _AVR_UART_PORT_ATMEGA328P_H
#define _AVR_UART_PORT_ATMEGA328P_H

#ifdef __AVR_ATmega328P__

#define port_udre_vect \
  USART_UDRE_vect

#define port_rxc_vect \
  USART_RX_vect

#define PORT_UDR \
  UDR0

#define port_calc_uart_baudreg(baud_rate) \
  (uint16_t)((F_CPU / (16L * baud_rate)) - 1)

#define port_uart_set_baud_rate(baud_rate) \
  UBRR0 = port_calc_uart_baudreg(baud_rate)

#define port_uart_set_char_size2(n) \
  if (n > 5) { \
    if (n == 9) { \
      UCSR0C |= (3<<UCSZ00); \
    } else { \
      UCSR0C |= ((n-5)<<UCSZ00); \
    } \
  }

#define port_uart_set_char_size(n) \
  if (n == 6) { \
    UCSR0C |= (1 << UCSZ00); \
  } else if (n == 7) { \
    UCSR0C |= (1 << UCSZ01); \
  } else if (n == 8) { \
    UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01); \
  }

#define port_uart_set_stop_bits(n) \
  if (n == 2) { \
    UCSR0C |= (1<<USBS0); \
  }

#define port_uart_set_parity(parity) \
  if (parity == PORT_UART_PARITY_EVEN) { \
    UCSR0C |= (1<<UPM01); \
  } else if (parity == PORT_UART_PARITY_ODD) { \
    UCSR0C |= (1<<UPM00) | (1<<UPM01); \
  }

#define port_uart_setup() \
  UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0)

#define port_enable_udre_interrupt() \
  UCSR0B |= (1<<UDRIE0)

#define port_disable_udre_interrupt() \
  UCSR0B &= ~(1<<UDRIE0)

#define port_enable_rxc_interrupt() \
  UCSR0B |= (1<<RXCIE0)

#define port_disable_rxc_interrupt() \
  UCSR0B &= ~(1<<RXCIE0)

#endif /* __AVR_ATmega328P__ */

#endif /* _AVR_UART_PORT_ATMEGA328P_H */
