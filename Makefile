#
# Makefile for the trace utility.
#
# M. Tim Jones <mtj@mtjones.com>
#

CC = oldgcc
LIBS = -lconfig -lhiredis
CFLAGS = -I/usr/include/hiredis -gdwarf-2 -g3
OBJS = trace.o symbols.o stack.o tools.o

pvtrace: $(OBJS)
	gcc -std=c99 -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -g -Wno-unused-but-set-variable -Wall -std=c99 -c $<

install: pvtrace
	cp pvtrace /usr/local/bin

clean:
	rm -f pvtrace *.o
