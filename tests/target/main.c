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
 * @file main.c
 * @author notweerdmonk
 * @brief Target test program for UART module
 *
 * This firmware runs on the AVR microcontroller and responds to
 * test commands from the host test driver.
 *
 * Features:
 * - Echo test string back to host
 * - Receive and compare test strings
 * - Pattern matching callback tests
 * - VCD trace generation for simulation
 *
 * @note Compile with MATCH=1 to enable pattern matching tests
 */
#include <uart.h>
#include <string.h>
#include <util/delay.h>

#define set_input_pin(ddr, pin) (ddr) &= ~(1 << (pin))
#define set_output_pin(ddr, pin) (ddr) |= 1 << (pin)
#define set_pin(port, pin) (port) |= 1 << (pin)
#define clear_pin(port, pin) (port) &= ~(1 << (pin))
#define toggle_pin(port, pin) (port) ^= 1 << (pin)

#define RX_PORT PORTD
#define RX_PIN  PD0

#define TX_PORT PORTD
#define TX_PIN  PD1

#define TRIGGER_DDR  DDRB
#define TRIGGER_PORT PORTB
#define TRIGGER_PIN  PB4

/**
 * @brief Pattern match callback data structure
 */
#ifdef __UART_MATCH
struct match_cb_data {
  const char *str;
  int len;
};

/**
 * @brief Pattern match callback handler
 *
 * Called by UART module when a registered pattern is matched.
 * Sends the response string back to the host.
 *
 * @param data Pointer to match_cb_data structure
 */
void uart_match_cb(void *data) {
  uart_send(
      ((struct match_cb_data*)data)->str,
      ((struct match_cb_data*)data)->len
    );
}
#endif

const char pattern1[] = "UUUU"; /* Generates a square wave with 8N1 */
const char pattern2[] = "AAAAAAAAAAAAAAAA";
const char pattern3[] = "aaaaaaaaaaaaaaaa";
const char pattern4[] = { 
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
  'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
  'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0x8a, 0x00 }; 
const char pattern5[] = "\x01\x02\x03\xff\xfe\xfd\xfc";
const char pattern6[] = "\xc\xa\xf\xe\xb\xa\xb\xe";

const char okstr[] = "recv OK";
const char erstr[] = "recv ER";

/**
 * @brief Main test program entry point
 *
 * Initializes UART and runs test sequences:
 * - Sends test string to host
 * - Receives and verifies test string
 * - Tests partial reception
 * - Runs pattern matching tests (if enabled)
 *
 * @return 0 (never returns - infinite loop at end)
 */
int main() {
  const char *teststring = pattern4;

#ifdef __RUNTIME_CONFIG

  uart_setup(
      &(struct uart_config){
        .baud_rate = 0,
        .char_size = UART_CHAR_SIZE,
        .stop_bits = UART_STOP_BITS,
        .parity = UART_PARITY,
      }
    );

#else /* !__RUNTIME_CONFIG */

  uart_setup();

#endif /* __RUNTIME_CONFIG */
  
  sei();

#if defined __EMIT_TRIGGER
  /* Wait a while */
  asm volatile ("ldi r24, 255 \n\t"
                "loop1%=:     \n\t"
                "ldi r25, 50  \n\t"
                "loop2%=:     \n\t"
                "dec r25      \n\t"
                "brne loop2%= \n\t"
                "dec r24      \n\t"
                "brne loop1%= \n\t"
                :
                :
               );
  set_output_pin(TRIGGER_DDR, TRIGGER_PIN);
  set_pin(TRIGGER_PORT, TRIGGER_PIN);
#endif

#ifdef __UART_STDIO
  puts(teststring);
#else
  uart_sendln(teststring, strlen(teststring));
#endif

  /* Let the characters be send over UART by UDRE interrupt */
  /*
   * us per byte = UART_BAUD_RATE / (UART_CHAR_SIZE + UART_STOP_BITS)
   */
  _delay_us((double)UART_BAUD_RATE / 8 * 53);

  /*
   * Simulation with simavr cannot facilitate sending input to UART
   */
#if !defined __SIMULATION && !defined __DEMO
  size_t teststringlen = strlen(teststring);
  char buffer[teststringlen + 1];

  uart_peek(buffer, teststringlen);
  buffer[teststringlen] = '\0';

  if (!strncmp(teststring, buffer, teststringlen)) {
    uart_send(okstr, strlen(okstr));
  } else {
    uart_send(erstr, strlen(erstr));
  }

  /* Let the characters be send over UART by UDRE interrupt */
  _delay_us(((double)UART_BAUD_RATE / 8 * strlen(okstr)) + 1000);

  /* Actually consume data from UART buffer */
  uart_recv(buffer, teststringlen);

  size_t len = uart_recv(buffer, teststringlen / 2);
  buffer[len] = '\0';
 
  if (!strncmp(teststring, buffer, teststringlen / 2)) {
    uart_send(okstr, strlen(okstr));
  } else {
    uart_send(erstr, strlen(erstr));
  }

  /* Let the characters be send over UART by UDRE interrupt */
  _delay_us(10000);
#endif /* __SIMULATION */

#if defined __SIMULATION || defined __SIMTEST

  /* Sleep with interrupts disabled so that simavr can exit */
  cli();
  sleep_cpu();

#elif defined __UART_MATCH

  const char str1[] = "Match 1";
  const char str2[] = "Match 2";
  const char str3[] = "Match 3";
  const char str4[] = "Match 4";
  const char str5[] = "Match 5";
  const char str6[] = "Match 6";

  struct match_cb_data data1 = { str1, strlen(str1) };
  uart_register_match("***", uart_match_cb, (void*)&data1);

  struct match_cb_data data2 = { str2, strlen(str2) };
  uart_register_match("qwe", uart_match_cb, (void*)&data2);

  struct match_cb_data data3 = { str3, strlen(str3) };
  uart_register_match("qwerty", uart_match_cb, (void*)&data3);

  struct match_cb_data data4 = { str4, strlen(str4) };
  uart_register_match("123", uart_match_cb, (void*)&data4);

  struct match_cb_data data5 = { str5, strlen(str5) };
  uart_register_match("?", uart_match_cb, (void*)&data5);

  struct match_cb_data data6 = { str6, strlen(str6) };
  /* F6 */
  //uart_register_match("\x1B[17~", uart_match_cb, (void*)&data6);
  /* ANSI foreground red */
  uart_register_match(
      "\x1B[1;31mtext in red\x1b[1;0m",
      uart_match_cb,
      (void*)&data6
    );

  uart_deregister_match("***");
  uart_register_match("!@#$", uart_match_cb, (void*)&data1);

  uart_deregister_match("123");
  uart_register_match("1234", uart_match_cb, (void*)&data4);

  for (;;) {
    uart_check_match();
  }

#else

  for(;;uart_send_byte(uart_recv_byte()));

#endif

  return 0;
}
