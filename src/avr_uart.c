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
 * @file avr_uart.c
 * @author notweerdmonk
 * @brief UART module implementation
 *
 * This file implements the buffered UART functionality for AVR microcontrollers.
 * Uses circular FIFO buffers for both transmit and receive, with interrupt-driven
 * data movement.
 */

#include <string.h>
#include <avr_uart.h>
#include <avr_portable.h>
#include <avr_ascii.h>
#include <util/delay.h>


/* FIFO buffered UART for AVR family of microcontrollers. */

/*****************************************************************************/
/* Variables */
/*****************************************************************************/

/**
 * @brief UART internal state structure
 *
 * Contains all runtime state for the UART module including
 * circular buffer management and data storage.
 */
static
struct _uart {

#undef RX_COUNT_SIZE_TYPE
#if (UART_RX_BUFFER_LEN < 256)
#define RX_COUNT_SIZE_TYPE uint8_t
#elif (UART_RX_BUFFER_LEN < 65536)
#define RX_COUNT_SIZE_TYPE uint16_t
#elif (UART_RX_BUFFER_LEN < 4294967296)
#define RX_COUNT_SIZE_TYPE uint32_t
#else
#error UART_RX_BUFFER_LEN too large
#endif

#undef TX_COUNT_SIZE_TYPE
#if (UART_TX_BUFFER_LEN < 256)
#define TX_COUNT_SIZE_TYPE uint8_t
#elif (UART_TX_BUFFER_LEN < 65536)
#define TX_COUNT_SIZE_TYPE uint16_t
#elif (UART_TX_BUFFER_LEN < 4294967296)
#define TX_COUNT_SIZE_TYPE uint16_t
#else
#error UART_TX_BUFFER_LEN too large
#endif

 volatile RX_COUNT_SIZE_TYPE rx_count;
 volatile TX_COUNT_SIZE_TYPE tx_count;

 unsigned char rx_in;
 unsigned char rx_out;

 unsigned char tx_in;
 unsigned char tx_out;

 char rx_buffer[UART_RX_BUFFER_LEN];
 char tx_buffer[UART_TX_BUFFER_LEN];

} uart;


/*****************************************************************************/
/* Extern linkages */
/*****************************************************************************/

/**
 * @internal
 * @brief External pattern matching handler
 *
 * Called from RX ISR when AVR_UART_MATCH is enabled to check
 * incoming bytes against registered patterns.
 *
 * @param udr The received byte from UART data register
 */
extern void avr_uart_do_match(uint8_t udr);

/*****************************************************************************/
/* Functions */
/*****************************************************************************/

#ifdef AVR_UART_STDIO

/**
 * @internal
 * @brief STDIO stream putchar implementation
 *
 * Callback function for STDIO stream to send characters via UART.
 * Automatically converts newline to carriage return + newline.
 *
 * @param c     Character to send
 * @param stream FILE stream (unused)
 * @return 0 on success, EOF on error
 */
int avr_uart_stream_putchar(char c, FILE *stream) {
  if (c == '\n') {
    avr_uart_stream_putchar('\r', stream);
  }
  avr_uart_send_byte(c);
  return 0;
}

/**
 * @internal
 * @brief STDIO stream getchar implementation
 *
 * Callback function for STDIO stream to receive characters from UART.
 *
 * @param stream FILE stream (unused)
 * @return Received character or EOF if error
 *
 * @note Returns EOF for null character (0x00)
 */
int avr_uart_stream_getchar(FILE *stream) {
  /* 
   * A null character can be received but shall get interpreted as string
   * terminator.
   */
  char c;
  return (c = avr_uart_recv_byte()) == '\0' ? EOF : c;
}

/**
 * @internal
 * @brief STDIO stream object
 *
 * Combined input/output stream configured for UART communication.
 */
static
FILE __uart_iostream = FDEV_SETUP_STREAM(avr_uart_stream_putchar,
    avr_uart_stream_getchar, _FDEV_SETUP_RW);

