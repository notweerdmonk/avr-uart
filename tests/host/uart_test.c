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
 * @file uart_test.c
 * @author notweerdmonk
 * @brief Host test driver for UART module
 *
 * This program communicates with an AVR microcontroller over a serial port
 * to run automated tests on the UART implementation.
 *
 * Features:
 * - Send and receive data over serial port
 * - Automated test execution with pass/fail reporting
 * - Support for pattern matching tests
 * - Configurable serial device
 *
 * @note Requires a connected AVR device running the target test firmware
 *
 * @code
 * # Build and run
 * make -C tests/host
 * ./uart_test -d /dev/ttyUSB0
 * @endcode
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>

#include <config.h>
#include <uart_config.h>
#include <logger.h>

#define STRINGIFY(s) #s
#define CONCAT(a, b) a ## b

#define SERDEV STRINGIFY(/dev/ttyUSB0)

#define N2BAUDRATE(baud) CONCAT(B, baud)

#define E2BAUDRATE(baud) \
  ({                     \
    speed_t speed;       \
    switch ((int)baud) { \
      case 50:           \
        speed = B50;     \
        break;           \
      case 75:           \
        speed = B75;     \
        break;           \
      case 110:          \
        speed = B110;    \
        break;           \
      case 134:          \
        speed = B134;    \
        break;           \
      case 150:          \
        speed = B150;    \
        break;           \
      case 200:          \
        speed = B200;    \
        break;           \
      case 300:          \
        speed = B300;    \
        break;           \
      case 600:          \
        speed = B600;    \
        break;           \
      case 1200:         \
        speed = B1200;   \
        break;           \
      case 1800:         \
        speed = B1800;   \
        break;           \
      case 2400:         \
        speed = B2400;   \
        break;           \
      case 4800:         \
        speed = B4800;   \
        break;           \
      case 9600:         \
        speed = B9600;   \
        break;           \
      case 19200:        \
        speed = B19200;  \
        break;           \
      case 38400:        \
        speed = B38400;  \
        break;           \
      case 0:            \
      default:           \
        speed = B0;      \
    }                    \
    speed;               \
  })

#define BUFLEN 256

static struct logger log;

static const char okstr[] = "recv OK";

