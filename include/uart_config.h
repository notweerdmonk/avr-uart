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

#ifndef _AVR_UART_UART_CONFIG_H_
#define _AVR_UART_UART_CONFIG_H_

#include <port.h>

#ifdef UART_RUNTIME_CONFIG

typedef struct uart_config {
  uint32_t baud_rate;
  uint8_t char_size : 4;
  uint8_t stop_bits : 2;
  uint8_t parity : 2;
} uart_config_t;

#endif /* UART_RUNTIME_CONFIG */

/*
 * NOTE:
 * Mind the size of the RAM when deciding Rx and Tx buffer sizes.
 */
enum { UART_TX_BUFFER_LEN_DEFAULT = 64 };
enum { UART_RX_BUFFER_LEN_DEFAULT = 64 };

enum { UART_MAX_SEQ_LEN_DEFAULT = 8 };
enum { UART_MATCH_MAX_DEFAULT = 8 };

enum { UART_BAUD_DEFAULT = 9600 };

enum { UART_CHAR_SIZE_DEFAULT = 8 };
enum { UART_STOP_BITS_DEFAULT = 1 };

#define UART_PARITY_NONE    PORT_UART_PARITY_DISABLED
#define UART_PARITY_EVEN    PORT_UART_PARITY_EVEN
#define UART_PARITY_ODD     PORT_UART_PARITY_ODD
#define UART_PARITY_MARK    PORT_UART_PARITY_DISABLED
#define UART_PARITY_SPACE   PORT_UART_PARITY_DISABLED
#define UART_PARITY_DEFAULT UART_PARITY_NONE

#ifndef UART_BAUD_RATE
#define UART_BAUD_RATE UART_BAUD_DEFAULT
#endif

#ifndef UART_CHAR_SIZE
#define UART_CHAR_SIZE UART_CHAR_SIZE_DEFAULT
#endif

#ifndef UART_STOP_BITS
#define UART_STOP_BITS UART_STOP_BITS_DEFAULT
#endif

#ifndef UART_TX_BUFFER_LEN
#define UART_TX_BUFFER_LEN UART_TX_BUFFER_LEN_DEFAULT
#endif

#ifndef UART_RX_BUFFER_LEN
#define UART_RX_BUFFER_LEN UART_RX_BUFFER_LEN_DEFAULT
#endif

#ifndef UART_PARITY
#define UART_PARITY UART_PARITY_NONE
#endif

#ifndef UART_MAX_SEQ_LEN
#define UART_MAX_SEQ_LEN UART_MAX_SEQ_LEN_DEFAULT
#endif

#ifndef UART_MATCH_MAX
#define UART_MATCH_MAX UART_MATCH_MAX_DEFAULT
#endif

#endif /* _AVR_UART_UART_CONFIG_H_ */
