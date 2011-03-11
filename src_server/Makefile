CC = gcc
#BUNNY_CC = bunny-gcc
LD = gcc

CFLAGS = -Wall -Wextra  -ggdb  -Wstrict-prototypes
LDFLAGS= 
LIBS   = -lpthread

CFILES  = $(wildcard *.c)
HFLILES = $(wildcard *.h)
OFILES  = $(CFILES:.c=.o)

CPROG = chat_server
ODIR = ../bin/
all: $(CPROG)
#bunny_test


$(CPROG): $(CFILES) $(HFILES)
	$(CC) $(CFILES) $(CFLAGS) -c
	$(LD) $(OFILES) $(LDFLAGS) $(LIBS) -o $(ODIR)$(CPROG)

#bunny_$(CPROG): $(CFILES) $(HFILES)
#	$(BUNNY_CC) $(CFILES) $(CFLAGS) $(LIBS) -o $(ODIR)bunny_$(CPROG)

clean:
	$(RM) $(OFILES) $(ODIR)$(CPROG)