static const char teststring[] = { 
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
  'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
  'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
  'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0x8a }; 

/**
 * @brief Logger write function
 *
 * Writes log messages to stdout or stderr based on log level.
 *
 * @param msg  Log message string
 * @param lvl  Log level (enum logger_level)
 */
static
void log_writer(const char *msg, enum logger_level lvl) {
  if (!msg) {
    return;
  }
  FILE *fptr = lvl > LOGGER_INFO ? stderr : stdout;
  for (; msg && *msg; ++msg) {
    if (isprint(*msg) || iscntrl(*msg)) {
      putc(*msg, fptr);
    } else {
      fprintf(fptr, "%#x", *msg);
    }
  }
  putc('\n', fptr);
}

static int __stop = 0;

/**
 * @brief Signal alarm handler
 *
 * Signal handler for SIGALRM used to implement timeouts
 * during serial operations.
 *
 * @param sig Signal number (unused)
 */
void sigalarm_handler(int sig __attribute__ ((unused))) {
  __stop = 1;
}

/**
 * @brief Setup alarm signal handler
 *
 * Configures the SIGALRM signal handler for timeout functionality.
 *
 * @param p_old_sa Pointer to store old signal action
 * @return 0 on success, -1 on failure
 */
int setup_alarm(struct sigaction* p_old_sa) {
  if (!p_old_sa) {
    return -1;
  }

  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = sigalarm_handler;
  sa.sa_flags = 0;
  if (sigaction(SIGALRM, &sa, p_old_sa) == -1) {
    LOG(
        &log,
        LOGGER_ERROR,
        "Sigaction: setting handler for SIGALARM failed: %s",
        strerror(errno)
      );
    return -1;
  }

  return 0;
}

/**
 * @brief Cleanup alarm signal handler
 *
 * Restores the previous SIGALRM signal handler.
 *
 * @param p_old_sa Pointer to old signal action to restore
 * @return 0 on success, -1 on failure
 */
int cleanup_alarm(struct sigaction* p_old_sa) {
  if (!p_old_sa) {
    return -1;
  }

  if (sigaction(SIGALRM, p_old_sa, 0) == -1) {
    LOG(
        &log,
        LOGGER_ERROR,
        "Sigaction: restoring handler for SIGALARM failed: %s",
        strerror(errno)
      );
    return -1;
  }
  return 0;
}

/**
 * @brief Convert string to hex representation
 *
 * Converts a string to its hexadecimal representation
 * for logging purposes.
 *
 * @param str Input string
 * @return Newly allocated hex string (caller must free), or NULL on error
 */
/* Returns malloced string */
char* strtohex(const char *str) {
  if (!str) {
    return NULL;
  }

  int len = strlen(str);
  char *hexstr = (char*)malloc((2 + len * 2 + 2) * sizeof(char));

  sprintf(hexstr, "0x");
  for (int i = 0, j = 0; i < len; ++i, j += 2) {
    sprintf(hexstr + 2 + j, "%02x", str[i] & 0xff);
  }

  return hexstr;
}

/**
 * @brief Convert parity type to string
 *
 * Converts UART parity constant to human-readable string.
 *
 * @param parity Parity constant (UART_PARITY_NONE, etc.)
 * @return Static string describing parity mode
 */
char* parity_type_to_str(int parity) {

  if (parity == UART_PARITY_NONE) {
    return "None";

  } else if (parity == UART_PARITY_EVEN) {
    return "Even";

  } else if (parity == UART_PARITY_ODD) {
    return "Odd";

  } else if (parity == UART_PARITY_MARK) {
    return "Mark";

  } else if (parity == UART_PARITY_SPACE) {
    return "Space";
  }

  return "None";
}

/**
 * @brief Parse command line arguments
 *
 * Parses command line options for the test program.
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @param k    Pointer to store keep-going flag
 * @param buffer Buffer to store device path
 * @param len   Buffer length
 * @return 0 on success, -1 on error (including -h flag)
 */
int parse_cmd_args(int argc, char *argv[], int *k, char *buffer, size_t len) {

  int c;

  (void)(k && (*k = 0));

  while ((c = getopt(argc, argv, ":hd:k")) != -1) {
    switch (c) {
      case 'd':
        strncpy(buffer, optarg, len);
        break;

      case 'k':
        (void)(k && (*k = 1));
        break;

      case 'h':
        printf("Usage: %s [OPTION] ...\n", argv[0]);
        printf("  -h             Display this message\n");
        printf("  -d [device]    Open device as serial port\n");
        printf("  -k             Keep going in case of failure\n");
        return -1;

      case '?':
        fprintf(stderr, "Invalid option: %c\n", optopt);
        return -1;

      case ':':
        fprintf(stderr, "Invalid option: %c requires an argument\n", optopt);
        return -1;

      default:
        return -1;
    }
  }
  return 0;
}

/**
 * @brief Open serial device
 *
 * Opens a serial port device for communication.
 *
 * @param device Path to serial device (e.g., /dev/ttyUSB0)
 * @return File descriptor on success, -1 on failure
 */
int open_serial_device(const char *device) {
  if (!device || !*device) {
    return -1;
  }

  LOG(&log, LOGGER_INFO, "Opening device: %s", device);

  int serdev = open(device, O_RDWR|O_NOCTTY);
  if (serdev == -1) {
    LOG(&log, LOGGER_ERROR, "Could not open device %s: %s", device,
        strerror(errno));
    return -1;
  }

  return serdev;
}

/* TODO: pass uart_config as argument */
#ifdef UART_RUNTIME_CONFIG

#else /* !UART_RUNTIME_CONFIG */

/**
 * @brief Configure serial port settings
 *
 * Sets up the serial port with matching parameters for the AVR UART.
 *
 * @param serdev File descriptor of open serial port
 * @param param  Configuration parameter (unused in compile-time mode)
 * @return 0 on success, -1 on failure
 */
int setup_serial_device(int serdev, void *param) {
#ifdef UART_RUNTIME_CONFIG

  if (!config) {
    return -1;
  }

#else /* !UART_RUNTIME_CONFIG */

  (void)param;

#endif /* UART_RUNTIME_CONFIG */

  int baud_rate;
  char char_size;
  char stop_bits;
  char parity;

#ifdef UART_RUNTIME_CONFIG

  struct uart_config *cfgptr = param;
  baud_rate = cfgptr->baud_rate;
  char_size = cfgptr->char_size;
  stop_bits = cfgptr->stop_bits;
  parity = cfgptr->parity;

#else /* !UART_RUNTIME_CONFIG */

  baud_rate = UART_BAUD_DEFAULT;
  char_size = UART_CHAR_SIZE;
  stop_bits = UART_STOP_BITS;
  parity = UART_PARITY;

#endif /* UART_RUNTIME_CONFIG */

  struct termios settings;
  tcgetattr(serdev, &settings);

  cfsetspeed(&settings, E2BAUDRATE(baud_rate));

  settings.c_cflag &= ~CSIZE;

  if (char_size == 8) {
    settings.c_cflag |= CS8;
  }
  else if (char_size == 7) {
    settings.c_cflag |= CS7;
  }

  if (stop_bits < 2) {
    settings.c_cflag &= ~CSTOPB;
  }

  if (parity > UART_PARITY_NONE) {
    settings.c_cflag |= PARENB;

    if (parity == UART_PARITY_ODD) {
      settings.c_cflag |= PARODD;
    }

  } else {
    settings.c_cflag &= ~PARENB;
  }

  settings.c_cflag &= ~CRTSCTS;
  settings.c_cflag |= CREAD | CLOCAL;

  settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);
  
  settings.c_iflag &= ~(IXON | IXOFF | IXANY);

  settings.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

  settings.c_oflag &= ~(OPOST | ONLCR);

  settings.c_cc[VMIN] = 0;
  settings.c_cc[VTIME] = 0;

  if (tcsetattr(serdev, TCSANOW, &settings) == -1) {
    LOG(&log, LOGGER_ERROR, "Failed to set serial port attributes!");
    close(serdev);
    return -1;
  }
  else {
    LOG(&log,
        LOGGER_INFO,
        "Serial port settings: %d Baud rate, %d-bits Character size, %d "
        "Stop bit, %s parity",
        UART_BAUD_DEFAULT,
        UART_CHAR_SIZE,
        UART_STOP_BITS,
        parity_type_to_str(UART_PARITY)
      );
  }

  return 0;;
}

