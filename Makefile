# Makefile
include Makefile.inc

DIRS=loader tests core lib 
ASM=start.s
LDFLAGS = -Trtx.ld -Wl,-Map=main.map
DEPS=./lib/dbug.h ./core/kernel.h ./core/soft_interrupts.h rtx.h \
	./processes/system_processes.h ./core/init.h ./lib/string.h \
	process.h loader/rtx_test.h ./core/queues.h
OBJS=./lib/dbug.o ./core/kernel.o ./core/soft_interrupts.o rtx.o \
	./processes/system_processes.o ./core/init.o ./lib/string.o \
	main.o ./core/queues.o
PWD_OBJS=./main.o ./rtx.o
TESTS=rtx_test_dummy.s19 mem_tests.s19 priority_tests.s19 message_tests.s19 provided_mlight.s19

all: tests 

# Note, GCC builds things in order, it's important to put the
# ASM first, so that it is located at the beginning of our program.
#rtx.s19:  $(OBJS)
rtx.s19: build_core build_lib build_processes $(PWD_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o rtx.bin $(ASM) $(OBJS) 
	$(OBJCPY) --output-format=srec rtx.bin rtx.s19

rtx_loader.s19:
	cd loader; $(MAKE) rtx_loader.s19;

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

build_core:
	cd core; $(MAKE);

build_lib:
	cd lib; $(MAKE);

build_processes:
	cd processes; $(MAKE);

build_tests: 
	cd tests; $(MAKE);

tests: main.s19 build_tests
	$(ECHO) Making tests
	@for test in $(TESTS); do ( $(MERGE) full_$$test tests/$$test main.s19; \
	chmod u+x full_$$test); done

