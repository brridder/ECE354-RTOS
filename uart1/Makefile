# Makefile

include ../Makefile.inc

START_ASM = ../start.s serial_entry.s
LDFLAGS = -T../rtx.ld -Wl,-Map=../main.map
OBJS = serial.o

serial.s19: $(OBJS) serial_entry.s 
	$(CC) $(CFLAGS) $(LDFLAGS) -o serial.bin $(START_ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec serial.bin serial.s19
	$(OBJDMP) -xdC serial.bin > serial.lst
	chmod u+x serial.s19

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