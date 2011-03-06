CC=gcc

WARNING_FLAGS=-Wall -Wextra -Werror-implicit-function-declaration -Wshadow -Wstrict-prototypes -pedantic-errors
CFLAGS=-ggdb -O2 -std=c99 $(WARNING_FLAGS)
LDFLAGS=-ggdb 

.c.o:
	$(CC) -c $(CFLAGS) $<

all: soapscan formrequest
soapscan: soapscan.o
	$(CC) $(LDFLAGS) -o soapscan soapscan.o
formrequest: formrequest.o
	$(CC) $(LDFLAGS) -o formrequest formrequest.o

clean:
	rm -f *.o
	rm -f soapscan formrequest

soapscan.o   : soapscan.c
formrequest.o : formrequest.c
