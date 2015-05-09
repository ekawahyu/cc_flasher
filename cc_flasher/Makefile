SRCDIR = ./src
OBJDIR = ./obj
BINDIR = ./bin
INSTALLDIR = /usr/bin
INSTALLFILENAME = cc_flasher
CC = gcc
LD = gcc

OBJS = \
	$(OBJDIR)/cc_flash.o \
	$(OBJDIR)/ccdebuginterfaceusb.o \
	$(OBJDIR)/ccdebuginterface.o \
	$(OBJDIR)/CCFlasher.o  \
	$(OBJDIR)/debugserver.o  \
	$(OBJDIR)/ccc/ccmutex.o  \
	$(OBJDIR)/ccc/ccthread.o  \
	$(OBJDIR)/ccc/tcpconnection.o  \
	$(OBJDIR)/ccc/tcplistener.o  \
	
	

BINS = $(BINDIR)/cc_flasher 

CFLAGS = -I$(SRCDIR) -O0 -g3  -I$(SRCDIR)/ccc/include -DCC_DEBUGER
LDFLAGS = -lusb  -lstdc++ -lpthread

all: $(BINS)

install: $(BINDIR)/cc_flasher
	install -d $(INSTALLDIR)
	install -s $(BINDIR)/cc_flasher $(INSTALLDIR)/$(INSTALLFILENAME)

$(SRCDIR)/crc8.lst: $(SRCDIR)/crc8.asm
	asx8051 -l $(SRCDIR)/crc8.asm

$(BINDIR)/cc_flasher: $(OBJS) $(SRCDIR)/crc8.lst
	$(LD) -o $(BINDIR)/cc_flasher $(OBJS) $(LDFLAGS)

#$(OBJDIR)/cc_flasher.o: $(SRCDIR)/cc_flasher.cpp 
#	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $< 
	
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $< 
	
clean:
	rm -f $(BINS)
	rm -f $(OBJS)
