o2=/Users/stinney/o2

LDFLAGS=-L${o2}/lib/.libs
LIBS=-loracc2
CPPFLAGS=-I${o2}/lib/sys -I${o2}/lib/std -I${o2}/include

lb: lb.o
	$(CC) $(LDFLAGS) -o $@ lb.o $(LIBS)

lb.o: lb.c

clean:
	rm -f lb.o lb
