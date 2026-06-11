CC      := cc
CFLAGS  := -Wall -Wextra -O2 -g
BIN_DIR := bin

#system audio capture through PulseAudio on Linux (also covers PipeWire via
#compatibility layer)
#Other platforms build the no-op def in listen.c
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	AUDIO_LIBS := -lpulse-simple -lpulse
else
	AUDIO_LIBS :=
endif

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/lambda: src/main.c src/menu.c src/display.c src/listen.c headers/menu.h headers/display.h headers/listen.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ src/main.c src/menu.c src/display.c src/listen.c $(AUDIO_LIBS)

$(BIN_DIR)/test_renderer: tests/test_renderer.c src/display.c headers/display.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_renderer.c src/display.c

$(BIN_DIR)/test_transliterate: tests/test_transliterate.c src/util.c headers/util.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_transliterate.c src/util.c

$(BIN_DIR)/test_listen: tests/test_listen.c src/listen.c headers/listen.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ tests/test_listen.c src/listen.c $(AUDIO_LIBS) -lm

.PHONY: lambda run test_renderer test_transliterate test_listen clean-tests
lambda: $(BIN_DIR)/lambda

run: $(BIN_DIR)/lambda
	./$(BIN_DIR)/lambda

test_renderer: $(BIN_DIR)/test_renderer
	./$(BIN_DIR)/test_renderer

test_transliterate: $(BIN_DIR)/test_transliterate
	./$(BIN_DIR)/test_transliterate

test_listen: $(BIN_DIR)/test_listen
	./$(BIN_DIR)/test_listen

clean-tests:
	rm -rf $(BIN_DIR)/*.o $(BIN_DIR)/test_*