SRC = $(wildcard src/*.c) $(wildcard src/**/*.c)
OBJ_EXCLUDE = src/pffft/fftpack.o src/pffft/test_pffft.o
OBJ = $(filter-out $(OBJ_EXCLUDE),$(SRC:.c=.o))
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
	$(LD) $(OBJ) -o $(EXE) $(LFLAGS)
	dsymutil $(EXE)
release: CFLAGS = -Ofast
release: $(OBJ) | $(FFTW) $(BUILD)
	$(LD) $(LDLIBS)  $(OBJ) -o $(EXE) $(LFLAGS)
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
echo_obj:
	@echo $(SRC)
	@echo $(OBJ)
