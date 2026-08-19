o2=/Users/stinney/o2

CFLAGS=-g
CPPFLAGS=-I${o2}/lib/sys -I${o2}/lib/std -I${o2}/include
LDFLAGS=-L${o2}/lib/.libs
LIBS=-loracc2

lb: lb.o lb_choose.o lb_log.o lb_print.o
	$(CC) $(LDFLAGS) -o $@ $? $(LIBS)

clean:
	rm -f *.o lb
