CC ?= cc
LD ?= ld
INCLUDE = 

SRC = a_string.c lexer.c 
OBJ = $(SRC:.c=.o)
HEADERS = common.h a_vector.h $(SRC:.c=.h)

RELEASE_CFLAGS = -O2 -Wall -Wextra -pedantic $(INCLUDE) 
DEBUG_CFLAGS = -O0 -g -Wall -Wextra -pedantic -fno-stack-protector -fsanitize=address $(INCLUDE)
TARBALLFILES = Makefile LICENSE.md README.md $(SRC) $(HEADERS) main.c 

TARGET=debug

ifeq ($(TARGET),debug)
	CFLAGS=$(DEBUG_CFLAGS)
else
	CFLAGS=$(RELEASE_CFLAGS)
endif

cimi: $(OBJ) $(HEADERS) main.o
	$(CC) $(CFLAGS) -o cimi main.o $(OBJ)

main.o: main.c common.h
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.c %.h common.h
	$(CC) -c $(CFLAGS) $< -o $@

tarball:
	mkdir -p cimi
	cp -r $(TARBALLFILES) cimi/
	tar czf cimi.tar.gz cimi
	rm -rf cimi

distclean: clean cleandeps

clean:
	rm -rf cimi cimi.tar.gz cimi $(OBJ)

.PHONY: clean cleanall
