CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11
LDFLAGS ?= -pthread
PREFIX  ?= /usr/local
BIN      = corebench
SRC      = src/corebench.c

.PHONY: all clean test install

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test: $(BIN)
	@echo "== corebench self-check =="
	@./$(BIN) --threads 1 --iterations 1000 --quiet || { echo "FAIL: single-thread"; exit 1; }
	@./$(BIN) --threads 4 --iterations 5000 --quiet || { echo "FAIL: quad-thread";   exit 1; }
	@./$(BIN) --threads 4 --iterations 5000 --json > /tmp/cb.json
	@grep -q '"tool":"corebench"' /tmp/cb.json || { echo "FAIL: json output"; exit 1; }
	@rm -f /tmp/cb.json
	@echo "PASS: all checks"

install: $(BIN)
	install -m 0755 $(BIN) $(DESTDIR)$(PREFIX)/bin/$(BIN)

clean:
	rm -f $(BIN)
