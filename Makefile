SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
OBJ += src/pffft/pffft.o
BUILD = build
EXE = $(BUILD)/main
CC  = clang
LD = clang
FFTW_CONF_ARGS = --prefix $(shell pwd)/fftw
ifdef RT_DOUBLE
# unused for now
else
endif

.PHONY: all release debug clean distclean run test
all: release
debug: CFLAGS = -Wall -g -O0
debug:  $(OBJ) | $(FFTW) $(BUILD)
	$(LD) $(OBJ) -o $(EXE)
	dsymutil $(EXE)
release: CFLAGS = -Ofast
release: $(OBJ) | $(FFTW) $(BUILD)
	$(LD) $(LDLIBS)  $(OBJ) -o $(EXE)
$(BUILD):
	-@mkdir -p $(BUILD)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
clean:
	-@rm $(OBJ) 2>/dev/null || true
distclean: clean
	-@rm -rf build 2>/dev/null || true
run: 
	$(EXE)
test: debug run clean
