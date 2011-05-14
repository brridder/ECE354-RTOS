# Makefile

CC=m68k-elf-gcc
CXX=m68k-elf-g++
CFLAGS= -Wall -m5307 -pipe -nostdlib
LD=m68k-elf-gcc
AS=m68k-elf-as
AR=m68k-elf-ar
ARFLAGS=
OBJCPY=m68k-elf-objcopy
ASM=./start.s
LDFLAGS = -Trtx.ld -Wl,-Map=main.map

DEPS=dbug.h memory.h
OBJS=dbug.o memory.o main.o

# Note, GCC builds things in order, it's important to put the
#  ASM first, so that it is located at the beginning of our program.
main.s19: $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) -o main.bin $(ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec main.bin main.s19

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -f *.s19 *.o *.bin *.map
