#pragma once

// Shared transport type documentation used by --stream, --input, --output help text.

#define TRANSPORT_HELP_SERIAL                                                                      \
    "  serial:\n"                                                                                  \
    "    device=<device>\n"                                                                        \
    "    baudrate=<baudrate>\n"                                                                    \
    "    databits=<5|6|7|8>\n"                                                                     \
    "    stopbits=<1|2>\n"                                                                         \
    "    parity=<none|odd|even>\n"

#define TRANSPORT_HELP_TCP_CLIENT                                                                  \
    "  tcp-client:\n"                                                                              \
    "    host=<host>\n"                                                                            \
    "    port=<port>\n"                                                                            \
    "    reconnect=<bool> (default=true)\n"                                                        \
    "    path=<path>\n"

#define TRANSPORT_HELP_TCP_SERVER                                                                  \
    "  tcp-server:\n"                                                                              \
    "    listen=<addr> (default=0.0.0.0)\n"                                                        \
    "    port=<port>\n"                                                                            \
    "    path=<path>\n"

#define TRANSPORT_HELP_UDP_CLIENT                                                                  \
    "  udp-client:\n"                                                                              \
    "    host=<host>\n"                                                                            \
    "    port=<port>\n"                                                                            \
    "    path=<path>\n"

#define TRANSPORT_HELP_UDP_SERVER                                                                  \
    "  udp-server:\n"                                                                              \
    "    listen=<addr> (default=0.0.0.0)\n"                                                        \
    "    port=<port>\n"                                                                            \
    "    path=<path>\n"

#define TRANSPORT_HELP_FILE_INPUT                                                                  \
    "  file:\n"                                                                                    \
    "    path=<path>\n"                                                                            \
    "    bps=<bytes_per_sec>\n"

#define TRANSPORT_HELP_FILE_OUTPUT                                                                 \
    "  file:\n"                                                                                    \
    "    path=<path>\n"                                                                            \
    "    append=<bool>\n"                                                                          \
    "    tbin=<bool>\n"
