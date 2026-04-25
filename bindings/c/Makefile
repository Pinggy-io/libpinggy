
CC ?= gcc
SDK_INCLUDE_DIR ?= ../../src/sdk

ifeq ($(OS),Windows_NT)
LIBPINGGY_DIR_EXAMPLE := ..\\..\\releases\\windows\\x86_64\MT
LIBPINGGY_DIR_SET_MESSAGE := set LIBPINGGY_DIR=$(LIBPINGGY_DIR_EXAMPLE) && make
LIBPINGGY_DIR_ALT_SET_MESSAGE := $$env:LIBPINGGY_DIR='$(LIBPINGGY_DIR_EXAMPLE)'; make
LIBPINGGY_DIR_ERROR_MESSAGE := Set LIBPINGGY_DIR to the directory containing libpinggy. cmd.exe: $(LIBPINGGY_DIR_SET_MESSAGE) ; PowerShell: $(LIBPINGGY_DIR_ALT_SET_MESSAGE)
RUN_ENV := PATH="$(LIBPINGGY_DIR);$(PATH)"
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
LIBPINGGY_DIR_EXAMPLE := ../../releases/macos/universal
RUN_ENV := DYLD_LIBRARY_PATH="$(LIBPINGGY_DIR):$(LIBPINGGY_DIR)/openssl/lib$(if $(DYLD_LIBRARY_PATH),:$(DYLD_LIBRARY_PATH))"
else ifeq ($(UNAME_S),Linux)
LIBPINGGY_DIR_EXAMPLE := ../../releases/linux/x86_64
RUN_ENV := LD_LIBRARY_PATH="$(LIBPINGGY_DIR):$(LIBPINGGY_DIR)/openssl/lib$(if $(LD_LIBRARY_PATH),:$(LD_LIBRARY_PATH))"
else
LIBPINGGY_DIR_EXAMPLE := /path/to/libpinggy-dir
RUN_ENV :=
endif
LIBPINGGY_DIR_SET_MESSAGE := export LIBPINGGY_DIR=$(LIBPINGGY_DIR_EXAMPLE) && make
LIBPINGGY_DIR_ALT_SET_MESSAGE := LIBPINGGY_DIR=$(LIBPINGGY_DIR_EXAMPLE) make
LIBPINGGY_DIR_ERROR_MESSAGE := Set LIBPINGGY_DIR to the directory containing libpinggy. bash/zsh: $(LIBPINGGY_DIR_SET_MESSAGE) ; one-shot: $(LIBPINGGY_DIR_ALT_SET_MESSAGE)
endif

ifndef LIBPINGGY_DIR
$(error $(LIBPINGGY_DIR_ERROR_MESSAGE))
endif

CPPFLAGS += -I$(SDK_INCLUDE_DIR)
LDFLAGS += -L$(LIBPINGGY_DIR)
LDLIBS += -lpinggy

all: example example_thread

help:
	@echo "Set LIBPINGGY_DIR to the directory containing libpinggy."
	@echo "Example commands for this platform:"
	@echo "  $(LIBPINGGY_DIR_SET_MESSAGE)"
	@echo "  $(LIBPINGGY_DIR_ALT_SET_MESSAGE)"

$(info Using libpinggy from $(LIBPINGGY_DIR))

example: example.c
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

ifeq ($(OS),Windows_NT)
example_run: example
	$(RUN_ENV) ./example.exe
else
example_run: example
	$(RUN_ENV) ./example
endif

example_thread: example_thread.c
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

example_thread_run: example_thread
	$(RUN_ENV) ./example_thread
