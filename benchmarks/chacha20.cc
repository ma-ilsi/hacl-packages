/*
 *    Copyright 2022 Cryspen Sarl
 *
 *    Licensed under the Apache License, Version 2.0 or MIT.
 *    - http://www.apache.org/licenses/LICENSE-2.0
 *    - http://opensource.org/licenses/MIT
 */

#include "util.h"

#include "Hacl_Chacha20.h"
#ifdef HACL_CAN_COMPILE_VEC128
#include "Hacl_Chacha20_Vec128.h"
#endif
#ifdef HACL_CAN_COMPILE_VEC256
#include "Hacl_Chacha20_Vec256.h"
#endif

const int INPUT_LEN = 1000;

static bytes plaintext(INPUT_LEN, 3);
static bytes key(32, 7);
static bytes nonce(12, 9);
static bytes openssl_nonce(16, 9);
static bytes ciphertext(INPUT_LEN, 0);

static bytes expected_ciphertext = {
  0x1c, 0x9c, 0x4e, 0x92, 0x90, 0x8a, 0xec, 0xbb, 0x34, 0x49, 0xdc, 0xa2, 0xd3,
  0x52, 0xdb, 0x60, 0x3a, 0x0b, 0x08, 0xf6, 0x61, 0x98, 0xd5, 0x08, 0x1e, 0xd4,
  0x46, 0x72, 0x51, 0x77, 0x05, 0x16, 0x92, 0x35, 0xce, 0xbe, 0x8a, 0xe9, 0xbb,
  0x16, 0x3b, 0xda, 0x7a, 0xce, 0x0b, 0xcc, 0x66, 0x81, 0xa2, 0xb4, 0xc0, 0x66,
  0x71, 0x86, 0x80, 0x8e, 0x18, 0x17, 0xbf, 0x4d, 0x45, 0x89, 0xa2, 0xc1, 0xff,
  0x7a, 0xf8, 0xd4, 0xe7, 0x0d, 0x86, 0xa6, 0xde, 0x6b, 0x0d, 0x3b, 0xc3, 0xe4,
  0xa7, 0xd2, 0x7d, 0x0d, 0x49, 0x8e, 0x3b, 0x04, 0x4e, 0x9b, 0x5f, 0xaf, 0x3d,
  0x95, 0x1e, 0x5d, 0x02, 0xd8, 0x9a, 0xd2, 0x3e, 0x1a, 0x9b, 0xb2, 0x4f, 0xbf,
  0x1e, 0x75, 0x05, 0x35, 0x62, 0x9d, 0x6c, 0x61, 0xd3, 0x9c, 0xb2, 0x5b, 0x8c,
  0xe9, 0xd5, 0x93, 0x59, 0xbb, 0xe1, 0xf6, 0x22, 0x30, 0xd0, 0x23, 0x1f, 0x75,
  0x23, 0x16, 0x22, 0x56, 0xa8, 0x7e, 0xfd, 0xaa, 0x64, 0x59, 0x86, 0x76, 0x2d,
  0x08, 0x92, 0x03, 0x74, 0x97, 0x58, 0x17, 0x39, 0x84, 0xd8, 0xaf, 0x30, 0xf2,
  0x07, 0x3a, 0x13, 0xe7, 0xa0, 0xcb, 0x7d, 0x79, 0x06, 0xa6, 0x98, 0x0c, 0x32,
  0x44, 0x4a, 0x10, 0x9e, 0xe8, 0x6b, 0x77, 0x93, 0x02, 0x46, 0x83, 0x02, 0xc5,
  0xf2, 0x83, 0x1f, 0x6a, 0xb7, 0x9f, 0xca, 0x2a, 0x79, 0xd1, 0x12, 0xcb, 0xc1,
  0x79, 0x30, 0x28, 0x0a, 0xe1, 0x9f, 0xa8, 0x85, 0x20, 0x06, 0xcc, 0x83, 0xc0,
  0x96, 0x32, 0x90, 0x69, 0xdb, 0x12, 0x57, 0x63, 0xb9, 0x95, 0x9b, 0xe2, 0xd0,
  0xc6, 0xc6, 0xb3, 0x13, 0x28, 0x07, 0xf0, 0x58, 0xeb, 0x62, 0xa2, 0xe8, 0xe7,
  0x87, 0x5a, 0x94, 0x57, 0x9d, 0x13, 0x0f, 0xce, 0xcd, 0xf0, 0xfb, 0xc7, 0x86,
  0x57, 0x9f, 0x80, 0x2b, 0x76, 0x8b, 0xe6, 0xb3, 0x04, 0x3b, 0x26, 0x02, 0x1c,
  0xb3, 0x75, 0x55, 0xfe, 0x88, 0xda, 0x88, 0x9e, 0x5e, 0xf4, 0xd8, 0xdf, 0xf4,
  0x0d, 0xe7, 0x2b, 0xaf, 0xef, 0x0a, 0xbf, 0x73, 0x70, 0xbe, 0x1d, 0xae, 0x60,
  0x06, 0x1c, 0x0c, 0xa0, 0x1e, 0xf4, 0x02, 0xf9, 0x05, 0x93, 0xd8, 0x79, 0x56,
  0x00, 0x25, 0x5c, 0xc1, 0x0c, 0x2a, 0x3e, 0x5d, 0xbf, 0x0a, 0xd9, 0xac, 0xd8,
  0x1d, 0xe2, 0x91, 0x12, 0x72, 0x61, 0x8e, 0xdb, 0xac, 0x63, 0x07, 0xbe, 0x90,
  0x23, 0x94, 0x26, 0xd4, 0x17, 0x6f, 0x80, 0xd2, 0x5f, 0x24, 0x24, 0xab, 0x69,
  0x6b, 0x2e, 0x59, 0x40, 0x7c, 0xe2, 0xb6, 0xff, 0x70, 0x92, 0xdd, 0xef, 0x84,
  0xbd, 0x5f, 0xd6, 0x35, 0xcf, 0xc8, 0x28, 0x49, 0xf8, 0xb1, 0xf1, 0x88, 0x80,
  0x00, 0x94, 0xc6, 0x1c, 0xd5, 0x25, 0x36, 0x27, 0x90, 0x97, 0x0d, 0x53, 0x68,
  0x6c, 0x1e, 0xa3, 0xe1, 0x87, 0xf0, 0x48, 0xc4, 0xd9, 0x25, 0x1a, 0x52, 0xc8,
  0x35, 0x44, 0x4a, 0x79, 0x73, 0x47, 0x83, 0xc4, 0x84, 0xec, 0x5d, 0x9e, 0xfb,
  0x6d, 0x6e, 0x22, 0x98, 0x08, 0xb1, 0xc1, 0xf1, 0x10, 0x1b, 0xc3, 0x9c, 0x37,
  0xd9, 0x44, 0x63, 0xbb, 0x07, 0x63, 0x72, 0x61, 0x6b, 0xa8, 0x6e, 0x2f, 0xdd,
  0x8b, 0xcd, 0x09, 0x95, 0x1b, 0x75, 0x7b, 0x05, 0xd8, 0x8b, 0x07, 0xe8, 0xf4,
  0x06, 0x80, 0xd8, 0x1e, 0x18, 0xf1, 0x0f, 0xda, 0x63, 0xd7, 0xdc, 0x07, 0xf5,
  0x6d, 0xb5, 0x04, 0x1a, 0x07, 0x25, 0x79, 0x89, 0xde, 0xfd, 0xa9, 0xe8, 0x59,
  0x68, 0x89, 0xde, 0xfe, 0x9d, 0x1c, 0x32, 0x44, 0x4b, 0xcd, 0xcf, 0xc9, 0x65,
  0x39, 0x39, 0x5a, 0x54, 0x53, 0x30, 0xb2, 0xd3, 0xb1, 0xdb, 0x44, 0x1f, 0x1e,
  0xe8, 0x41, 0xc2, 0x60, 0x8b, 0x90, 0x41, 0x0b, 0x5f, 0xcc, 0x2f, 0x3a, 0xea,
  0x1a, 0x24, 0x70, 0x30, 0x31, 0x73, 0x6e, 0x67, 0xfc, 0x53, 0x87, 0x3d, 0x08,
  0xc0, 0x3a, 0x8d, 0x03, 0x00, 0xe7, 0x07, 0xa6, 0x95, 0xa5, 0xf7, 0xc1, 0x1d,
  0x68, 0xb5, 0xb3, 0xb5, 0x1d, 0x3a, 0xaf, 0x6d, 0xaa, 0x78, 0x05, 0xe7, 0x26,
  0x80, 0x9b, 0xeb, 0xe4, 0x96, 0x9f, 0x2a, 0xc3, 0x84, 0x6b, 0xba, 0x06, 0x27,
  0xf8, 0xaa, 0xe9, 0x15, 0xa7, 0xda, 0xe4, 0xbf, 0x7f, 0x7a, 0xbf, 0x89, 0x1a,
  0x22, 0xb3, 0xfd, 0x03, 0x7e, 0xdb, 0x1a, 0xf1, 0xb7, 0x54, 0x86, 0x22, 0x28,
  0xb0, 0xe6, 0x56, 0xb5, 0xef, 0x06, 0x96, 0x54, 0x17, 0xd8, 0x4c, 0xd0, 0x94,
  0x16, 0xb3, 0x00, 0x20, 0xe8, 0xa4, 0xf8, 0x66, 0x82, 0xe0, 0xf5, 0xdf, 0x8b,
  0x1e, 0x90, 0x0f, 0x42, 0xb7, 0x84, 0x75, 0x20, 0xd7, 0x98, 0x04, 0x6d, 0x4c,
  0xdb, 0x3e, 0xa6, 0xec, 0x15, 0x44, 0xa9, 0xab, 0xa6, 0x73, 0xaa, 0x29, 0xf8,
  0x33, 0xc8, 0x0f, 0x87, 0xc5, 0xb5, 0xee, 0x1a, 0x61, 0xe6, 0x33, 0x8d, 0x78,
  0x17, 0x22, 0x50, 0x5d, 0x3b, 0x80, 0xf3, 0xf1, 0x51, 0xe6, 0x0d, 0x75, 0x68,
  0x82, 0x2b, 0xba, 0x38, 0xf6, 0xb5, 0x7b, 0xb9, 0x73, 0x75, 0x44, 0xb0, 0xcd,
  0xaf, 0x25, 0xe9, 0x81, 0xfa, 0x25, 0xe1, 0x22, 0xf0, 0x7c, 0xea, 0x01, 0xfb,
  0x32, 0xff, 0xd6, 0x78, 0x58, 0xb5, 0x8a, 0x9d, 0x94, 0x2e, 0x94, 0xe2, 0x5f,
  0x76, 0x01, 0x68, 0x18, 0xe0, 0x82, 0x19, 0xcb, 0x0a, 0xcf, 0xa2, 0xa4, 0xbc,
  0x80, 0x27, 0xca, 0xc1, 0x9f, 0xa2, 0x41, 0xb5, 0xb3, 0x29, 0xb5, 0xca, 0x7b,
  0x37, 0xc1, 0x31, 0x82, 0x8c, 0x93, 0x37, 0xcd, 0x37, 0x3b, 0xd5, 0x72, 0xd4,
  0xce, 0x5e, 0xa9, 0x69, 0x29, 0x0e, 0x37, 0x45, 0xd2, 0x9f, 0xac, 0x4b, 0xcd,
  0x1a, 0xed, 0x5c, 0x36, 0x35, 0x17, 0xcb, 0x6f, 0x06, 0x87, 0xf5, 0x39, 0xa8,
  0xdb, 0x39, 0x8c, 0xf8, 0xb9, 0x34, 0xfb, 0x6e, 0x98, 0x23, 0xd0, 0x7c, 0xbd,
  0x17, 0xda, 0xcd, 0xe7, 0xf9, 0xbe, 0x89, 0x74, 0x73, 0x25, 0x07, 0xb6, 0x8a,
  0x2c, 0x36, 0x4e, 0x7d, 0x7e, 0xcf, 0x71, 0x29, 0xdf, 0x52, 0x3b, 0xfd, 0x20,
  0xd5, 0xbd, 0x30, 0xd5, 0x4f, 0xd8, 0xdb, 0x13, 0x5e, 0x36, 0xca, 0x1d, 0xbb,
  0x0c, 0x66, 0x81, 0x6f, 0x2e, 0x28, 0xba, 0xef, 0x39, 0xc9, 0x3e, 0xde, 0xfd,
  0xfb, 0xde, 0x7a, 0x33, 0x16, 0x39, 0x22, 0x13, 0x7a, 0x0e, 0x6f, 0x76, 0x7e,
  0xbd, 0xf6, 0xdd, 0x8b, 0xef, 0xcd, 0xce, 0xd9, 0xf6, 0x5f, 0xf4, 0xe8, 0xdd,
  0x93, 0x38, 0x50, 0x01, 0x85, 0xd6, 0xbd, 0x13, 0x15, 0x82, 0xe7, 0x13, 0x33,
  0x47, 0xba, 0x22, 0x6c, 0x25, 0x87, 0xee, 0x90, 0x63, 0x72, 0xa6, 0xb6, 0xae,
  0x13, 0x6c, 0xa4, 0x4d, 0x49, 0xc6, 0x57, 0xe8, 0xf8, 0x75, 0xa8, 0x0c, 0x02,
  0x22, 0x9a, 0x67, 0x2b, 0xde, 0xe5, 0x59, 0xb1, 0x25, 0x26, 0xd4, 0xe4, 0x32,
  0xd2, 0x0a, 0xea, 0xb2, 0x45, 0x9f, 0x4e, 0x7e, 0xf0, 0x82, 0x71, 0xe8, 0x5f,
  0x39, 0x46, 0xc1, 0x12, 0xc5, 0x6f, 0x2e, 0x01, 0x3b, 0xa8, 0x25, 0xb1, 0x99,
  0xb0, 0x12, 0xbf, 0x31, 0x3c, 0x4d, 0xcd, 0xa4, 0x1e, 0xf8, 0xe4, 0x84, 0x45,
  0x9b, 0xd2, 0x73, 0x42, 0x6d, 0x80, 0xa9, 0xa0, 0x17, 0xfe, 0xb4, 0x3b, 0x39,
  0x92, 0xb6, 0x4e, 0x81, 0x6b, 0x30, 0xd0, 0xb6, 0x3b, 0xe1, 0x75, 0x7c, 0x38,
  0x94, 0xfa, 0x09, 0x23, 0x39, 0xdd, 0xb1, 0xef, 0x49, 0xca, 0xab, 0x1b, 0x47,
  0x94, 0xa9, 0x33, 0x6a, 0x08, 0x06, 0xce, 0x78, 0x88, 0x10, 0x0a, 0x2e
};

