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

#ifndef _AVR_UART_CONFIG_H_
#define _AVR_UART_CONFIG_H_

/*
 * Define macros in this file to configure features and compilation behaviour.
 * Uncomment the line(s) which contain definition of desired macro(s).
 */

/* Enable runtime UART configuration (call uart_setup with config struct) */
//#define AVR_UART_RUNTIME_CONFIG

/* Enable use of UART in I/O streams (stdin/stdout/stderr) */
//#define AVR_UART_UART_IOSTREAM

/* Enable UART input pattern match */
//#define AVR_UART_UART_MATCH

/* Use strncmp for pattern matching */
//#define AVR_UART_STRNCMP_MATCH

/* Emit a trigger signal that can be used by logic analyser to start capture */
//#define AVR_UART_EMIT_TRIGGER

/* SIM denotes that source code will compiled for simulation */
//#define AVR_UART_SIMULATION

/* Denotes that source code will be compiled for off-target testing */
//#define AVR_UART_SIMTEST

/* Demo mode with serial communication program */
//#define AVR_UART_DEMO

/* Enable debug build with debugging information and symbols */
//#define AVR_UART_DEBUG

/* Preserve compilation intermediaries */
//#define AVR_UART_SAVE_TEMPS

/* Set the compiler optimization level (-O0, -O1, -O2, -O3, -Os, etc.) */
//#define AVR_UART_OPTIM -O2

#endif /* _AVR_UART_CONFIG_H_ */
