SOURCES = $(wildcard ./irc/*.c)
INCLUDES = $(wildcard ./irc/*.h)
LIBOUT = /usr/local/lib
INCOUT = /usr/local/include/twitchchat
TARGET = libtwitchchat
COMPILER = gcc

all: install
.PHONY: all

install:
	$(COMPILER) -I /usr/local/include -c $(SOURCES)
	mkdir $(INCOUT)
	ar -cvq $(TARGET).a *.o
	mv $(TARGET).a $(LIBOUT)
	cp $(INCLUDES) $(INCOUT)
	rm *.o

clean:
	rm -rf $(INCOUT)
	rm -f $(LIBOUT)/$(TARGET).a
