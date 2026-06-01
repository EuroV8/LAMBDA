CC      := cc
CFLAGS  := -Wall -Wextra -O2 -g
BIN_DIR := bin

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/test_renderer: tests/test_renderer.c src/display.c headers/display.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_renderer.c src/display.c

$(BIN_DIR)/test_transliterate: tests/test_transliterate.c src/util.c headers/util.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_transliterate.c src/util.c

.PHONY: test_renderer test_transliterate clean-tests
test_renderer: $(BIN_DIR)/test_renderer
	./$(BIN_DIR)/test_renderer

test_transliterate: $(BIN_DIR)/test_transliterate
	./$(BIN_DIR)/test_transliterate

clean-tests:
	rm -rf $(BIN_DIR)/*.o $(BIN_DIR)/test_*