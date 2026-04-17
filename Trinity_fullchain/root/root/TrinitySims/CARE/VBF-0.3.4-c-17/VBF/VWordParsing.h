/*
 * VWordParsing.h -- code stolen from tsf_internal.h (which is my code anyway)
 * by Filip Pizlo, 2005
 */

#ifndef V_WORD_PARSING_H
#define V_WORD_PARSING_H

#define CAN_DO_MISALIGNED_LDST
#define CAN_DO_FLOAT_MISALIGNED_LDST

#include <inttypes.h>

/* get ntohl, htonl, ntohs, htons */
#include <arpa/inet.h>

/* define stupid trivial stuff */
#define htonc(x) (x)
#define ntohc(x) (x)

#if (BYTE_ORDER==LITTLE_ENDIAN)

/* define long long equivalent of the arpa/inet functions */
#define htonll(x) (x)

/* define misaligned copy equivalents of all of the arpa/inet functions */
static inline void copy_htonc(uint8_t *dst,
                              uint8_t *src) {
    *dst=*src;
}

static inline void copy_htons(uint8_t *dst,
                              uint8_t *src) {
#ifdef CAN_DO_MISALIGNED_LDST
    *((uint16_t*)dst)=*((uint16_t*)src);
#else
    dst[0]=src[0];
    dst[1]=src[1];
#endif
}

static inline void copy_htonl(uint8_t *dst,
                              uint8_t *src) {
#ifdef CAN_DO_MISALIGNED_LDST
    uint32_t tmp=*((uint32_t*)src);
    *((uint32_t*)dst)=tmp;
#else
    dst[0]=src[0];
    dst[1]=src[1];
    dst[2]=src[2];
    dst[3]=src[3];
#endif
}

static inline void copy_htonll(uint8_t *dst,
                               uint8_t *src) {
#ifdef CAN_DO_MISALIGNED_LDST
    *((uint64_t*)dst)=*((uint64_t*)src);
#else
    dst[0]=src[0];
    dst[1]=src[1];
    dst[2]=src[2];
    dst[3]=src[3];
    dst[4]=src[4];
    dst[5]=src[5];
    dst[6]=src[6];
    dst[7]=src[7];
#endif
}

#define copy_ntohc(dst,src) copy_htonc(dst,src)
#define copy_ntohs(dst,src) copy_htons(dst,src)
#define copy_ntohl(dst,src) copy_htonl(dst,src)
#define copy_ntohll(dst,src) copy_htonll(dst,src)

#else

#define NEED_INT_CONVERSION

/* define long long equivalent of the arpa/inet functions */
static inline uint64_t htonll(uint64_t x) {
    /* probably not very efficient, but oh well. */
    union {
        uint64_t x;
        uint8_t c[8];
    } u;
    u.x=x;
    u.c[0]^=u.c[7]^=u.c[0]^=u.c[7];
    u.c[1]^=u.c[6]^=u.c[1]^=u.c[6];
    u.c[2]^=u.c[5]^=u.c[2]^=u.c[5];
    u.c[3]^=u.c[4]^=u.c[3]^=u.c[4];
    return u.x;
}

/* define misaligned copy equivalents of all of the arpa/inet functsions */
static inline void copy_htonc(uint8_t *dst,
                              uint8_t *src) {
    *dst=*src;
}

static inline void copy_htons(uint8_t *dst,
                              uint8_t *src) {
    dst[0]=src[1];
    dst[1]=src[0];
}

static inline void copy_htonl(uint8_t *dst,
                              uint8_t *src) {
    dst[0]=src[3];
    dst[1]=src[2];
    dst[2]=src[1];
    dst[3]=src[0];
}

static inline void copy_htonll(uint8_t *dst,
                               uint8_t *src) {
    dst[0]=src[7];
    dst[1]=src[6];
    dst[2]=src[5];
    dst[3]=src[4];
    dst[4]=src[3];
    dst[5]=src[2];
    dst[6]=src[1];
    dst[7]=src[0];
}

#define copy_ntohc(dst,src) copy_htonc(dst,src)
#define copy_ntohs(dst,src) copy_htons(dst,src)
#define copy_ntohl(dst,src) copy_htonl(dst,src)
#define copy_ntohll(dst,src) copy_htonll(dst,src)

#endif

#define ntohll(x) (htonll(x))

/* floating point stuff */

#ifdef NEED_INT_CONVERSION
#define NEED_FLOAT_CONVERSION
#endif

static inline float htonf(float x) {
    uint32_t ret=htonl(*((long*)&x));
    return *((float*)&ret);
}

static inline double htond(double x) {
    uint64_t ret=htonll(*((uint64_t*)&x));
    return *((double*)&ret);
}

