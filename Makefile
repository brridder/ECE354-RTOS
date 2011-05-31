# Makefile
include Makefile.inc

DIRS=loader
ASM=start.s
LDFLAGS = -Trtx.ld -Wl,-Map=main.map
DEPS=dbug.h memory.h kernel.h soft_interrupts.h rtx.h system_processes.h init.h string.h process.h loader/rtx_test.h
OBJS=dbug.o memory.o kernel.o soft_interrupts.o rtx.o system_processes.o init.o string.o main.o 

all: full.s19

# Note, GCC builds things in order, it's important to put the
# ASM first, so that it is located at the beginning of our program.
rtx.s19: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o rtx.bin $(ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec rtx.bin rtx.s19

loader/rtx_loader.s19:
	cd loader; $(MAKE) rtx_loader.s19;

full.s19: rtx.s19 loader/rtx_loader.s19
	$(ECHO) Merging rtx with loader...
	$(MERGE) full.s19 rtx.s19 loader/rtx_loader.s19
	chmod u+x full.s19

-include $(OBJS:.o=.d)

%.o : %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

.PHONY: clean
clean:
	rm -f *.s19 *.o *.bin *.map *.lst *.d
	@for d in $(DIRS); do (cd $$d; $(MAKE) clean); done
