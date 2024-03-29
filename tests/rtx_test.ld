/*
 * MCF5307 CJDesign linker script, modified by 
 * David Grant
 * Thomas Reidemeister 
 * Irene Huang
 */
 
OUTPUT_ARCH(m68k)
/* OUTPUT_FORMAT(srec) */
GROUP(-lgcc)
SEARCH_DIR(.)
__DYNAMIC  =  0;

/*
 * Setup the memory map of the CJDesign 5307 
 * stack grows down from high memory.
 *
 * The memory map look like this:
 * +--------------------+ <- low memory 0x10100000
 * | .text              |
 * |        _etext      |
 * |        ctor list   | the ctor and dtor lists are for
 * |        dtor list   | C++ support
 * +--------------------+
 * | .data              | initialized data goes here
 * |        _edata      |
 * +--------------------+
 * | .bss               |
 * |        __bss_start | start of bss, cleared by crt0
 * |        _end        | start of heap, used by sbrk()
 * +--------------------+
 * .                    .
 * .                    .
 * .                    .
 * |        __stack     | top of stack
 * +--------------------+
 */
MEMORY
{
	/* DG: ram actually starts at 0x1000000, but the vector table usually
     * IH: rtx starts at 0x10100000 and does not exceed 1M
	 *     rtx loader starts at 0x10200000  
     */
  ram     : ORIGIN = 0x10400000, LENGTH = 1M
}

/*
 * Initalize some symbols to be zero so we can reference them in the
 * crt0 without core dumping. These functions are all optional, but
 * we do this so we can have our crt0 always use them if they exist. 
 * This is so BSPs work better when using the crt0 installed with gcc.
 * We have to initalize them twice, so we cover a.out (which prepends
 * an underscore) and coff object file formats.
 */
PROVIDE (hardware_init_hook = 0);
PROVIDE (_hardware_init_hook = 0);
PROVIDE (software_init_hook = 0);
PROVIDE (_software_init_hook = 0);

/*
 * stick everything in ram (of course)
 */
SECTIONS
{
  .text :
  {
    *(.text)
    . = ALIGN(0x4);
     __CTOR_LIST__ = .;
    LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
    *(.ctors)
    LONG(0)
    __CTOR_END__ = .;
    __DTOR_LIST__ = .;
    LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
    *(.dtors)
     LONG(0)
    __DTOR_END__ = .;
    *(.rodata*)
    *(.gcc_except_table) 

    __INIT_SECTION__ = . ;
    LONG (0x4e560000)	/* linkw %fp,#0 */
    *(.init)
    SHORT (0x4e5e)	/* unlk %fp */
    SHORT (0x4e75)	/* rts */

    __FINI_SECTION__ = . ;
    LONG (0x4e560000)	/* linkw %fp,#0 */
    *(.fini)
    SHORT (0x4e5e)	/* unlk %fp */
    SHORT (0x4e75)	/* rts */

    _etext = .;
    *(.lit)
  } > ram

  .data BLOCK (0x4) :
  {
    *(.shdata)
    *(.data)
    _edata = .;
  } > ram

  .bss BLOCK (0x4) :
  {
    __bss_start = . ;
    *(.shbss)
    *(.bss)
    *(COMMON)
    _end =  ALIGN (0x8);
    __end = _end;
  } > ram

  .stab 0 (NOLOAD) :
  {
    *(.stab)
  }

  .stabstr 0 (NOLOAD) :
  {
    *(.stabstr)
  }

  __REGISTER_RTX__ 0x10300000:
  {
    __REGISTER_RTX_ENTRY__ = ABSOLUTE(.) ;
    *(__REGISTER_RTX__);
  }

  /* see rtx_loader.ld for reference */
  __REGISTER_TEST_PROCS_ENTRY__ = 0x10380000 ;


  __REGISTER_TEST_PROCS__ 0x10380000:
  {
      __REGISTER_TEST_PROCS_ENTRY__ = ABSOLUTE(.) ;
      *(__REGISTER_TEST_PROCS__)
  }

  
  /* shared test data goes here */

  __RTX_TEST_DATA__ 0x10500000:
  {
      *(__RTX_TEST_DATA__)
  }

  __RTX_PROFILER_DATA__ 0x10600000:
  {
      *(__RTX_PROFILER_DATA__)
  }
}
