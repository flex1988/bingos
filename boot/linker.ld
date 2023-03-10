OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K)
	{
        code = .; _code = .; __code = .;
		*(.multiboot)
		*(.text)
	}

        .rodata BLOCK(4K) : ALIGN(4K)
        {
            *(.rodata)
        }

	.data BLOCK(4K) : ALIGN(4K)
	{
        data = .;
		*(.data)
        *(.symbols)
        PROVIDE(kernel_symbols_start = .);
        PROVIDE(kernel_symbols_end = .);
	}

	.bss ALIGN(4K) :
	{
        bss = .; _bss = .;
		*(COMMON)
		*(.bss)
        kernel_end = .;
	}
}