static void
HACL_Chacha20_32_encrypt(benchmark::State& state)
{
  for (auto _ : state) {
    Hacl_Chacha20_chacha20_encrypt(INPUT_LEN,
                                   ciphertext.data(),
                                   plaintext.data(),
                                   key.data(),
                                   nonce.data(),
                                   0);
    if (ciphertext != expected_ciphertext) {
      state.SkipWithError("Wrong ciphertext");
      break;
    }
  }
}

BENCHMARK(HACL_Chacha20_32_encrypt)->Setup(DoSetup);

#ifdef HACL_CAN_COMPILE_VEC128
static void
HACL_Chacha20_Vec128_encrypt(benchmark::State& state)
{
  if (!vec128_support()) {
    state.SkipWithError("No vec128 support");
    return;
  }

  for (auto _ : state) {
    Hacl_Chacha20_Vec128_chacha20_encrypt_128(INPUT_LEN,
                                              ciphertext.data(),
                                              plaintext.data(),
                                              key.data(),
                                              nonce.data(),
                                              0);
    if (ciphertext != expected_ciphertext) {
      state.SkipWithError("Wrong ciphertext");
      break;
    }
  }
}

BENCHMARK(HACL_Chacha20_Vec128_encrypt)->Setup(DoSetup);
#endif

