SRC = $(wildcard src/*.c) $(wildcard src/**/*.c)
OBJ_EXCLUDE = src/pffft/fftpack.o src/pffft/test_pffft.o src/tests.o
OBJ = $(filter-out $(OBJ_EXCLUDE),$(SRC:.c=.o))
BUILD = build
LIB = $(BUILD)/lib/librtstft.a
TEST_EXE = $(BUILD)/test_main
CC  = clang
LD = clang
FFTW_CONF_ARGS = --prefix $(shell pwd)/fftw
ifdef RT_DOUBLE
# unused for now
else
endif

.PHONY: all release debug clean distclean build-test run-test test
all: release
debug: CFLAGS = -Wall -Wformat=0 -g -O0
debug:  $(OBJ) | $(FFTW) $(BUILD)
	$(AR) crs $(LIB) $(OBJ) 
release: CFLAGS = -Ofast
release: $(OBJ) | $(FFTW) $(BUILD)
	$(AR) crs $(LIB) $(OBJ) 
$(BUILD):
	-@mkdir -p $(BUILD)
	-@mkdir -p $(BUILD)/lib
	-@mkdir -p $(BUILD)/include
	sh gen_header.sh	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
clean: 
	@echo CLEAN TIME
	-@rm $(OBJ) 2>/dev/null || true
distclean: clean
	-@rm -rf build 2>/dev/null || true
run-test:
	@echo "test running below:\n"
	-@$(TEST_EXE)
build-test:
	$(CC) -c -Ibuild/include -g src/tests.c -o $(BUILD)/tests.o  
	$(LD) -o $(TEST_EXE) $(BUILD)/tests.o -Lbuild/lib -lrtstft 
	dsymutil $(TEST_EXE)
	@rm $(BUILD)/tests.o
test: debug build-test  run-test clean
	-@echo $(CFLAGS)
echo_obj:
	@echo $(SRC)
	@echo $(OBJ)
