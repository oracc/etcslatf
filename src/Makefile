o2=/Users/stinney/o2

CFLAGS=-g
CPPFLAGS=-I${o2}/lib/sys -I${o2}/lib/std -I${o2}/include
LDFLAGS=-L${o2}/lib/.libs
LIBS=-loracc2

lb: lb.o
	$(CC) $(LDFLAGS) -o $@ lb.o $(LIBS)

lb.o: lb.c

clean:
	rm -f lb.o lb