#ifdef HACL_CAN_COMPILE_VEC256
static void
HACL_Chacha20_Vec256_encrypt(benchmark::State& state)
{
  if (!vec256_support()) {
    state.SkipWithError("No vec256 support");
    return;
  }

  for (auto _ : state) {
    Hacl_Chacha20_Vec256_chacha20_encrypt_256(INPUT_LEN,
                                              ciphertext.data(),
                                              plaintext.data(),
                                              key.data(),
                                              nonce.data(),
                                              0);
    if (ciphertext != expected_ciphertext) {
      state.SkipWithError("Wrong ciphertext");
      break;
    }
  }
}

BENCHMARK(HACL_Chacha20_Vec256_encrypt)->Setup(DoSetup);
#endif

static void
EverCrypt_Chacha20_encrypt(benchmark::State& state)
{
  // TODO
  state.SkipWithError("Unimplemented");
}

BENCHMARK(EverCrypt_Chacha20_encrypt)->Setup(DoSetup);

#ifndef NO_OPENSSL
static void
OpenSSL_Chacha20_encrypt(benchmark::State& state)
{
  // For OpenSSL we need to prepend the counter to the nonce.
  openssl_nonce[0] = 0;
  openssl_nonce[1] = 0;
  openssl_nonce[2] = 0;
  openssl_nonce[3] = 0;

  for (auto _ : state) {
    int out_len, unused_len;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    int result = EVP_EncryptInit_ex2(
      ctx, EVP_chacha20(), key.data(), openssl_nonce.data(), NULL);
    if (result != 1) {
      state.SkipWithError("");
      EVP_CIPHER_CTX_free(ctx);
      break;
    }
    result = EVP_EncryptUpdate(
      ctx, ciphertext.data(), &out_len, plaintext.data(), INPUT_LEN);
    if (result != 1) {
      state.SkipWithError("");
      EVP_CIPHER_CTX_free(ctx);
      break;
    }
    result = EVP_EncryptFinal_ex(ctx, plaintext.data() + out_len, &unused_len);
    if (result != 1 || unused_len != 0) {
      state.SkipWithError("");
      EVP_CIPHER_CTX_free(ctx);
      break;
    }
    EVP_CIPHER_CTX_free(ctx);
    if (ciphertext != expected_ciphertext) {
      state.SkipWithError("Wrong ciphertext");
      break;
    }
  }
}

BENCHMARK(OpenSSL_Chacha20_encrypt)->Setup(DoSetup);
#endif

// TODO: decrypt (even though it should be the same we should measure it)

BENCHMARK_MAIN();
