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

/**
 * @file uart_config.h
 * @author notweerdmonk
 * @brief UART configuration definitions and defaults
 *
 * This file provides compile-time configuration for the UART module.
 * Default values can be overridden by defining macros before including
 * this header, or by using runtime configuration mode.
 *
 * Configuration options:
 * - UART_BAUD_RATE: Baud rate (default 9600)
 * - UART_CHAR_SIZE: Data bits (default 8)
 * - UART_STOP_BITS: Stop bits (default 1)
 * - UART_PARITY: Parity mode (default none)
 * - UART_TX_BUFFER_LEN: TX buffer size (default 64)
 * - UART_RX_BUFFER_LEN: RX buffer size (default 64)
 * - UART_MAX_SEQ_LEN: Max pattern match length (default 8)
 * - UART_MATCH_MAX: Max number of patterns (default 8)
 *
 * @note Memory-constrained devices may need smaller buffer sizes
 */

#include <port.h>

#ifdef AVR_UART_RUNTIME_CONFIG

/**
 * @brief UART configuration structure for runtime setup
 *
 * This structure is used with uart_setup() when AVR_UART_RUNTIME_CONFIG
 * is defined. It allows configuring UART parameters at runtime rather
 * than compile time.
 *
 * @note All fields have sensible defaults if set to 0
 */
typedef struct uart_config {
  uint32_t baud_rate;    /**< UART baud rate (e.g., 9600, 115200) */
  uint8_t char_size : 4; /**< Number of data bits (5-8) */
  uint8_t stop_bits : 2; /**< Number of stop bits (1 or 2) */
  uint8_t parity : 2;    /**< Parity mode (none/even/odd) */
} uart_config_t;

#endif /* AVR_UART_RUNTIME_CONFIG */

/*
 * NOTE:
 * Mind the size of the RAM when deciding Rx and Tx buffer sizes.
 */

/**
 * @brief Default transmit buffer length in bytes
 */
enum { UART_TX_BUFFER_LEN_DEFAULT = 64 };

/**
 * @brief Default receive buffer length in bytes
 */
enum { UART_RX_BUFFER_LEN_DEFAULT = 64 };

/**
 * @brief Default maximum sequence length for pattern matching
 */
enum { UART_MAX_SEQ_LEN_DEFAULT = 8 };

/**
 * @brief Default maximum number of pattern matches
 */
enum { UART_MATCH_MAX_DEFAULT = 8 };

/**
 * @brief Default UART baud rate
 */
enum { UART_BAUD_DEFAULT = 9600 };

/**
 * @brief Default character size (data bits)
 */
enum { UART_CHAR_SIZE_DEFAULT = 8 };

/**
 * @brief Default number of stop bits
 */
enum { UART_STOP_BITS_DEFAULT = 1 };

#define UART_PARITY_NONE    PORT_UART_PARITY_DISABLED
/**< No parity checking */
#define UART_PARITY_EVEN    PORT_UART_PARITY_EVEN
/**< Even parity */
#define UART_PARITY_ODD     PORT_UART_PARITY_ODD
/**< Odd parity */
#define UART_PARITY_MARK    PORT_UART_PARITY_DISABLED
/**< Mark parity (not supported, treated as none) */
#define UART_PARITY_SPACE   PORT_UART_PARITY_DISABLED
/**< Space parity (not supported, treated as none) */
#define UART_PARITY_DEFAULT UART_PARITY_NONE
/**< Default parity mode */

#ifndef UART_BAUD_RATE
/**
 * @brief UART baud rate override
 *
 * Define this before including uart_config.h to set custom baud rate.
 * Default: 9600
 */
#define UART_BAUD_RATE UART_BAUD_DEFAULT
#endif

#ifndef UART_CHAR_SIZE
/**
 * @brief UART character size override
 *
 * Define this before including uart_config.h to set custom data bits (5-8).
 * Default: 8
 */
#define UART_CHAR_SIZE UART_CHAR_SIZE_DEFAULT
#endif

#ifndef UART_STOP_BITS
/**
 * @brief UART stop bits override
 *
 * Define this before including uart_config.h to set custom stop bits (1 or 2).
 * Default: 1
 */
#define UART_STOP_BITS UART_STOP_BITS_DEFAULT
#endif

#ifndef UART_TX_BUFFER_LEN
/**
 * @brief UART TX buffer size override
 *
 * Define this before including uart_config.h to set custom TX buffer size.
 * Default: 64
 */
#define UART_TX_BUFFER_LEN UART_TX_BUFFER_LEN_DEFAULT
#endif

#ifndef UART_RX_BUFFER_LEN
/**
 * @brief UART RX buffer size override
 *
 * Define this before including uart_config.h to set custom RX buffer size.
 * Default: 64
 */
#define UART_RX_BUFFER_LEN UART_RX_BUFFER_LEN_DEFAULT
#endif

#ifndef UART_PARITY
/**
 * @brief UART parity mode override
 *
 * Define this before including uart_config.h to set custom parity mode.
 * Options: UART_PARITY_NONE, UART_PARITY_EVEN, UART_PARITY_ODD
 * Default: UART_PARITY_NONE
 */
#define UART_PARITY UART_PARITY_NONE
#endif

#ifndef UART_MAX_SEQ_LEN
/**
 * @brief UART max sequence length override
 *
 * Define this before including uart_config.h to set max pattern match length.
 * Default: 8
 */
#define UART_MAX_SEQ_LEN UART_MAX_SEQ_LEN_DEFAULT
#endif

#ifndef UART_MATCH_MAX
/**
 * @brief UART max match count override
 *
 * Define this before including uart_config.h to set max number of patterns.
 * Default: 8
 */
#define UART_MATCH_MAX UART_MATCH_MAX_DEFAULT
#endif

#endif /* _AVR_UART_UART_CONFIG_H_ */
