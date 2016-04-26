SHELL	= /bin/bash
CC	= gcc
RM	= rm -f

CFLAGS	= -Wall -std=gnu99 -ggdb
LIBS	= -lm

SOURCES	    = $(wildcard *.c)
HEADERS	    = $(wildcard *.h)
BASENAMES   = $(basename $(SOURCES))
OBJECTS	    = $(addsuffix .o , $(BASENAMES))
EXEC	    = compilateur

all:: $(EXEC)

.c.o:
	$(CC) $(CFLAGS) -c $*.c

$(EXEC): $(OBJECTS)
	$(CC) $^ -o $@ $(LIBS)

clean::
	$(RM) $(OBJECTS)
	
allclean::
	$(RM) $(OBJECTS) $(EXEC)

depend::
	gcc -MM $(SOURCES)
