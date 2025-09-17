CC ?= cc
LD ?= ld
INCLUDE = 

SRC = a_string.c main.c 
OBJ = a_string.o main.o 
HEADERS = common.h a_string.h a_vector.h

RELEASE_CFLAGS = -O2 -Wall -Wextra -pedantic $(INCLUDE) 
DEBUG_CFLAGS = -O0 -g -Wall -Wextra -pedantic -fno-stack-protector -fsanitize=address $(INCLUDE)
TARBALLFILES = Makefile LICENSE.md README.md $(SRC) 

TARGET=debug

ifeq ($(TARGET),debug)
	CFLAGS=$(DEBUG_CFLAGS)
else
	CFLAGS=$(RELEASE_CFLAGS)
endif

cimi: $(OBJ) $(HEADERS)
	$(CC) $(CFLAGS) -o cimi $(OBJ)

%.o: %.h

tarball:
	mkdir -p cimi
	cp -r $(TARBALLFILES) cimi/
	tar czf cimi.tar.gz cimi
	rm -rf cimi

distclean: clean cleandeps

clean:
	rm -rf cimi cimi.tar.gz cimi $(OBJ)

.PHONY: clean cleanall
