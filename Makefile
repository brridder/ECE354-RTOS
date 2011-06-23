# Makefile
include Makefile.inc

DIRS=loader tests
ASM=start.s
LDFLAGS = -Trtx.ld -Wl,-Map=main.map
DEPS=dbug.h kernel.h soft_interrupts.h rtx.h system_processes.h init.h string.h process.h loader/rtx_test.h
OBJS=dbug.o kernel.o soft_interrupts.o rtx.o system_processes.o init.o string.o main.o
TESTS=rtx_test_dummy.s19 mem_tests.s19 priority_tests.s19

all: tests 

# Note, GCC builds things in order, it's important to put the
# ASM first, so that it is located at the beginning of our program.
rtx.s19: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o rtx.bin $(ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec rtx.bin rtx.s19

rtx_loader.s19:
	cd loader; $(MAKE) rtx_loader.s19;

#rtx_test_dummy.s19:
#	cd tests; $(MAKE) rtx_test_dummy.s19;

priority_tests.s19:
	cd tests: $(MAKE) priority_tests.s19;

main.s19: rtx.s19 rtx_loader.s19
	$(ECHO) Merging rtx with loader...
	$(MERGE) main.s19 rtx.s19 loader/rtx_loader.s19
	$(ECHO) Done

-include $(OBJS:.o=.d)

%.o : %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

.PHONY: clean tests build_tests
clean:
	rm -f *.s19 *.o *.bin *.map *.lst *.d
	@for d in $(DIRS); do (cd $$d; $(MAKE) clean); done

build_tests: 
	cd tests; $(MAKE);

tests: main.s19 build_tests
	$(ECHO) Making tests
	@for test in $(TESTS); do ( $(ECHO) Making full_$$test; \
	$(MERGE) full_$$test tests/$$test main.s19; \
	chmod u+x full_$$test); done