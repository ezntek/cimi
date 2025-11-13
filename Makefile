CC ?= cc
LD ?= ld
INCLUDE = 

SRC = a_string.c lexer.c expr.c stmt.c parser.c ast_printer.c
OBJ = $(SRC:.c=.o)
HEADERS = common.h a_vector.h $(SRC:.c=.h)

RELEASE_CFLAGS = -std=c99 -O2 -Wall -Wextra -pedantic $(INCLUDE) 
DEBUG_CFLAGS = -std=c99 -O0 -g -Wall -Wextra -pedantic -fno-stack-protector -fsanitize=address $(INCLUDE)
TARBALLFILES = Makefile LICENSE.md README.md 3rdparty $(SRC) $(HEADERS) main.c 

TARGET=debug

ifeq (,$(filter clean cleandeps,$(MAKECMDGOALS)))

# goodbye windowze™
ifeq ($(OS),Windows_NT)
$(error building on Windows is not supported.)
endif

ifeq (,$(shell command -v curl))
$(error curl is not installed on your system.)
endif

ifeq ($(TARGET),debug)
	CFLAGS=$(DEBUG_CFLAGS)
else
	CFLAGS=$(RELEASE_CFLAGS)
endif

endif

cimi: deps $(OBJ) $(HEADERS) main.o
	$(CC) $(CFLAGS) -o cimi main.o $(OBJ)

main.o: main.c common.h
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.c %.h common.h
	$(CC) -c $(CFLAGS) $< -o $@

dep_uthash:
	mkdir -p 3rdparty/
	if [ ! -f 3rdparty/uthash.h ]; then \
		curl -fL -o 3rdparty/uthash.h https://raw.githubusercontent.com/troydhanson/uthash/refs/heads/master/src/uthash.h; \
	fi

deps: dep_uthash

tarball:
	mkdir -p cimi
	cp -r $(TARBALLFILES) cimi/
	tar czf cimi.tar.gz cimi
	rm -rf cimi

distclean: clean cleandeps

clean:
	rm -rf cimi cimi.tar.gz cimi $(OBJ) main.o

.PHONY: clean cleanall
