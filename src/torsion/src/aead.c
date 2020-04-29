/*!
 * aead.c - aead for libtorsion
 * Copyright (c) 2020, Christopher Jeffrey (MIT License).
 * https://github.com/bcoin-org/libtorsion
 *
 * Resources:
 *   https://tools.ietf.org/html/rfc7539#section-2.8
 *   https://github.com/openssh/openssh-portable
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <torsion/chacha20.h>
#include <torsion/poly1305.h>
#include <torsion/aead.h>
#include "bio.h"
#include "internal.h"

/*
 * AEAD
 */

void
aead_init(aead_t *aead,
          const unsigned char *key,
          const unsigned char *iv,
          size_t iv_len) {
  memset(aead->key, 0, 64);

  chacha20_init(&aead->chacha, key, 32, iv, iv_len, 0);
  chacha20_encrypt(&aead->chacha, aead->key, aead->key, 64);
  poly1305_init(&aead->poly, aead->key);

  aead->mode = 0;
  aead->adlen = 0;
  aead->ctlen = 0;
}

void
aead_aad(aead_t *aead, const unsigned char *aad, size_t len) {
  ASSERT(aead->mode == 0);

  poly1305_update(&aead->poly, aad, len);

  aead->adlen += len;
}

static void
aead_pad16(aead_t *aead, uint64_t size) {
  static const unsigned char padding[16] = {0};
  uint64_t pos = size & 15;

  if (pos > 0)
    poly1305_update(&aead->poly, padding, 16 - pos);
}

void
aead_encrypt(aead_t *aead,
             unsigned char *out,
             const unsigned char *in,
             size_t len) {
  if (aead->mode == 0) {
    aead_pad16(aead, aead->adlen);
    aead->mode = 1;
  }

  ASSERT(aead->mode == 1);

  chacha20_encrypt(&aead->chacha, out, in, len);
  poly1305_update(&aead->poly, out, len);

  aead->ctlen += len;
}

void
aead_decrypt(aead_t *aead,
             unsigned char *out,
             const unsigned char *in,
             size_t len) {
  if (aead->mode == 0) {
    aead_pad16(aead, aead->adlen);
    aead->mode = 2;
  }

  ASSERT(aead->mode == 2);

  aead->ctlen += len;

  poly1305_update(&aead->poly, in, len);
  chacha20_encrypt(&aead->chacha, out, in, len);
}

void
aead_auth(aead_t *aead, const unsigned char *in, size_t len) {
  if (aead->mode == 0) {
    aead_pad16(aead, aead->adlen);
    aead->mode = 3;
  }

  ASSERT(aead->mode == 3);

  aead->ctlen += len;

  poly1305_update(&aead->poly, in, len);
}

void
aead_final(aead_t *aead, unsigned char *tag) {
  uint8_t len[16];

  write64le(len + 0, aead->adlen);
  write64le(len + 8, aead->ctlen);

  if (aead->mode == 0)
    aead_pad16(aead, aead->adlen);

  aead_pad16(aead, aead->ctlen);

  poly1305_update(&aead->poly, len, 16);
  poly1305_final(&aead->poly, tag);

  aead->mode = -1;
}

int
aead_verify(const unsigned char *mac1, const unsigned char *mac2) {
  return poly1305_verify(mac1, mac2) != 0;
}