#endif /* UART_RUNTIME_CONFIG */

/**
 * @brief Send data over serial port
 *
 * Sends data with timeout support using select() for non-blocking I/O.
 *
 * @param serdev  File descriptor of serial port
 * @param data    Data buffer to send
 * @param len     Number of bytes to send
 * @param timeout Timeout in seconds
 * @return Number of bytes sent, or -1 on error
 */
size_t serial_send(int serdev, const char *data, size_t len,
    unsigned int timeout) {

  size_t remaining = len;
  size_t next_byte = 0;
  ssize_t bytes_count;

  __stop = 0;

  LOG(&log, LOGGER_DEBUG, "Sending data:\n%.*s", (int)len, data);

  alarm(timeout);

  do {
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(serdev, &wfds);

    struct timeval select_timeout;
    select_timeout.tv_sec = 0;
    select_timeout.tv_usec = 1000;

    if (select(serdev + 1, NULL, &wfds, NULL, &select_timeout) > 0) {

      bytes_count = write(serdev, data + next_byte, remaining);
      if (bytes_count >= 0) {
        next_byte += bytes_count;
        remaining -= bytes_count;
        continue;
      }

      LOG(&log, LOGGER_ERROR, "Write to serial port failed: %s",
          strerror(errno));

      if (errno == EINTR) {
        __stop = 0;
        alarm(timeout);
        continue;
      }

      alarm(0);
      return -1;
    }

  } while ( !__stop && (next_byte < len) );

  alarm(0);

  LOG(&log, LOGGER_INFO, "Sent %ld bytes", next_byte);

  return next_byte;
}

/**
 * @brief Receive data over serial port
 *
 * Receives data with timeout support using select() for non-blocking I/O.
 *
 * @param serdev  File descriptor of serial port
 * @param buffer  Buffer to store received data
 * @param len     Maximum bytes to receive
 * @param timeout Timeout in seconds
 * @return Number of bytes received, or -1 on error
 */
size_t serial_recv(int serdev, char *buffer, size_t len,
    unsigned int timeout) {

  size_t remaining = len;
  size_t next_byte = 0;
  ssize_t bytes_count;

  __stop = 0;

  alarm(timeout);

  do {
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(serdev, &wfds);

    struct timeval select_timeout;
    select_timeout.tv_sec = 0;
    select_timeout.tv_usec = 1000;

    if (select(serdev + 1, NULL, &wfds, NULL, &select_timeout) > 0) {

      bytes_count = read(serdev, buffer + next_byte, remaining);
      if (bytes_count >= 0) {
        next_byte += bytes_count;
        remaining -= bytes_count;
        continue;
      }

      LOG(&log, LOGGER_ERROR, "Read from serial port failed: %s",
          strerror(errno));

      if (errno == EINTR) {
        __stop = 0;
        alarm(timeout);
        continue;
      }

      alarm(0);
      return -1;
    }

  } while ( !__stop && (next_byte < len) );

  alarm(0);

  buffer[next_byte] = '\0';
  LOG(&log, LOGGER_DEBUG, "Received %ld bytes:\n%s", next_byte, buffer);

  return next_byte;
}

