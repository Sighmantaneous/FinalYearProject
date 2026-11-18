CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Werror -g -I../components/mesh/include -I.

CORE    := ../components/mesh/packet.c
MOCKS   := mock_phy.c

TESTS   := $(wildcard test_*.c)      # every test_*.c is a test
BINS    := $(TESTS:.c=)              # test_packet_serialise.c -> test_packet_serialise

all: $(BINS)

# pattern rule: build each test_X from test_X.c + core + mocks
test_%: test_%.c $(CORE) $(MOCKS)
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: test clean
test: $(BINS)
	@for t in $(BINS); do \
	    echo "== $$t"; \
	    ./$$t || { echo "FAILED: $$t"; exit 1; }; \
	done

clean:
	rm -f $(BINS)



