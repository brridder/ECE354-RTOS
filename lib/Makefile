# Makefile

include ../Makefile.inc

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

all: $(OBJS)

#-include $(OBJS:.o=.d)

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
	@for d in $(DIRS); do (cd $$d; $(MAKE) clean); done

