.PHONY: clean all

all:
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ kernel.bin VERBOSE=$(VERBOSE)

clean:
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C user/

debug:
	qemu-system-i386 -s -S -m 256M -kernel kernel.bin &
    emacs --eval '(setq gdb-many-windows t)' --eval '(gdb "gdb -i=mi kernel.bin")' --eval '(insert "target remote :1234")' --eval '(gdb-frame-disassembly-buffer)'

