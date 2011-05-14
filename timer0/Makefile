include ../Makefile.inc

START_ASM=../shared/start.s timer_entry.s
LDFLAGS=-T../shared/rtx.ld -Wl,-Map=timer.map
TIMER_OBJS=../dbug/dbug.o ../shared/uart/uart.o ../shared/string.o timer.o 

timer.s19: timer_entry.s $(TIMER_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o timer.bin $(START_ASM) $(TIMER_OBJS) 
	$(OBJCPY) --output-format=srec timer.bin timer.s19
	$(OBJDMP) -xdC timer.bin > timer.lst
	chmod u+x timer.s19

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.s19 *.o *.bin *.lst *.map
