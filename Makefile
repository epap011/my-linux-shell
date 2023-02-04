############################
# Efthymios Papageorgioou  #
# csd4340@csd.uoc.gr       #
############################
CC     = gcc
CFLAGS =

all:cs345sh

cs345sh:cs345sh.o
	${CC} ${CFLAGS} -o $@ cs345sh.o

cs345sh.o:cs345sh.c
	$(CC) $(CFLAGS) -c cs345sh.c

clean:
	rm -f *.o cs345sh