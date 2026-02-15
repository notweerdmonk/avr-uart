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

#ifndef _AVR_UART_H_
#define _AVR_UART_H_

/**
 * @file uart.h
 * @author notweerdmonk
 * @brief Public API for the UART module
 *
 * This module provides buffered UART communication for AVR microcontrollers.
 * 
 * Features:
 * - FIFO circular buffers for TX and RX
 * - Interrupt-driven transmission and reception
 * - Optional pattern matching callbacks
 * - Optional STDIO integration
 */

#include <stddef.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <uart_config.h>
#include <match.h>
#ifdef __UART_STDIO
#include <stdio.h>
#endif

#define ascii(c) (48 + c)
/**< @brief Convert digit (0-9) to ASCII character */

/**
 * @brief ASCII carriage return character (0x0D)
 */
#define c_RETURN   0x0D

/**
 * @brief ASCII newline character (0x0A)
 */
#define c_NEWLINE  0x0A

/**
 * @brief ASCII tab character (0x09)
 */
#define c_TAB      0x09

/**
 * @brief ASCII backspace character (0x08)
 */
#define c_BKSPACE  0x08

/**
 * @brief ASCII escape character (0x1B)
 */
#define c_ESCAPE   0x1B

/**
 * @brief ASCII delete character (0x7F)
 */
#define c_DEL      0x7F

/* ASCII escape sequences */
/**
 * @brief Escape sequence to clear terminal screen
 */
#define c_CLEARSCREEN_STRING "\x1b\x5b\x48\x1b\x5b\x32\x4a"

/* CRLF */
/**
 * @brief Carriage return + newline string
 */
#define c_NEWLINE_STRING "\x0d\x0a"

#ifdef __RUNTIME_CONFIG

/**
 * @brief Initialize UART with runtime configuration
 *
 * Configures the UART hardware with the specified parameters at runtime.
 * This allows changing UART settings after compilation.
 *
 * @param config Pointer to uart_config structure containing baud rate,
 *               character size, stop bits, and parity settings
 *
 * @note If config is NULL or baud_rate is 0, defaults will be used
 * @note This function is only available when __RUNTIME_CONFIG is defined
 *
 * @example
 * @code
 * struct uart_config cfg = {
 *     .baud_rate = 115200,
 *     .char_size = 8,
 *     .stop_bits = 1,
 *     .parity = UART_PARITY_NONE
 * };
 * uart_setup(&cfg);
 * @endcode
 */
void uart_setup(struct uart_config *config);

#else /* !__RUNTIME_CONFIG */

/**
 * @brief Initialize UART with compile-time configuration
 *
 * Configures the UART hardware using the settings defined in uart_config.h
 * at compile time. This is the default mode when __RUNTIME_CONFIG is
 * not defined.
 *
 * @note Configuration is determined by UART_BAUD_RATE, UART_CHAR_SIZE,
 *       UART_STOP_BITS, and UART_PARITY macros in uart_config.h
 *
 * @example
 * @code
 * // In uart_config.h:
 * // #define UART_BAUD_RATE 9600
 * // #define UART_CHAR_SIZE 8
 * // #define UART_STOP_BITS 1
 * // #define UART_PARITY UART_PARITY_NONE
 *
 * uart_setup();  // Uses compile-time settings
 * @endcode
 */
void uart_setup();

#endif /* __RUNTIME_CONFIG */

/**
 * @brief Flush the receive buffer
 *
 * Clears all data from the RX buffer and resets buffer indices.
 * Does not wait for transmission to complete.
 */
void uart_flush_rx(void);

/**
 * @brief Flush the transmit buffer
 *
 * Waits for all data in the TX buffer to be transmitted before
 * clearing the buffer and resetting indices.
 */
void uart_flush_tx(void);

/**
 * @brief Flush both TX and RX buffers
 *
 * Combines uart_flush_rx() and uart_flush_tx() to clear both
 * transmit and receive buffers. Note that uart_flush_tx() blocks
 * until all pending transmission is complete.
 */
#define uart_flush() uart_flush_rx(), uart_flush_tx()

/**
 * @brief Peek at next byte in receive buffer without removing it
 *
 * Reads the next byte that would be returned by uart_recv_byte() without
 * removing it from the buffer. Useful for checking incoming data without
 * consuming it.
 *
 * @return The next character in the buffer, or 0 if buffer is empty
 *
 * @code
 * char c = uart_peek_byte();  // Check next byte without consuming
 * @endcode
 */
char uart_peek_byte(void);

/**
 * @brief Receive a single byte from UART (blocking)
 *
 * Waits until a byte is available in the receive buffer and returns it.
 * This function blocks until data is received.
 *
 * @return The received character (0-255)
 *
 * @note This function will block indefinitely if no data is received
 * @warning Disables interrupts briefly during buffer access
 *
 * @code
 * char c = uart_recv_byte();  // Blocks until data received
 * @endcode
 */
char uart_recv_byte(void);

