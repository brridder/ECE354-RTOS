# Makefile
include Makefile.inc

DIRS=dbug shared timer0 uart1

START_ASM = start.s
LDFLAGS = -Trtx.ld -Wl,-Map=main.map
DEPS=dbug.h memory.h
OBJS=dbug.o memory.o main.o

all: main.s19 uart1 timer0

# Note, GCC builds things in order, it's important to put the
# ASM first, so that it is located at the beginning of our program.
main.s19: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o main.bin $(START_ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec main.bin main.s19

%.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d

uart1: shared force_look
	cd uart1; $(MAKE) $(MFLAGS)

timer0: shared force_look
	cd timer0; $(MAKE) $(MFLAGS)

shared:
	cd shared; $(MAKE) $(MFLAGS)

.PHONY: clean
clean:
	$(ECHO) cleaning up in .
	-$(RM) -f *.s19 *.o *.bin *.map
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

force_look:
	true