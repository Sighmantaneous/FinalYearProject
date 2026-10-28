#include <stddef.h>
#include <stdint.h>

#define PHY_MAX_FRAME 255

int phy_send(const uint8_t *buff, size_t len);

int phy_rec(uint8_t *buff, size_t cap);
