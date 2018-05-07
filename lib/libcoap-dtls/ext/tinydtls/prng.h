/* prng.h -- Pseudo Random Numbers
 *
 * Copyright (C) 2010--2012 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the library tinydtls. Please see
 * README for terms of use. 
 */

/** 
 * @file prng.h
 * @brief Pseudo Random Numbers
 */

#ifndef _DTLS_PRNG_H_
#define _DTLS_PRNG_H_

#include "tinydtls.h"

/** 
 * @defgroup prng Pseudo Random Numbers
 * @{
 */

//#ifndef WITH_CONTIKI
#if defined(WITH_LWIP) && !defined(LWIP_RAND)
#include <stdlib.h>

/**
 * Fills \p buf with \p len random bytes. This is the default
 * implementation for prng().  You might want to change prng() to use
 * a better PRNG on your specific platform.
 */
static inline int
dtls_prng(unsigned char *buf, size_t len) {
  while (len--)
    *buf++ = rand() & 0xFF;
  return 1;
}
#endif

static inline void
dtls_prng_init(unsigned short seed) {
	srand(seed);
}

#ifdef WITH_CONTIKI
#include <string.h>
#include "random.h"

#ifdef HAVE_PRNG
static inline int
dtls_prng(unsigned char *buf, size_t len)
{
	return contiki_prng_impl(buf, len);
}
#else
/**
 * Fills \p buf with \p len random bytes. This is the default
 * implementation for prng().  You might want to change prng() to use
 * a better PRNG on your specific platform.
 */
static inline int
dtls_prng(unsigned char *buf, size_t len) {
  unsigned short v = random_rand();
  while (len > sizeof(v)) {
    memcpy(buf, &v, sizeof(v));
    len -= sizeof(v);
    buf += sizeof(v);
    v = random_rand();
  }

  memcpy(buf, &v, len);
  return 1;
}
#endif /* HAVE_PRNG */

static inline void
dtls_prng_init(unsigned short seed) {
  /* random_init() messes with the radio interface of the CC2538 and
   * therefore must not be called after the radio has been
   * initialized. */
#ifndef CONTIKI_TARGET_CC2538DK
	random_init(seed);
#endif
}
#endif /* WITH_CONTIKI */
//modified by archer
#if defined(WITH_LWIP) && defined(LWIP_RAND)
static inline int 
_lwip_prng_impl(unsigned char *buf, size_t len) {
  u32_t v = LWIP_RAND();
  while (len > sizeof(v)) {
    memcpy(buf, &v, sizeof(v));
    len -= sizeof(v);
    buf += sizeof(v);
    v = LWIP_RAND();
  }

  memcpy(buf, &v, len);
  return 1;
}

#ifndef dtls_prng
#define dtls_prng(Buf,Length) _lwip_prng_impl((Buf), (Length))
#endif

#ifndef prng_init
#define prng_init(Value)
#endif

#endif /* WITH_LWIP */

#ifndef prng
/** 
 * Fills \p Buf with \p Length bytes of random data. 
 * 
 * @hideinitializer
 */
#define prng(Buf,Length) dtls_prng_impl((Buf), (Length))
#endif

#ifndef prng_init
/** 
 * Called to set the PRNG seed. You may want to re-define this to
 * allow for a better PRNG.
 *
 * @hideinitializer
 */
#define prng_init(Value) srand((unsigned long)(Value))
#endif

/** @} */

#endif /* _DTLS_PRNG_H_ */
