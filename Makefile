SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
EXE = build/main
CC  = clang


.PHONY: all release debug clean deepclean run test
all: release clean
debug: $(OBJ) 
	$(CC) -lfftw3 -g -lm $(OBJ) -o $(EXE)
	dsymutil $(EXE)
release: OFLAGS = -Ofast
release: $(OBJ) 
	$(CC) -lfftw3 -lm $(OBJ) -o $(EXE)
%.o: %.c
	$(CC) $(OFLAGS) -c -g -o $@ $<
clean:
	-@rm $(OBJ) 2>/dev/null || true
deepclean: clean
	-@rm -rf $(EXE) $(EXE).dSYM 2>/dev/null || true
ech:
	@echo $(SRC)
run: debug
	$(EXE)
test: debug run deepclean