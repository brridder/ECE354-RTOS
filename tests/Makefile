# Makefile
include ../Makefile.inc

ASM=../start.s
LDFLAGS_RTX_LOADER = -Trtx_loader.ld -Wl,-Map=rtx_loader.map
LDFLAGS_RTX_TEST = -Trtx_test.ld -Wl,-Map=rtx_test.map

OBJS = ../lib/dbug.o ../lib/string.o ../core/kernel.o ../rtx.o ../core/soft_interrupts.o
RTX_TEST_DUMMY_OBJS = rtx_test_dummy.o $(OBJS)
MEM_TESTS_OBJS= mem_tests.o $(OBJS)
PRIORITY_TESTS_OBJS= priority_tests.o $(OBJS)
MESSAGE_TESTS_OBJS= message_tests.o $(OBJS)

all: rtx_test_dummy.s19 mem_tests.s19 priority_tests.s19 message_tests.s19

rtx_test_dummy.s19: $(RTX_TEST_DUMMY_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o rtx_test_dummy.bin $(ASM) $(RTX_TEST_DUMMY_OBJS) 
	$(OBJCPY) --output-format=srec rtx_test_dummy.bin rtx_test_dummy.s19
	$(OBJDUMP) -xdC rtx_test_dummy.bin > rtx_test_dummy.lst

#%.s19: $(RTX_TEST_DUMMY_OBJS) %.o
#	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o $*.bin $(ASM) $(RTX_TEST_DUMMY_OBJS) $*.o 
#	$(OBJCPY) --output-format=srec $*.bin $*.s19

mem_tests.s19: $(MEM_TESTS_OBJS) mem_tests.o
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o mem_tests.bin $(ASM) $(MEM_TESTS_OBJS)
	$(OBJCPY) --output-format=srec mem_tests.bin mem_tests.s19

priority_tests.s19: $(PRIORITY_TESTS_OBJS) priority_tests.o
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o priority_tests.bin $(ASM) $(PRIORITY_TESTS_OBJS)
	$(OBJCPY) --output-format=srec priority_tests.bin priority_tests.s19

message_tests.s19: $(MESSAGE_TESTS_OBJ) message_tests.o
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o message_tests.bin $(ASM) $(MESSAGE_TESTS_OBJS)
	$(OBJCPY) --output-format=srec message_tests.bin message_tests.s19


%.o : %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d
	@mv -f $*.d $*.d.tmp
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

#.c.o:
#	$(CC) $(CFLAGS) -c $< 

.PHONY: clean
clean:
	rm -f *.bin *.s19 *.o *.map *.lst 
