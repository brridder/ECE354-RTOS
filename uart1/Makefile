# Makefile

include ../Makefile.inc

START_ASM = ../start.s serial_entry.s
LDFLAGS = -T../rtx.ld -Wl,-Map=../main.map
SERIAL_OBJS=../dbug/dbug.o ../shared/uart/uart.o serial.o 

serial: serial_entry.s $(SERIAL_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o serial.bin $(START_ASM) $(SERIAL_OBJS) 
	$(OBJCPY) --output-format=srec serial.bin serial.s19
	$(OBJDMP) -xdC serial.bin > serial.lst
	chmod u+x serial.s19

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.s19 *.o *.bin *.lst *.map