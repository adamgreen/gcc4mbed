// Automatically generated configuration file.
// DO NOT EDIT, content will be overwritten.

#ifndef __MBED_CONFIG_DATA__
#define __MBED_CONFIG_DATA__

// Configuration parameters
#define MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE 9600 // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_FLUSH_AT_EXIT      1    // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_CONVERT_NEWLINES   0    // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_BAUD_RATE          9600 // set by library:platform
#define MBED_CONF_FILESYSTEM_PRESENT                1    // set by library:filesystem

#define MBED_CONF_LWIP_IPV4_ENABLED                 1    // set by library:lwip
#define MBED_CONF_LWIP_IPV6_ENABLED                 0    // set by library:lwip
#define MBED_CONF_LWIP_IP_VER_PREF                  4    // set by library:lwip
#define MBED_CONF_LWIP_ADDR_TIMEOUT                 5    // set by library:lwip
#define MBED_CONF_LWIP_ETHERNET_ENABLED             1
#define MBED_CONF_LWIP_DEBUG_ENABLED                0
#define NSAPI_PPP_AVAILABLE                         0
#define MBED_CONF_LWIP_USE_MBED_TRACE               0
#define MBED_CONF_LWIP_ENABLE_PPP_TRACE             0
#define MBED_CONF_LWIP_SOCKET_MAX                   4    // set by library:lwip
#define MBED_CONF_LWIP_TCP_ENABLED                  1
#define MBED_CONF_LWIP_TCP_SERVER_MAX               4    // set by library:lwip
#define MBED_CONF_LWIP_TCP_SOCKET_MAX               4    // set by library:lwip
#define MBED_CONF_LWIP_UDP_SOCKET_MAX               4    // set by library:lwip
#define MBED_CONF_LWIP_TCPIP_THREAD_STACKSIZE       1200
#define MBED_CONF_LWIP_DEFAULT_THREAD_STACKSIZE     512
#define MBED_CONF_LWIP_PPP_THREAD_STACKSIZE         768

#define MBED_CONF_DRIVERS_UART_SERIAL_TXBUF_SIZE    256
#define MBED_CONF_DRIVERS_UART_SERIAL_RXBUF_SIZE    256

// Macros
#define UNITY_INCLUDE_CONFIG_H                           // defined by library:utest

// Define MBED_RTOS_WORDS_STACK_SIZE to override the size of the main thread's stack.
// It is in units of words so the actual stack will be MBED_RTOS_WORDS_STACK_SIZE * 4.
// It defaults to 512 words (4k) for GCC_ARM.
//#define MBED_RTOS_WORDS_STACK_SIZE                  128

// Set MBED_RTOS_SINGLE_THREAD macro if you are using mbed-os with a single threaded application so that it can
// reduce the task and timer counts accordingly to save RAM.
//#define MBED_RTOS_SINGLE_THREAD                     1

#endif
