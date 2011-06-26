# Makefile
include ../Makefile.inc

ASM=./start.s
LDFLAGS_RTX_LOADER = -Trtx_loader.ld -Wl,-Map=rtx_loader.map
LDFLAGS_RTX_TEST = -Trtx_test.ld -Wl,-Map=rtx_test.map

RTX_LOADER_OBJS = ../lib/dbug.o rtx_loader.o 

all: rtx_loader.s19

rtx_loader.s19: $(RTX_LOADER_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_LOADER) -o rtx_loader.bin $(ASM) $(RTX_LOADER_OBJS) 
	$(OBJCPY) --output-format=srec rtx_loader.bin rtx_loader.s19
	$(OBJDUMP) -xdC rtx_loader.bin > rtx_loader.lst

.c.o:
	$(CC) $(CFLAGS) -c $< 

.PHONY: clean
clean:
	rm -f *.bin *.s19 *.o *.map *.lst 
