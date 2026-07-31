#ifndef __khrplatform_h_
#define __khrplatform_h_

/*
** Copyright (c) 2008-2018 The Khronos Group Inc.
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and/or associated documentation files (the "Materials"), to
** deal in the Materials without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Materials, and to permit persons to whom the Materials are
** furnished to do so, subject to the following conditions:
*/

#if defined(_WIN32) && !defined(__SCITECH_SNAP__)
#   define KHRONOS_APICALL
#   define KHRONOS_APIENTRY __stdcall
#else
#   define KHRONOS_APICALL
#   define KHRONOS_APIENTRY
#endif

#if defined(__KHRONOS_EXPORTS)
#   undef KHRONOS_APICALL
#   define KHRONOS_APICALL __declspec(dllexport)
#endif

#define KHRONOS_APIATTRIBUTES

#include <stdint.h>
#include <stddef.h>
typedef int32_t                 khronos_int32_t;
typedef uint32_t                khronos_uint32_t;
typedef int64_t                 khronos_int64_t;
typedef uint64_t                khronos_uint64_t;
typedef char                    khronos_int8_t;
typedef unsigned char           khronos_uint8_t;
typedef short                   khronos_int16_t;
typedef unsigned short          khronos_uint16_t;

typedef float                   khronos_float_t;

typedef intptr_t                khronos_intptr_t;
typedef uintptr_t               khronos_uintptr_t;
typedef ptrdiff_t               khronos_ssize_t;
typedef size_t                  khronos_usize_t;
typedef size_t                  khronos_size_t;

typedef enum {
    KHRONOS_FALSE = 0,
    KHRONOS_TRUE  = 1,
    KHRONOS_BOOLEAN_ENUM_FORCE_SIZE = 0x7FFFFFFF
} khronos_boolean_enum;

#endif
