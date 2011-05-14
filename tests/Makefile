# Makefile

include ../Makefile.inc

DEPS = ../dbug.o ../memory.o

START_ASM = ../start.s
LDFLAGS = -T../rtx.ld -Wl,-Map=../main.map
OBJS=$(patsubst %.c,%.o,$(wildcard *.c)) $(DEPS)

test_memory.s19: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o test_memory.bin $(START_ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec test_memory.bin test_memory.s19
	$(OBJDMP) -xdC test_memory.bin > test_memory.lst
	chmod u+x test_memory.s19

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