/**
 * @brief Run a test and report results
 *
 * Macro that executes a test function and logs pass/fail status.
 *
 * @param name   Test name string
 * @param result Variable to store test result
 * @param func   Test function to call
 * @param nargs  Number of arguments to pass
 * @param ...    Arguments to pass to test function
 */
#define RUN_TEST(name, result, func, nargs, ...) \
  do {                                                  \
    LOG(&log, LOGGER_INFO, "Running test: %s", (name)); \
    if (!func((nargs), __VA_ARGS__)) {                  \
      LOG(&log, LOGGER_INFO, "%s passed", (name));      \
      result = 0;                                       \
    } else {                                            \
      LOG(&log, LOGGER_ERROR, "%s failed", (name));     \
      result = 1;                                       \
    }                                                   \
  } while (0)

/**
 * @brief Unpack variadic arguments for test functions
 *
 * Extracts serdev and buffer from variadic arguments.
 * Asserts that exactly 2 arguments are provided.
 *
 * @param list Variable to store va_list
 * @param arg  Expected argument count (should be 2)
 */
#define UNPACK_ARGS(list, arg) \
  assert(arg == 2);                     \
  va_list (list);                       \
  va_start((list), (arg));              \
  int serdev = va_arg((list), int);     \
  char *buffer = va_arg((list), char*); \
  va_end((list));


/**
 * @brief Test UART send functionality
 *
 * Tests that the AVR can send data correctly over UART.
 * GIVEN AVR microcontroller WHEN uC sends a string over UART
 * THEN received string should match with stored string
 *
 * @param nargs Number of arguments
 * @return 0 on success (test passed), -1 on failure
 */
int send_test(int nargs, ...) {

  /*
   * GIVEN AVR microcontroller
   * WHEN uC sends a string over UART
   * THEN received string should match with stored string
   */

  UNPACK_ARGS(args, nargs);

  serial_recv(serdev, buffer, sizeof(teststring) + 2, 3);
  
  /* uart_sendln sends CRLF after the end of a string */
  if (strncmp(buffer, teststring, sizeof(teststring)) ||
      strncmp(buffer + sizeof(teststring), "\r\n", 2)) {
    return -1;
  }

  return 0;
}

/**
 * @brief Test UART receive functionality
 *
 * Tests that the AVR can receive data correctly over UART.
 * GIVEN AVR microcontroller WHEN uC waits to receive a string over UART,
 * compares it with stored string and sends "recv OK" on match
 * THEN "recv OK" should be received
 *
 * @param nargs Number of arguments
 * @return 0 on success (test passed), -1 on failure
 */
int recv_test(int nargs, ...) {

  /*
   * GIVEN AVR microcontroller
   * WHEN uC waits to recieve a string over UART, compares it with stored
   * string and sends "recv OK" on match and "recv ER" otherwise
   * THEN "recv OK" should be recieved
   */

  UNPACK_ARGS(args, nargs);

  serial_send(serdev, teststring, sizeof(teststring), 3);

  serial_recv(serdev, buffer, sizeof(okstr) - 1, 1);

  if (strncmp(buffer, okstr, sizeof(okstr) - 1)) {
    return -1;
  }

  return 0;
}

/**
 * @brief Test partial UART buffer read functionality
 *
 * Tests that the UART library correctly handles reading the receive
 * buffer partially (i.e., reading fewer bytes than available).
 * GIVEN AVR microcontroller WHEN host sends a string and uC reads only
 * half the bytes THEN remaining bytes should still be in buffer
 *
 * @param nargs Number of arguments
 * @return 0 on success (test passed), -1 on failure
 */
int partial_recv_test(int nargs, ...) {

  /*
   * GIVEN AVR microcontroller
   * WHEN uC waits to recieve a string over UART, compares it with stored
   * string and sends "recv OK" on match and "recv ER" otherwise
   * THEN "recv OK" should be recieved
   */

  UNPACK_ARGS(args, nargs);

  serial_send(serdev, teststring, sizeof(teststring), 3);

  serial_recv(serdev, buffer, sizeof(okstr) - 1, 1);

  if (strncmp(buffer, okstr, sizeof(okstr) - 1)) {
    return -1;
  }

  return 0;
}

/**
 * @brief Pattern match data structure
 */
#ifdef __UART_MATCH
struct _match_data {
  const char *pattern;
  const char *response;
};

