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

#ifndef _AVR_UART_MATCH_H_
#define _AVR_UART_MATCH_H_

/**
 * @file match.h
 * @author notweerdmonk
 * @brief Pattern matching API for UART
 *
 * This module provides pattern matching functionality that triggers
 * callbacks when specific sequences are received via UART.
 *
 * Features:
 * - Register multiple patterns to match against incoming data
 * - Non-blocking pattern detection in ISR context
 * - Callback functions executed when patterns are matched
 *
 * @note This feature requires __UART_MATCH to be defined
 */

#include <stdint.h>
#include <uart.h>

/**
 * @brief Callback function type for pattern matches
 *
 * This is the function pointer type that handlers must match.
 * The handler receives a user-provided data pointer that was
 * passed during pattern registration.
 *
 * The data parameter contains the user-provided context pointer.
 *
 * @code
 * void my_handler(void *data) {
 *     // Handle pattern match
 *     (void)data;  // unused
 * }
 * @endcode
 */
typedef void (*uart_match_handler)(void *);

/**
 * @brief Register a pattern to match against incoming UART data
 *
 * Registers a character sequence that will trigger a callback when
 * completely received via UART.
 *
 * @param str     Pattern string to match (null-terminated)
 * @param handler Callback function to execute when pattern matches
 * @param data    User data to pass to callback (can be NULL)
 * @return 0 on success, -1 on failure (no handler or max patterns reached)
 *
 * @note Pattern length cannot exceed UART_MAX_SEQ_LEN
 * @note Maximum UART_MATCH_MAX patterns can be registered
 * @note Pattern matching occurs in ISR context
 *
 * @code
 * void on_command(void *data) {
 *     uart_send("Command received!\r\n", 20);
 * }
 *
 * // Register pattern "cmd" to trigger callback
 * uart_register_match("cmd", on_command, NULL);
 * @endcode
 */
uint8_t uart_register_match(const char *str, uart_match_handler handler,
    void *data);

/**
 * @brief Deregister a previously registered pattern
 *
 * Removes a pattern from the match list.
 *
 * @param str Pattern string to remove (must match exactly)
 *
 * @code
 * uart_deregister_match("cmd");  // Remove "cmd" pattern
 * @endcode
 */
void uart_deregister_match(const char *str);

/**
 * @brief Check for and process triggered pattern matches
 *
 * Must be called periodically (e.g., in main loop) to process
 * patterns that were matched in the ISR. Calls the registered
 * handler for each triggered pattern.
 *
 * @note This function should be called from non-ISR context
 *
 * @code
 * while (1) {
 *     uart_check_match();  // Process any triggered patterns
 *     // Do other work
 * }
 * @endcode
 */
void uart_check_match();

#endif /* _AVR_UART_MATCH_H_ */
