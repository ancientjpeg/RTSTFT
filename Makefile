SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
BUILD = build
EXE = $(BUILD)/main
CC  = clang
LD = clang
LDLIBS = -L./fftw/lib
FFTW_CONF_ARGS = --prefix $(shell pwd)/fftw
ifdef RT_DOUBLE
CFLAGS += -D RT_DOUBLE
LDLIBS += -lfftw3
FFTW = fftw/lib/libfftw3.a
else
LDLIBS += -lfftw3f
FFTW = fftw/lib/libfftw3f.a
FFTW_CONF_ARGS += --enable-float
endif

.PHONY: all release debug clean deepclean run test
all: release
release: OFLAGS = -Odebug
debug:  $(OBJ) | $(FFTW) $(BUILD)
	$(LD) $(LDLIBS) $(OBJ) -o $(EXE)
	dsymutil $(EXE)
release: OFLAGS = -Ofast
release: $(OBJ) | $(FFTW) $(BUILD)
	$(LD) $(LDLIBS)  $(OBJ) -o $(EXE)
$(BUILD):
	-@mkdir -p $(BUILD)
%.o: %.c
	$(CC) -ansi -I fftw/include $(CFLAGS)$(OFLAGS) -c -g -o $@ $<
clean:
	-@rm $(OBJ) 2>/dev/null || true
deepclean: clean
	-@rm -rf build 2>/dev/null || true
ech:
	@echo $(SRC)
setup-lib: $(FFTW)
$(FFTW): | fftw
	cd fftw_src && ./configure $(FFTW_CONF_ARGS)
	make -C fftw_src
	make install -C fftw_src
	make distclean -C fftw_src
fftw:
	mkdir fftw
run: debug
	$(EXE)
test: debug run clean
show:
	echo $(STUFF)
