SRCMOD = string.c
OBJMOD = $(SRCMOD:.c=.o)
CFLAGS = -ansi -pedantic -Ilib

%.o: lib/%.c lib/%.h
	$(CC) $(CFLAGS) -c $< -o $@

all: cathex.c $(OBJMOD)
	$(CC) $(CFLAGS) $^ -o cathex
	rm -f *.o

clean:
	rm -f *.o cathex
