GROUP=vimsutee
PREFIX=/usr

vimsutee: vimsutee.c
	gcc -Wall -g $< -o $@

clean:
	rm -f core vimsutee *~ *.o

install: vimsutee
	install -g $(GROUP) -m 4755 $< $(PREFIX)/bin
	strip $(PREFIX)/bin/$<
