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
 * @file uart.c
 * @author notweerdmonk
 * @brief Implements the UART module
 */
#include <uart.h>
#include <port.h>
#include <string.h>
#include <util/delay.h>


/* FIFO buffered UART for AVR family of microcontrollers. */

/*****************************************************************************/
/* Variables */
/*****************************************************************************/

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
extern void uart_do_match(uint8_t udr);

/*****************************************************************************/
/* Functions */
/*****************************************************************************/

#ifdef __UART_STDIO

int uart_stream_putchar(char c, FILE *stream) {
  if (c == '\n') {
    uart_stream_putchar('\r', stream);
  }
  uart_send_byte(c);
  return 0;
}

int uart_stream_getchar(FILE *stream) {
  /* 
   * A null character can be received but shall get interpreted as string
   * terminator.
   */
  char c;
  return (c = uart_recv_byte()) == '\0' ? EOF : c;
}

static
FILE __uart_iostream = FDEV_SETUP_STREAM(uart_stream_putchar,
    uart_stream_getchar, _FDEV_SETUP_RW);

#endif /* __UART_STDIO */

#ifdef UART_RUNTIME_CONFIG

void uart_setup(struct uart_config *config) {
  if (!config) {
    return;
  }

  if (config->baud_rate == 0) {
    config->baud_rate = UART_BAUD_DEFAULT;
  }

  port_uart_set_baud_rate(config->baud_rate);
  port_uart_set_char_size(config->char_size);
  port_uart_set_stop_bits(config->stop_bits);
  port_uart_set_parity(config->parity);

#else

void uart_setup() {

  port_uart_set_baud_rate(UART_BAUD_RATE);
  port_uart_set_char_size(UART_CHAR_SIZE);
  port_uart_set_stop_bits(UART_STOP_BITS);
  port_uart_set_parity(UART_PARITY);

#endif /* UART_RUNTIME_CONFIG */

  port_uart_setup();

#ifdef __UART_STDIO
  stdout = stdin = stderr = &__uart_iostream;
#endif

  uart_flush();
}

ISR(port_udre_vect, ISR_BLOCK) {

  if (uart.tx_count > 0) {
    PORT_UDR = uart.tx_buffer[uart.tx_out];

    if (++uart.tx_out == UART_TX_BUFFER_LEN) {
      uart.tx_out = 0;
    }

    if(--uart.tx_count == 0) {
      port_disable_udre_interrupt();
    }
  }
}

ISR(port_rxc_vect, ISR_BLOCK) {

#ifdef __UART_MATCH
  uint8_t udr = PORT_UDR;
  uart_do_match(udr);
#else
#define udr PORT_UDR
#endif

  uart.rx_buffer[uart.rx_in] = udr;
  ++uart.rx_count;

  if (++uart.rx_in == UART_RX_BUFFER_LEN) {
    uart.rx_in = 0;
  }
}

void uart_flush_rx() {

  uart.rx_in = uart.rx_out = 0;
  uart.rx_count = 0;
}

void uart_flush_tx() {

  while(uart.tx_count > 0);

  uart.tx_in = uart.tx_out = 0;
  uart.tx_count = 0;
}

char uart_peek_byte(void) {

  return (uart.rx_count > 0 ? uart.rx_buffer[uart.rx_out]: 0);
}

char uart_recv_byte() {

  /* Wait till atleast one character has been received */
  while ( !(uart.rx_count > 0) );

  port_disable_rxc_interrupt();

  uart.rx_count--;
  char c = uart.rx_buffer[uart.rx_out];
  if (++uart.rx_out == UART_RX_BUFFER_LEN) {
    uart.rx_out = 0;
  }

  port_enable_rxc_interrupt();

  return c;
}

char uart_try_recv_byte() {

  return (uart.rx_count > 0 ?
      ({
        port_disable_rxc_interrupt();

        uart.rx_count--;
        char c = uart.rx_buffer[uart.rx_out];
        if (++uart.rx_out == UART_RX_BUFFER_LEN) {
          uart.rx_out = 0;
        }

        port_enable_rxc_interrupt();

        c;
      }) :
    0);
}

size_t uart_peek(char *str, size_t len) {

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
 * uart_recv_byte will block and the loop shall run until len reaches 0.
 */
size_t uart_recv(char* str, size_t len) {

  size_t n = 0;

  while(len--) {
    *str++ = uart_recv_byte();
    n++;
  }

  return n;
}

void uart_send_byte(char c) {

  while (uart.tx_count == UART_TX_BUFFER_LEN);

  port_disable_udre_interrupt();

  ++uart.tx_count;
  uart.tx_buffer[uart.tx_in] = c;
  if (++uart.tx_in == UART_TX_BUFFER_LEN) {
    uart.tx_in = 0;
  }

  port_enable_udre_interrupt();
}

char uart_try_send_byte(char c) {

  return ( (uart.tx_count < UART_TX_BUFFER_LEN) &&
      ({
        port_disable_udre_interrupt();

        ++uart.tx_count;
        uart.tx_buffer[uart.tx_in] = c;
        if (++uart.tx_in == UART_TX_BUFFER_LEN) {
          uart.tx_in = 0;
        }

        port_enable_udre_interrupt();
      })
    );
}

void uart_send(const char *s, size_t len) {

  while (len--) uart_send_byte(*s++);
}

void uart_pgm_send(PGM_P s) {

  for (char c = pgm_read_byte(s); c != 0; c = pgm_read_byte(++s)) {
    uart_send_byte(c);
  } 
}

void uart_send_uint(unsigned int u) {

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
    uart_send_byte(ascii(digits[--idx]));
  }
}

void uart_send_int(int n) {

  if (n < 0) {
    uart_send_byte('-');
    n = -n;
  }

  uart_send_uint(n);
}

/* TODO: sprintf with -lprintf_flt or dtostrf, but bigger size */
void uart_send_float(float f, uint8_t m) {

  /* NOTE: fractional part greater than 4 does not work */
  if (m > 4) m = 4;

  uart_send_int(f);
  uart_send_byte('.');

  f = f - (int)f;
  for (uint8_t i = 0; i < m; i++) {
    f = f * 10;
  }

  uart_send_int(f);
}

void uart_send_double(double d, uint8_t m) {

}

void uart_newline() {
  uart_send(c_NEWLINE_STRING, 2);
}

void uart_clear() {
  uart_send(c_CLEARSCREEN_STRING, 7);
}
