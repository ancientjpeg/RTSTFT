SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
BUILD = build
EXE = $(BUILD)/main
CC  = clang


.PHONY: all release debug clean deepclean run test
all: release clean
debug: $(OBJ) | $(BUILD)
	$(CC) -lfftw3 -g -lm $(OBJ) -o $(EXE)
	dsymutil $(EXE)
release: OFLAGS = -Ofast
release: $(OBJ) | $(BUILD)
	$(CC) -lfftw3 -lm $(OBJ) -o $(EXE)
$(BUILD):
	-@mkdir -p $(BUILD)
%.o: %.c
	$(CC) $(OFLAGS) -c -g -o $@ $<
clean:
	-@rm $(OBJ) 2>/dev/null || true
deepclean: clean
	-@rm -rf build 2>/dev/null || true
ech:
	@echo $(SRC)
run: debug
	$(EXE)
test: debug run clean