struct _match_data match_data[] = {
    { "***", "" },
    { "qwe", "Match 2" },
    { "qwerty", "Match 2" },
    { "123", "" },
    { "?", "Match 5" },
    //{ "\x1B[17~", "Match 6" }, // F6
    { "\x1B[1;31mtext in red\x1b[1;0m", "Match 6" }, // ANSI foreground red
    { "!@#$", "Match 1" },
    { "1234", "Match 4" },
  };

/**
 * @brief Test pattern matching functionality
 *
 * Tests that the AVR correctly triggers callbacks when
 * registered patterns are matched in incoming data.
 *
 * @param nargs Number of arguments (should be 4)
 * @return 0 on success (test passed), -1 on failure
 */
int match_test(int nargs, ...) {

  assert(nargs == 4);
  va_list(args);
  va_start(args, nargs);

  int serdev = va_arg(args, int);
  char *buffer = va_arg(args, char*);
  const char* pattern = va_arg(args, const char*);
  const char* response = va_arg(args, const char*);
  int testnum = va_arg(args, int);

  va_end(args);
  
  if (testnum == 6) {
    char *hexstr = strtohex(pattern);
    LOG(&log, LOGGER_INFO, "Sending match pattern: %s", hexstr);
    free(hexstr);
  } else {
    LOG(&log, LOGGER_INFO, "Sending match pattern: %s", pattern);
  }
  serial_send(serdev, pattern, strlen(pattern), 1);

  LOG(&log, LOGGER_INFO, "Waiting for match response...");
  serial_recv(serdev, buffer, BUFLEN - 1, 1);

  if (strncmp(buffer, response, strlen(response))) {
    return -1;
  }

  return 0;
}
#endif

/**
 * @brief Main test driver entry point
 *
 * Main function that orchestrates all UART tests:
 * - send_test: Verifies AVR can transmit
 * - recv_test: Verifies AVR can receive and echo
 * - partial_recv_test: Verifies partial data handling
 * - match_test: Verifies pattern matching (if enabled)
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success, non-zero on failure
 */
int main(int argc, char *argv[]) {

  int serdev;
  int keep_going, result;
  char buffer[BUFLEN];
  struct sigaction old_sa;

  logger_set_level(&log, LOGGER_ALL);
  logger_set_function(&log, log_writer);

  /* Copy default serial device path to buffer */
  strncpy(buffer, SERDEV, BUFLEN);

  if (parse_cmd_args(argc, argv, &keep_going, buffer, BUFLEN) == -1) {
    return -1;
  }
 
  if ( (serdev = open_serial_device(buffer)) == -1 ) {
    return -1;
  }

#ifdef UART_RUNTIME_CONFIG

  setup_serial_device(
      serdev,
      &(struct uart_config){
        .baud_rate = 0,
        .char_size = UART_CHAR_SIZE,
        .stop_bits = UART_STOP_BITS,
        .parity = UART_PARITY,
      }
    );

#else /* !UART_RUNTIME_CONFIG */

  setup_serial_device(serdev, NULL);

#endif /* UART_RUNTIME_CONFIG */


#ifndef __SIMTEST
  /* Discard any data received over serial port */
  tcflush(serdev, TCIFLUSH);
#endif

  if (setup_alarm(&old_sa) == -1) {
    return -1;
  }

  RUN_TEST(
      "send test",
      result,
      send_test,
      2,
      serdev,
      buffer
    );

  if (!keep_going && result) {
    return -1;
  }

  RUN_TEST(
      "recv test",
      result,
      recv_test,
      2,
      serdev,
      buffer
    );

  if (!keep_going && result) {
    return -1;
  }

  RUN_TEST(
      "partial recv test",
      result,
      partial_recv_test,
      2,
      serdev,
      buffer
    );

  if (!keep_going && result) {
    return -1;
  }

#ifdef __UART_MATCH
  int num_passed = 0;
  for (long unsigned int i = 0;
      i < sizeof(match_data) / sizeof(struct _match_data) ; i++) {

    char testname[32];
    snprintf(testname, 32, "Match test %lu", i + 1);

    RUN_TEST(
        testname,
        result,
        match_test,
        4,
        serdev,
        buffer,
        match_data[i].pattern,
        match_data[i].response,
        i + 1
      );

    if (!keep_going && result) {
      return -1;
    } else {
      ++num_passed;
    }
  }

  LOG(
      &log,
      LOGGER_INFO,
      "Match test: %d out of %d tests passed",
      num_passed,
      sizeof(match_data) / sizeof(struct _match_data)
    );
#endif

  cleanup_alarm(&old_sa);

  close(serdev);

  return 0;
}