#define ntohf(x) (htonf(x))
#define ntohd(x) (htond(x))

#if !defined(NEED_INT_CONVERSION) &&\
    defined(CAN_DO_MISALIGNED_LDST) &&\
    !defined(CAN_DO_FLOAT_MISALIGNED_LDST)
static inline void copy_htonf(uint8_t *dst,
                              uint8_t *src) {
    dst[0]=src[0];
    dst[1]=src[1];
    dst[2]=src[2];
    dst[3]=src[3];
}

static inline void copy_htond(uint8_t *dst,
                              uint8_t *src) {
    dst[0]=src[0];
    dst[1]=src[1];
    dst[2]=src[2];
    dst[3]=src[3];
    dst[4]=src[4];
    dst[5]=src[5];
    dst[6]=src[6];
    dst[7]=src[7];
}
#else
#define copy_htonf(dst,src) copy_htonl(dst,src)
#define copy_htond(dst,src) copy_htonll(dst,src)
#endif

#define copy_ntohf(dst,src) copy_htonf(dst,src)
#define copy_ntohd(dst,src) copy_htond(dst,src)

#define copy_htonc_incdst(dst,src) do {     \
    copy_htonc(dst,src);                    \
    (dst)=(uint8_t*)(((uint8_t*)(dst))+1);     \
} while (0)

#define copy_ntohc_incdst(dst,src) do {     \
    copy_ntohc(dst,src);                    \
    (dst)=(uint8_t*)(((uint8_t*)(dst))+1);     \
} while (0)

#define copy_htons_incdst(dst,src) do {     \
    copy_htons(dst,src);                    \
    (dst)=(uint8_t*)(((uint16_t*)(dst))+1);    \
} while (0)

#define copy_ntohs_incdst(dst,src) do {     \
    copy_ntohs(dst,src);                    \
    (dst)=(uint8_t*)(((uint16_t*)(dst))+1);    \
} while (0)

#define copy_htonl_incdst(dst,src) do {     \
    copy_htonl(dst,src);                    \
    (dst)=(uint8_t*)(((uint32_t*)(dst))+1);    \
} while (0)

#define copy_ntohl_incdst(dst,src) do {     \
    copy_ntohl(dst,src);                    \
    (dst)=(uint8_t*)(((uint32_t*)(dst))+1);    \
} while (0)

#define copy_htonll_incdst(dst,src) do {    \
    copy_htonll(dst,src);                   \
    (dst)=(uint8_t*)(((uint64_t*)(dst))+1);    \
} while (0)

#define copy_ntohll_incdst(dst,src) do {    \
    copy_ntohll(dst,src);                   \
    (dst)=(uint8_t*)(((uint64_t*)(dst))+1);    \
} while (0)

#define copy_htonf_incdst(dst,src) do {     \
    copy_htonf(dst,src);                    \
    (dst)=(uint8_t*)(((float*)(dst))+1);       \
} while (0)

#define copy_ntohf_incdst(dst,src) do {     \
    copy_ntohf(dst,src);                    \
    (dst)=(uint8_t*)(((float*)(dst))+1);       \
} while (0)

#define copy_htond_incdst(dst,src) do {     \
    copy_htond(dst,src);                    \
    (dst)=(uint8_t*)(((double*)(dst))+1);      \
} while (0)

#define copy_ntohd_incdst(dst,src) do {     \
    copy_ntohd(dst,src);                    \
    (dst)=(uint8_t*)(((double*)(dst))+1);      \
} while (0)

/* source-incrementing misaligned copy macros.  these do
 * no bounds checking. */

#define copy_htonc_incsrc(dst,src) do {     \
    copy_htonc(dst,src);                    \
    (src)=(uint8_t*)(((uint8_t*)(src))+1);     \
} while (0)

#define copy_ntohc_incsrc(dst,src) do {     \
    copy_ntohc(dst,src);                    \
    (src)=(uint8_t*)(((uint8_t*)(src))+1);     \
} while (0)

#define copy_htons_incsrc(dst,src) do {     \
    copy_htons(dst,src);                    \
    (src)=(uint8_t*)(((uint16_t*)(src))+1);    \
} while (0)

#define copy_ntohs_incsrc(dst,src) do {     \
    copy_ntohs(dst,src);                    \
    (src)=(uint8_t*)(((uint16_t*)(src))+1);    \
} while (0)

#define copy_htonl_incsrc(dst,src) do {     \
    copy_htonl(dst,src);                    \
    (src)=(uint8_t*)(((uint32_t*)(src))+1);    \
} while (0)

#define copy_ntohl_incsrc(dst,src) do {     \
    copy_ntohl(dst,src);                    \
    (src)=(uint8_t*)(((uint32_t*)(src))+1);    \
} while (0)

