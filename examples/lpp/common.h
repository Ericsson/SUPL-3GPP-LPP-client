#pragma once

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef UNUSED
#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define UNUSED [[maybe_unused]]
#else
#define UNUSED
#endif
#else
#define UNUSED
#endif
#endif

#define ALLOC_ZERO(T) ((T*)calloc(1, sizeof(T)))
#define ARRAY_COUNT(X) sizeof((X)) / sizeof(*(X))

#define KM_2_M (1000.0)

// SPT_X = 2^(-X)
#define SPT_5 (0.03125)
#define SPT_11 (0.00048828125)
#define SPT_19 (1.9073486328125e-06)
#define SPT_20 (9.5367431640625e-07)
#define SPT_29 (1.86264514923096e-09)
#define SPT_30 (9.31322574615479e-10)
#define SPT_31 (4.65661287307739e-10)
#define SPT_32 (2.3283064365387e-10)
#define SPT_33 (1.16415321826935e-10)
#define SPT_40 (9.09494701772928e-13)
#define SPT_43 (1.13686837721616e-13)
#define SPT_55 (2.77555756156289e-17)
#define SPT_58 (3.46944695195361e-18)

// SPT_PX = 2^(X)
#define SPT_P4 (16)