#endif /* AVR_UART_STDIO */

#ifdef AVR_UART_RUNTIME_CONFIG

void avr_uart_setup(struct avr_uart_config *config) {
  if (!config) {
    return;
  }

  if (config->baud_rate == 0) {
    config->baud_rate = UART_BAUD_DEFAULT;
  }

  PORT_UART_SET_BAUD_RATE(config->baud_rate);
  PORT_UART_SET_CHAR_SIZE(config->char_size);
  PORT_UART_SET_STOP_BITS(config->stop_bits);
  PORT_UART_SET_PARITY(config->parity);

#else /* !AVR_UART_RUNTIME_CONFIG */

void avr_uart_setup() {

  PORT_UART_SET_BAUD_RATE(UART_BAUD_RATE);
  PORT_UART_SET_CHAR_SIZE(UART_CHAR_SIZE);
  PORT_UART_SET_STOP_BITS(UART_STOP_BITS);
  PORT_UART_SET_PARITY(UART_PARITY);

#endif /* AVR_UART_RUNTIME_CONFIG */

  PORT_UART_INIT();

#ifdef AVR_UART_STDIO
  stdout = stdin = stderr = &__uart_iostream;
#endif /* AVR_UART_STDIO */

  avr_uart_flush();
}

/**
 * @internal
 * @brief UART Data Register Empty ISR
 *
 * Interrupt service routine triggered when the UART transmit data
 * register is empty and ready for new data. Transfers bytes from
 * the TX circular buffer to the UART hardware.
 *
 * @note Runs in ISR context with other interrupts blocked
 */
ISR(PORT_UDRE_VECT, ISR_BLOCK) {

  if (uart.tx_count > 0) {
    PORT_UDR = uart.tx_buffer[uart.tx_out];

    if (++uart.tx_out == UART_TX_BUFFER_LEN) {
      uart.tx_out = 0;
    }

    if(--uart.tx_count == 0) {
      PORT_DISABLE_UDRE_INTERRUPT();
    }
  }
}

/**
 * @internal
 * @brief UART Receive Complete ISR
 *
 * Interrupt service routine triggered when a byte is received
 * via UART. Stores the byte in the RX circular buffer and
 * optionally checks for pattern matches.
 *
 * @note Runs in ISR context with other interrupts blocked
 */
ISR(PORT_RXC_VECT, ISR_BLOCK) {

#ifdef AVR_UART_MATCH

  uint8_t udr = PORT_UDR;
  avr_uart_do_match(udr);

#else /* !AVR_UART_MATCH */

#define udr PORT_UDR

#endif /* AVR_UART_MATCH */

  uart.rx_buffer[uart.rx_in] = udr;
  ++uart.rx_count;

  if (++uart.rx_in == UART_RX_BUFFER_LEN) {
    uart.rx_in = 0;
  }
}

void avr_uart_flush_rx() {

  uart.rx_in = uart.rx_out = 0;
  uart.rx_count = 0;
}

void avr_uart_flush_tx() {

  while(uart.tx_count > 0);

  uart.tx_in = uart.tx_out = 0;
  uart.tx_count = 0;
}

char avr_uart_peek_byte(void) {

  return (uart.rx_count > 0 ? uart.rx_buffer[uart.rx_out]: 0);
}

char avr_uart_recv_byte() {

  /* Wait till atleast one character has been received */
  while ( !(uart.rx_count > 0) );

  PORT_DISABLE_RXC_INTERRUPT();

  uart.rx_count--;
  char c = uart.rx_buffer[uart.rx_out];
  if (++uart.rx_out == UART_RX_BUFFER_LEN) {
    uart.rx_out = 0;
  }

  PORT_ENABLE_RXC_INTERRUPT();

  return c;
}

char avr_uart_try_recv_byte() {

  return (uart.rx_count > 0 ?
      ({
        PORT_DISABLE_RXC_INTERRUPT();

        uart.rx_count--;
        char c = uart.rx_buffer[uart.rx_out];
        if (++uart.rx_out == UART_RX_BUFFER_LEN) {
          uart.rx_out = 0;
        }

        PORT_ENABLE_RXC_INTERRUPT();

        c;
      }) :
    0);
}

size_t avr_uart_peek(char *str, size_t len) {

  if (len > UART_RX_BUFFER_LEN) {
    len = UART_RX_BUFFER_LEN;
  }

  /* Wait till atleast len characters has been received */
  while (uart.rx_count < len);

  unsigned char rx_out = uart.rx_out;
  size_t n = 0;

  while (len--) {
    *str++ = uart.rx_buffer[rx_out++];
    if (rx_out == UART_RX_BUFFER_LEN) {
      rx_out = 0;
    }
    n++;
  }

  return n;
}

/*
 * When `len` is greater than `rx_count`,
 * avr_uart_recv_byte will block and the loop shall run until len reaches 0.
 */
size_t avr_uart_recv(char* str, size_t len) {

  size_t n = 0;

  while(len--) {
    *str++ = avr_uart_recv_byte();
    n++;
  }

  return n;
}

void avr_uart_send_byte(char c) {

  while (uart.tx_count == UART_TX_BUFFER_LEN);

  PORT_DISABLE_UDRE_INTERRUPT();

  ++uart.tx_count;
  uart.tx_buffer[uart.tx_in] = c;
  if (++uart.tx_in == UART_TX_BUFFER_LEN) {
    uart.tx_in = 0;
  }

  PORT_ENABLE_UDRE_INTERRUPT();
}

char avr_uart_try_send_byte(char c) {

  return ( (uart.tx_count < UART_TX_BUFFER_LEN) &&
      ({
        PORT_DISABLE_UDRE_INTERRUPT();

        ++uart.tx_count;
        uart.tx_buffer[uart.tx_in] = c;
        if (++uart.tx_in == UART_TX_BUFFER_LEN) {
          uart.tx_in = 0;
        }

        PORT_ENABLE_UDRE_INTERRUPT();
      })
    );
}

void avr_uart_send(const char *s, size_t len) {

  while (len--) avr_uart_send_byte(*s++);
}

void avr_uart_pgm_send(PGM_P s) {

  for (char c = pgm_read_byte(s); c != 0; c = pgm_read_byte(++s)) {
    avr_uart_send_byte(c);
  } 
}

void avr_uart_send_uint(unsigned int u) {

#if (__SIZEOF_INT__ == 1)
  uint8_t digits[4];
#elif (__SIZEOF_INT__ == 2)
  uint8_t digits[6];
#endif
  uint8_t idx = 0;

  if (u == 0) {
    digits[idx++] = 0;
  } else {
    while (u > 0) {
      digits[idx++] = u % 10;
      u /= 10;
    }
  }

  while (idx > 0) {
    avr_uart_send_byte(avr_util_ascii(digits[--idx]));
  }
}

void avr_uart_send_int(int n) {

  if (n < 0) {
    avr_uart_send_byte('-');
    n = -n;
  }

  avr_uart_send_uint(n);
}

/* TODO: sprintf with -lprintf_flt or dtostrf, but bigger size */
void avr_uart_send_float(float f, uint8_t m) {

  /* NOTE: fractional part greater than 4 does not work */
  if (m > 4) m = 4;

  avr_uart_send_int(f);
  avr_uart_send_byte('.');

  f = f - (int)f;
  for (uint8_t i = 0; i < m; i++) {
    f = f * 10;
  }

  avr_uart_send_int(f);
}

void avr_uart_send_double(double d, uint8_t m) {

}

void avr_uart_newline() {
  avr_uart_send(AVR_UTIL_CONSTANT_CRLF, 2);
}

void avr_uart_clear() {
  avr_uart_send(AVR_UTIL_CONSTANT_CLEARSCREEN_STRING, 7);
}
