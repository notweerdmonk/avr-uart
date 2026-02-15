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

#ifndef _AVR_UART_PORT_H
#define _AVR_UART_PORT_H

/**
 * @file port.h
 * @author notweerdmonk
 * @brief Port abstraction layer for UART hardware
 *
 * This header provides the port abstraction layer that maps to
 * target-specific UART hardware registers and interrupt vectors.
 * Each supported AVR variant has its own port implementation.
 *
 * The port layer defines:
 * - Parity mode enumerations
 * - Interrupt vector names
 * - UART register macros
 * - Hardware setup functions
 *
 * @note Include the appropriate MCU header before this file
 *       (e.g., __AVR_ATmega328P__)
 */

/**
 * @brief UART parity mode selection
 *
 * Used to configure UART parity checking mode.
 */
typedef enum port_uart_parity {
  PORT_UART_PARITY_DISABLED = 0, /**< No parity */
  PORT_UART_PARITY_EVEN,          /**< Even parity */
  PORT_UART_PARITY_ODD           /**< Odd parity */
} port_uart_parity_t;

#ifdef __AVR_ATmega328P__
#include <mega328p/port_mega328p.h>
#endif /* __AVR_ATmega328P__ */

#endif /* _AVR_UART_PORT_H */
