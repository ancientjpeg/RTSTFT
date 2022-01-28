SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
BUILD = build
EXE = $(BUILD)/main
CC  = clang
LD = clang
ifdef RT_DOUBLE
CFLAGS += -D RT_DOUBLE
LDLIBS += -lfftw3
else
LDLIBS += -lfftw3f
endif

.PHONY: all release debug clean deepclean run test
all: release
release: OFLAGS = -Odebug
debug: $(OBJ) | $(BUILD)
	$(LD) $(LDLIBS) $(OBJ) -o $(EXE)
	dsymutil $(EXE)
release: OFLAGS = -Ofast
release: $(OBJ) | $(BUILD)
	$(LD) $(LDLIBS) -lm $(OBJ) -o $(EXE)
$(BUILD):
	-@mkdir -p $(BUILD)
%.o: %.c
	$(CC) -ansi $(CFLAGS)$(OFLAGS) -c -g -o $@ $<
clean:
	-@rm $(OBJ) 2>/dev/null || true
deepclean: clean
	-@rm -rf build 2>/dev/null || true
ech:
	@echo $(SRC)
run: debug
	$(EXE)
test: debug run clean


# fftw other arch commands: ./configure --build=x86_64-apple-darwin 
# --enable-float --prefix "/Users/officialnsa/Desktop/installs"