CC = gcc
BUNNY_CC = bunny-gcc
LD = gcc

CFLAGS = -Wall -Wextra  -ggdb  -Wstrict-prototypes
LDFLAGS= 
LIBS   = 
#-lpthread

CFILES  = $(wildcard *.c)
HFLILES = $(wildcard *.h)
OFILES  = $(CFILES:.c=.o)

CPROG = dirc_test
ODIR = ../bin/
all: $(CPROG) bunny_test


$(CPROG): $(CFILES) $(HFILES)
	$(CC) $(CFILES) $(CFLAGS) -c
	$(LD) $(OFILES) $(LDFLAGS) $(LIBS) -o $(ODIR)$(CPROG)

bunny_test: $(CFILES) $(HFILES)
	$(BUNNY_CC) $(CFILES) $(CFLAGS) $(LIBS) -o $(ODIR)bunny_test

clean:
	$(RM) $(OFILES) $(CPROG) bunny_test