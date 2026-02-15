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

#ifdef __UART_MATCH

/**
 * @file match.c
 * @author notweerdmonk
 * @brief Pattern matching implementation for UART
 *
 * This file implements the pattern matching functionality that detects
 * specific character sequences in incoming UART data and triggers
 * callback handlers.
 *
 * @note This file is only compiled when __UART_MATCH is defined
 */

#include <stdint.h>
#include <uart.h>

/**
 * @internal
 * @brief Pattern match state structure
 *
 * Maintains state for all registered patterns and their match progress.
 */
static
struct _uart_match {
  uint8_t match_idx_max;
  struct _match {
    uint8_t count : 4;
    uint8_t len   : 4;
    char seq[UART_MAX_SEQ_LEN];
    void (*event_handler)(void *);
    void *data;
  } match[UART_MATCH_MAX];
  uint16_t triggered_mask;
} match;

uint8_t uart_register_match(const char *str, uart_match_handler handler,
    void *data) {
  if (!handler) {
    return -1;
  }
  if (match.match_idx_max == UART_MATCH_MAX) {
    return -1;
  }

  struct _match *p_match = &match.match[match.match_idx_max];
  uint8_t i = 0;

  while ( (i < UART_MAX_SEQ_LEN) && (*str != '\0') ) {
    p_match->seq[i++] = *str++;
  }
  p_match->seq[i] = '\0';
  p_match->len = i;

  p_match->event_handler = handler;
  p_match->data = data;

  match.match_idx_max++;

  return 0;
}

void uart_deregister_match(const char *str) {

  if (str == NULL) {
    return;
  }

  uint8_t i = 0;
  uint8_t found = 0;

  for (; i < match.match_idx_max; i++) {
    struct _match *p_match = &match.match[i];

#ifdef __STRNCMP_MATCH
    if (strncmp(p_match->seq, str, p_match->len) == 0) {
      found = 1;
      break;
    }
#else
    if (0 == ({
          uint8_t ret = 0;
          size_t n = p_match->len;
          char *p = &p_match->seq[0];
          char *q = (char*)&str[0];

          while (n--) {
            if (*p != *q) {
              ret = 1;
              break;
            }
            if (*p == '\0') {
              break;
            }
            p++;
            q++;
          }
          ret;
        })) {
      found = 1;
      break;
    }
#endif
  }

  if (found) {
    for (; i < match.match_idx_max - 1; i++) {
      match.match[i] = match.match[i+1];
    }
    match.match_idx_max--;
  }
}

void uart_check_match() {

  uint16_t triggered_mask = match.triggered_mask;
  uint16_t triggered = 1;

  for (uint8_t i = 0; i < match.match_idx_max; i++) {
    if (triggered_mask & 1) {
      struct _match *p_match = &match.match[i];

      if (p_match->event_handler) {
        (*p_match->event_handler)(p_match->data);
      }

      match.triggered_mask &= ~triggered;
    }

    triggered_mask = triggered_mask >> 1;
    triggered = triggered << 1;
  }
}

/**
 * @internal
 * @brief Process incoming byte for pattern matching
 *
 * Called from UART RX ISR to check incoming bytes against registered
 * patterns. Updates match progress and sets triggered flags when
 * patterns complete.
 *
 * @param udr The received byte to check
 *
 * @note Runs in ISR context - keeps processing fast and simple
 * @note Uses partial matching - resets on mismatch but restarts if
 *       next byte matches pattern start
 */
void uart_do_match(uint8_t udr) {

  if (match.match_idx_max == 0) {
    return;
  }

  uint16_t match_id_mask = 1;

  for (uint8_t i = 0; i < match.match_idx_max; i++) {
    struct _match *p_match = &match.match[i];

    if (udr == p_match->seq[p_match->count]) {
      if (++p_match->count == p_match->len) {
        p_match->count = 0;
        match.triggered_mask |= match_id_mask;
        return;
      }
    } else if (p_match->count > 0) {
      if (udr == p_match->seq[0]) {
        p_match->count = 1;
      } else {
        p_match->count = 0;
      }
    }

    match_id_mask = match_id_mask << 1;
  }
}

#endif /* __UART_MATCH */
