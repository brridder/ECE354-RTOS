# Makefile
include Makefile.inc

DIRS=shared timer0 uart1 tests

START_ASM = start.s
LDFLAGS = -Trtx.ld -Wl,-Map=main.map
DEPS=dbug.h memory.h kernel.h soft_interrupts.h rtx.h system_processes.h init.h globals.h process.h
OBJS=dbug.o memory.o kernel.o soft_interrupts.o rtx.o system_processes.o init.o main.o shared/string.o

all: main.s19 uart1 timer0 tests

# Note, GCC builds things in order, it's important to put the
# ASM first, so that it is located at the beginning of our program.
main.s19: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o main.bin $(START_ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec main.bin main.s19

-include $(OBJS:.o=.d)

%.o : %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

uart1: shared force_look
	@cd uart1; $(MAKE) $(MFLAGS)

timer0: shared force_look
	@cd timer0; $(MAKE) $(MFLAGS)

tests: shared force_look
	@cd tests; $(MAKE) $(MFLAGS)

shared:
	@cd shared; $(MAKE) $(MFLAGS)

.PHONY: clean
clean:
	rm -f *.s19 *.o *.bin *.map *.lst *.d
	@for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

force_look:
	@true
