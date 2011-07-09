# Makefile
include ../Makefile.inc

ASM=../start.s
LDFLAGS_RTX_LOADER = -Trtx_loader.ld -Wl,-Map=rtx_loader.map
LDFLAGS_RTX_TEST = -Trtx_test.ld -Wl,-Map=rtx_test.map

OBJS = ../lib/dbug.o ../lib/string.o ../core/kernel.o ../rtx.o \
	../core/soft_interrupts.o ../core/queues.o \
	../processes/system_processes.o  ../uart/uart.o
RTX_TEST_DUMMY_OBJS = rtx_test_dummy.o $(OBJS)
MEM_TESTS_OBJS= mem_tests.o $(OBJS)
PRIORITY_TESTS_OBJS= priority_tests.o $(OBJS)
MESSAGE_TESTS_OBJS= message_tests.o $(OBJS)
DELAY_TESTS_OBJS= delay_tests.o $(OBJS)
CRT_DISPLAY_TESTS_OBJS = crt_display_tests.o $(OBJS)
KCD_TESTS_OBJS = kcd_tests.o $(OBJS)

all: rtx_test_dummy.s19 mem_tests.s19 priority_tests.s19 message_tests.s19 \
	crt_display_tests.s19 delay_tests.s19 kcd_tests.s19

rtx_test_dummy.s19: $(RTX_TEST_DUMMY_OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o rtx_test_dummy.bin $(ASM) $(RTX_TEST_DUMMY_OBJS) 
	$(OBJCPY) --output-format=srec rtx_test_dummy.bin rtx_test_dummy.s19
	$(OBJDUMP) -xdC rtx_test_dummy.bin > rtx_test_dummy.lst

mem_tests.s19: $(MEM_TESTS_OBJS) mem_tests.o
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o mem_tests.bin $(ASM) $(MEM_TESTS_OBJS)
	$(OBJCPY) --output-format=srec mem_tests.bin mem_tests.s19

priority_tests.s19: $(PRIORITY_TESTS_OBJS) priority_tests.o
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o priority_tests.bin $(ASM) $(PRIORITY_TESTS_OBJS)
	$(OBJCPY) --output-format=srec priority_tests.bin priority_tests.s19

message_tests.s19: $(MESSAGE_TESTS_OBJ) message_tests.o
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o message_tests.bin $(ASM) $(MESSAGE_TESTS_OBJS)
	$(OBJCPY) --output-format=srec message_tests.bin message_tests.s19

delay_tests.s19: $(DELAY_TESTS_OBJ) delay_tests.o
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o delay_tests.bin $(ASM) $(DELAY_TESTS_OBJS)
	$(OBJCPY) --output-format=srec delay_tests.bin delay_tests.s19

crt_display_tests.s19: $(CRT_DISPLAY_TESTS_OBJS) crt_display_tests.o
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o crt_display_tests.bin $(ASM) $(CRT_DISPLAY_TESTS_OBJS)
	$(OBJCPY) --output-format=srec crt_display_tests.bin crt_display_tests.s19

kcd_tests.s19: $(KCD_TESTS_OBJS) kcd_tests.o
	$(CC) $(CFLAGS) $(LDFLAGS_RTX_TEST) -o kcd_tests.bin $(ASM) $(KCD_TESTS_OBJS)
	$(OBJCPY) --output-format=srec kcd_tests.bin kcd_tests.s19

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
	rm -f *.bin *.o *.map *.lst `find *.s19 -type f | grep -v '^provided_'`
