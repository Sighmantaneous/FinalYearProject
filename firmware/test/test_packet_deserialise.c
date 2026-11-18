#include "packet.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const uint8_t WIRE[11] = {0x11, 0x22, 0x34, 0x12, 0x33, 0x44,
                                 0x55, 0x03, 0xAA, 0xBB, 0xCC};

static void test_valid_frame(void) {
  packet_t out = {0};
  int rc = packet_deserialise(WIRE, sizeof WIRE, &out);
  assert(rc == 0);

  assert(out.dest == 0x11);
  assert(out.source == 0x22);
  assert(out.msg_id == 0x1234);
  assert(out.type == 0x33);
  assert(out.ttl == 0x44);
  assert(out.hop_start == 0x55);
  assert(out.len == 3);
  assert(memcmp(out.payload, &WIRE[8], 3) == 0);
  printf("deserialise valid: PASS\n");
}

static void test_length_gate(void) {
  packet_t out = {0};

  assert(packet_deserialise(WIRE, 7, &out) == -1); /* < header */
  assert(packet_deserialise(WIRE, 10, &out) ==
         -1); /* len says 3, only 2 present */

  uint8_t longer[12];
  memcpy(longer, WIRE, sizeof WIRE);
  longer[11] = 0x00;
  assert(packet_deserialise(longer, sizeof longer, &out) == -1);
  printf("deserialise length gate: PASS\n");
}
static void test_null_args(void) {
  packet_t out = {0};
  assert(packet_deserialise(NULL, sizeof WIRE, &out) == -1);
  assert(packet_deserialise(WIRE, sizeof WIRE, NULL) == -1);
  printf("deserialise null args: PASS\n");
}

static void test_len_too_large(void) {
  uint8_t frame[256] = {0};
  frame[7] = 248;
  packet_t out = {0};
  assert(packet_deserialise(frame, 256, &out) == -1);
}

int main(void) {

  test_valid_frame();
  test_length_gate();
  test_null_args();
  test_len_too_large();
  return 0;
}
