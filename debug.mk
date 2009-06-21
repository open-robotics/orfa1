gdbinit = $(target).gdb

ddd: $(gdbinit)
	ddd --debugger "avr-gdb -x $(gdbinit)"

gdbserver: $(gdbinit)
	simulavr --device $(MCU) --gdbserver

$(gdbinit): $(target).elf
	@echo "file $<" > $(gdbinit)
	@echo "target remote localhost:1212" >> $(gdbinit)
	@echo "load" >> $(gdbinit)
	@echo "break main" >> $(gdbinit)
	@echo "continue" >> $(gdbinit)
	@echo
	@echo "Use 'avr-gdb -x $(gdbinit)'"