/**
 * @brief Try to receive a single byte from UART (non-blocking)
 *
 * Attempts to receive a byte from the buffer without blocking.
 * Returns immediately regardless of whether data is available.
 *
 * @return The received character, or 0 if buffer is empty
 *
 * @note Use this for polling-based receive patterns
 * @warning Disables interrupts briefly during buffer access
 *
 * @code
 * char c;
 * while ((c = uart_try_recv_byte()) == 0) {
 *     // Do other work
 * }
 * @endcode
 */
char uart_try_recv_byte(void);

/**
 * @brief Peek multiple bytes from receive buffer without removing
 *
 * Copies up to n bytes from the receive buffer into str without removing
 * them from the buffer. Blocks until at least n bytes are available.
 *
 * @param str Buffer to store peeked bytes
 * @param n   Maximum number of bytes to peek
 * @return Number of bytes actually peeked
 *
 * @note Blocks until n bytes are received
 *
 * @code
 * char buffer[32];
 * uart_peek(buffer, 10);  // Peek 10 bytes without removing
 * @endcode
 */
size_t uart_peek(char* str, size_t n);

/**
 * @brief Receive multiple bytes from UART
 *
 * Reads exactly n bytes from the receive buffer, blocking until all
 * bytes are available.
 *
 * @param str Buffer to store received bytes
 * @param n   Number of bytes to receive
 * @return Number of bytes actually received
 *
 * @note Blocks until n bytes are received
 *
 * @code
 * char buffer[32];
 * uart_recv(buffer, 10);  // Blocks until 10 bytes received
 * @endcode
 */
size_t uart_recv(char* str, size_t n);

/**
 * @brief Send a single byte via UART
 *
 * Adds a byte to the transmit buffer. If the buffer is full, this
 * function blocks until space becomes available.
 *
 * @param c Character to send (0-255)
 *
 * @note Transmission occurs asynchronously via ISR
 *
 * @code
 * uart_send_byte('A');       // Send single character
 * uart_send_byte(0x55);      // Send hex value
 * @endcode
 */
void uart_send_byte(char c);

/**
 * @brief Try to send a single byte via UART (non-blocking)
 *
 * Attempts to add a byte to the transmit buffer without blocking.
 *
 * @param c Character to send (0-255)
 * @return Non-zero if successful, 0 if buffer is full
 *
 * @note Use this for non-blocking transmit
 *
 * @code
 * while (!uart_try_send_byte('A')) {
 *     // Wait for buffer space
 * }
 * @endcode
 */
char uart_try_send_byte(char c);

/**
 * @brief Send multiple bytes via UART
 *
 * Transmits a string of exactly len bytes. Each byte is added to the
 * transmit buffer and sent asynchronously.
 *
 * @param str Pointer to data to send
 * @param len Number of bytes to send
 *
 * @note Blocks if TX buffer becomes full
 *
 * @code
 * uart_send("Hello", 5);    // Send string from RAM
 * uart_send(data, sizeof(data));  // Send buffer
 * @endcode
 */
void uart_send(const char* str, size_t len);

/**
 * @brief Send a string from program memory
 *
 * Similar to uart_send() but reads from program memory (flash) instead
 * of RAM. Useful for sending constant strings without consuming RAM.
 *
 * @param str Pointer to string in program memory (PGM_P type)
 *
 * @note Uses pgm_read_byte() to access data
 * @note String must be null-terminated
 */
void uart_pgm_send(PGM_P str);

/**
 * @brief Send an unsigned integer as decimal text
 *
 * Converts an unsigned integer to its decimal string representation
 * and sends it via UART.
 *
 * @param u Unsigned integer to send
 *
 * @code
 * uart_send_uint(12345);  // Sends "12345"
 * @endcode
 */
void uart_send_uint(unsigned int u);

/**
 * @brief Send a signed integer as decimal text
 *
 * Converts a signed integer to its decimal string representation
 * and sends it via UART. Handles negative numbers.
 *
 * @param n Signed integer to send
 *
 * @code
 * uart_send_int(-42);  // Sends "-42"
 * @endcode
 */
void uart_send_int(int n);

/**
 * @brief Send a floating-point number as text
 *
 * Converts a float to its string representation with specified
 * number of decimal places and sends via UART.
 *
 * @param f Float to send
 * @param m Number of decimal places (0-4)
 *
 * @note Maximum 4 decimal places supported
 * @note Fractional part > 4 digits may not work correctly
 *
 * @code
 * uart_send_float(3.14159, 2);  // Sends "3.14"
 * @endcode
 */
void uart_send_float(float f, uint8_t m);

/**
 * @brief Send a newline (CRLF)
 *
 * Sends carriage return + line feed sequence. Use this for
 * compatibility with terminals that expect CRLF line endings.
 */
void uart_newline(void);

/**
 * @brief Send string followed by newline
 *
 * Macro that combines uart_send() and uart_newline() for
 * convenient line output.
 *
 * @param str Pointer to string to send
 * @param len Length of string
 */
#define uart_sendln(str, len) uart_send((str), (len)), uart_newline()

/**
 * @brief Clear the terminal screen
 *
 * Sends an ANSI escape sequence that clears the terminal screen
 * and moves cursor to home position.
 */
void uart_clear(void);

#endif /* _AVR_UART_H_ */
