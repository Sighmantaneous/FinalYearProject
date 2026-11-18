#include "packet.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static packet_t make_pkt(void) {
  packet_t p = {0};
  p.dest = 0x11;
  p.source = 0x22;
  p.msg_id = 0x1234;
  p.type = 0x33;
  p.ttl = 0x44;
  p.hop_start = 0x55;
  p.len = 3;
  p.payload[0] = 0xAA;
  p.payload[1] = 0xBB;
  p.payload[2] = 0xCC;
  return p;
}

static void test_wire_layout(void) {
  packet_t p = make_pkt();
  uint8_t buf[16];
  memset(buf, 0xEE, sizeof buf);

  int n = packet_serialise(&p, buf, sizeof buf);
  assert(n == 11);

  const uint8_t expect[11] = {0x11, 0x22, 0x34, 0x12, 0x33, 0x44,
                              0x55, 0x03, 0xAA, 0xBB, 0xCC};
  assert(memcmp(buf, expect, sizeof expect) == 0);
  assert(buf[11] == 0xEE);
  printf("serialise wire layout: PASS\n");
}

static void test_cap_boundary(void) {
  packet_t p = make_pkt();
  uint8_t buf[16];

  assert(packet_serialise(&p, buf, 11) == 11);
  assert(packet_serialise(&p, buf, 10) == -1);
  printf("serialise cap boundary: PASS\n");
}
static void test_null_args(void) {
  packet_t p = make_pkt();
  uint8_t buf[16];

  assert(packet_serialise(NULL, buf, sizeof buf) == -1);
  assert(packet_serialise(&p, NULL, sizeof buf) == -1);
  printf("serialise null args: PASS\n");
}
static void test_payload_limit(void) {
  packet_t p = make_pkt();
  uint8_t buf[PHY_MAX_FRAME + 8];

  p.len = 247;
  assert(packet_serialise(&p, buf, sizeof buf) == 255);

  p.len = 248;
  assert(packet_serialise(&p, buf, sizeof buf) == -1);
}
int main() {

  test_wire_layout();
  test_cap_boundary();
  test_null_args();
  test_payload_limit();

  return 0;
}
