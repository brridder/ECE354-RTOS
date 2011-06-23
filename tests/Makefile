# Makefile
include ../Makefile.inc

ASM=#./start.s
LDFLAGS_RTX_LOADER = -Trtx_loader.ld -Wl,-Map=rtx_loader.map
LDFLAGS_RTX_TEST = -Trtx_test.ld -Wl,-Map=rtx_test.map

RTX_TEST_DUMMY_OBJS = ../dbug.o ../string.o rtx_test_dummy.o

all: rtx_test_dummy.s19

rtx_test_dummy.s19: $(RTX_TEST_DUMMY_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o rtx_test_dummy.bin $(ASM) $(RTX_TEST_DUMMY_OBJS) 
	$(OBJCPY) --output-format=srec rtx_test_dummy.bin rtx_test_dummy.s19
	$(OBJDUMP) -xdC rtx_test_dummy.bin > rtx_test_dummy.lst

.c.o:
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean
clean:
	rm -f *.bin *.s19 *.o *.map *.lst 
