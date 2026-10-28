#include "mock_phy.h"
#include "packet.h"
#include "phy.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_packet_flow(void) {

  mock_phy_resetq();

  packet_t tx = {0};
  tx.dest = 0xFF;
  tx.source = 0x01;
  tx.msg_id = 0x1234;
  tx.type = 0x00;
  tx.ttl = 3;
  tx.hop_start = 3;
  tx.len = 5;
  memcpy(tx.payload, "HelloWorld", 5);

  uint8_t wire[PACKET_MAX];
  int n = packet_serialise(&tx, wire, sizeof wire);
  assert(n == 8 + 5);

  assert(phy_send(wire, (size_t)n) == 0);

  uint8_t rxbuff[PACKET_MAX];
  int got = phy_rec(rxbuff, sizeof rxbuff);
  assert(got == n);

  packet_t rx = {0};

  assert(packet_deserialise(rxbuff, (size_t)got, &rx) == 0);

  assert(rx.dest == tx.dest);
  assert(rx.source == tx.source);
  assert(rx.msg_id == tx.msg_id);
  assert(rx.type == tx.type);
  assert(rx.ttl == tx.ttl);
  assert(rx.hop_start == tx.hop_start);
  assert(rx.len == tx.len);
  assert(memcmp(rx.payload, tx.payload, rx.len) == 0);

  printf("test flow : PASS\r\n\n");
}

int main(void) {

  test_packet_flow();
  return 0;
}
