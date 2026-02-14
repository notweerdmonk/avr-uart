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

/* Enable use of UART in I/O streams (stdin/stdout/stderr) */
//#define __UART_IOSTREAM 1

/* Enable UART input pattern match */
//#define __UART_MATCH 1

/* Use strncmp for pattern matching */
//#define __STRNCMP_MATCH 1

/* Emit a trigger signal that can be used by logic analyser to start capture */
//#define __EMIT_TRIGGER 1

/* SIM denotes that source code will compiled for simulation */
//#define __SIMULATION 1

/* Denotes that source code will be compiled for off-target testing */
//#define __SIMTEST 1

/* Demo mode with serial communication program */
//#define __DEMO 1

/* Enable debug build with debugging information and symbols */
//#define __DEBUG 1

/* Preserve compilation intermediaries */
//#define __SAVE_TEPMS 1

/* Set the compiler optimization level */
//#define __OPTIM 1

#endif /* _AVR_UART_CONFIG_H_ */
