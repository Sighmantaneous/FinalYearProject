CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Werror -g -I../components/mesh/include -I.

CORE    := ../components/mesh/packet.c

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
