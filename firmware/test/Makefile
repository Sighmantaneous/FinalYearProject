CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Werror -g -I../components/mesh/include -I.

# Portable core under test (add flooding.c etc. as later steps land)
CORE    := ../components/mesh/packet.c

# Test-side implementations of phy.h + the tests themselves
MOCKS   := mock_phy.c
TESTS   := test_packet.c

BIN     := run_tests

$(BIN): $(CORE) $(MOCKS) $(TESTS)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: test clean
test: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
