include ../Makefile.inc

DEPS=../shared/string.o ../shared/uart/uart.o ../dbug.o

START_ASM=../shared/start.s timer_entry.s
LDFLAGS=-T../shared/rtx.ld -Wl,-Map=timer.map
OBJS=$(patsubst %.c,%.o,$(wildcard *.c)) $(DEPS)

timer.s19: $(OBJS) timer_entry.s
	$(CC) $(CFLAGS) $(LDFLAGS) -o timer.bin $(START_ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec timer.bin timer.s19
	$(OBJDMP) -xdC timer.bin > timer.lst
	chmod u+x timer.s19

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
	rm -f *.s19 *.o *.bin *.lst *.map *.d
