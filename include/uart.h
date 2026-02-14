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

/**
 * @file uart.h
 * @author notweerdmonk
 * @brief Header file for the UART module
 */
#ifndef _AVR_UART_H_
#define _AVR_UART_H_

#include <stddef.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <uart_config.h>
#include <match.h>
#ifdef __UART_STDIO
#include <stdio.h>
#endif

#define ascii(c) (48 + c)

/* ASCII character codes */
#define c_RETURN   0x0D
#define c_NEWLINE  0x0A
#define c_TAB      0x09
#define c_BKSPACE  0x08
#define c_ESCAPE   0x1B
#define c_DEL      0x7F

/* ASCII escape sequences */
#define c_CLEARSCREEN_STRING "\x1b\x5b\x48\x1b\x5b\x32\x4a"

/* CRLF */
#define c_NEWLINE_STRING "\x0d\x0a"

#ifdef UART_RUNTIME_CONFIG

void uart_setup(struct uart_config *config);

#else /* !UART_RUNTIME_CONFIG */

void uart_setup();

#endif /* UART_RUNTIME_CONFIG */

void uart_flush_rx(void);

void uart_flush_tx(void);

#define uart_flush() uart_flush_rx(), uart_flush_tx()

/**
 * @brief Peek a single character from buffer.
 *
 * @return A single character.
 */
char uart_peek_byte(void);

/**
 * @brief Read a single charater from buffer.
 *
 * The function will block until a byte is available in the buffer.
 *
 * @return A single character.
 */
char uart_recv_byte(void);

/**
 * @brief Read a single charater from buffer.
 *
 * The function will not block. A NULL character is returned when the buffer is
 * empty.
 *
 * @return A single character.
 */
char uart_try_recv_byte(void);

/**
 * @brief 
 */
size_t uart_peek(char* str, size_t n);

size_t uart_recv(char* str, size_t n);

void uart_send_byte(char c);

char uart_try_send_byte(char c);

void uart_send(const char* str, size_t len);

void uart_pgm_send(PGM_P str);

void uart_send_uint(unsigned int u);

void uart_send_int(int n);

void uart_send_float(float f, uint8_t m);

void uart_newline(void);

#define uart_sendln(str, len) uart_send((str), (len)), uart_newline()

void uart_clear(void);

#endif /* _AVR_UART_H_ */