#define copy_htonll_incsrc(dst,src) do {    \
    copy_htonll(dst,src);                   \
    (src)=(uint8_t*)(((uint64_t*)(src))+1);    \
} while (0)

#define copy_ntohll_incsrc(dst,src) do {    \
    copy_ntohll(dst,src);                   \
    (src)=(uint8_t*)(((uint64_t*)(src))+1);    \
} while (0)

#define copy_htonf_incsrc(dst,src) do {     \
    copy_htonf(dst,src);                    \
    (src)=(uint8_t*)(((float*)(src))+1);       \
} while (0)

#define copy_ntohf_incsrc(dst,src) do {     \
    copy_ntohf(dst,src);                    \
    (src)=(uint8_t*)(((float*)(src))+1);       \
} while (0)

#define copy_htond_incsrc(dst,src) do {     \
    copy_htond(dst,src);                    \
    (src)=(uint8_t*)(((double*)(src))+1);      \
} while (0)

#define copy_ntohd_incsrc(dst,src) do {     \
    copy_ntohd(dst,src);                    \
    (src)=(uint8_t*)(((double*)(src))+1);      \
} while (0)

/* source-incrementing misaligned copy macros.  these do
 * bounds checks.  when a bounds check fails, they jump to
 * the label you give them. */

#define copy_htonc_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<1) {        \
        goto label;                                     \
    }                                                   \
    copy_htonc(dst,src);                                \
    (src)=(uint8_t*)(((uint8_t*)(src))+1);                 \
} while (0)

#define copy_ntohc_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<1) {        \
        goto label;                                     \
    }                                                   \
    copy_ntohc(dst,src);                                \
    (src)=(uint8_t*)(((uint8_t*)(src))+1);                 \
} while (0)

#define copy_htons_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<2) {        \
        goto label;                                     \
    }                                                   \
    copy_htons(dst,src);                                \
    (src)=(uint8_t*)(((uint16_t*)(src))+1);                \
} while (0)

#define copy_ntohs_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<2) {        \
        goto label;                                     \
    }                                                   \
    copy_ntohs(dst,src);                                \
    (src)=(uint8_t*)(((uint16_t*)(src))+1);                \
} while (0)

#define copy_htonl_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<4) {        \
        goto label;                                     \
    }                                                   \
    copy_htonl(dst,src);                                \
    (src)=(uint8_t*)(((uint32_t*)(src))+1);                \
} while (0)

#define copy_ntohl_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<4) {        \
        goto label;                                     \
    }                                                   \
    copy_ntohl(dst,src);                                \
    (src)=(uint8_t*)(((uint32_t*)(src))+1);                \
} while (0)

#define copy_htonll_incsrc_bc(dst,src,end,label) do {   \
    if (((uint8_t*)(end))-((uint8_t*)(src))<8) {        \
        goto label;                                     \
    }                                                   \
    copy_htonll(dst,src);                               \
    (src)=(uint8_t*)(((uint64_t*)(src))+1);                \
} while (0)

#define copy_ntohll_incsrc_bc(dst,src,end,label) do {   \
    if (((uint8_t*)(end))-((uint8_t*)(src))<8) {        \
        goto label;                                     \
    }                                                   \
    copy_ntohll(dst,src);                               \
    (src)=(uint8_t*)(((uint64_t*)(src))+1);                \
} while (0)

#define copy_htonf_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<4) {        \
        goto label;                                     \
    }                                                   \
    copy_htonf(dst,src);                                \
    (src)=(uint8_t*)(((float*)(src))+1);                   \
} while (0)

#define copy_ntohf_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<4) {        \
        goto label;                                     \
    }                                                   \
    copy_ntohf(dst,src);                                \
    (src)=(uint8_t*)(((float*)(src))+1);                   \
} while (0)

#define copy_htond_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<8) {        \
        goto label;                                     \
    }                                                   \
    copy_htond(dst,src);                                \
    (src)=(uint8_t*)(((double*)(src))+1);                  \
} while (0)

#define copy_ntohd_incsrc_bc(dst,src,end,label) do {    \
    if (((uint8_t*)(end))-((uint8_t*)(src))<8) {        \
        goto label;                                     \
    }                                                   \
    copy_ntohd(dst,src);                                \
    (src)=(uint8_t*)(((double*)(src))+1);                  \
} while (0)

#endif

// **************************************************************************
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 
// 2011, 2012 Purdue University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//  this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are 
// those of the authors and should not be interpreted as representing official
// policies, either expressed or implied, of Purdue University or the VERITAS 
// Collaboration.
// **************************************************************************
