#
# Make examples
#

priority_and_sleep:
	$(MAKE) -f Makefile.gcc -C ./priority_and_sleep all

priority_and_sleep_qemu:
	$(MAKE) -f Makefile.gcc -C ./priority_and_sleep all qemu

round_robin:
	$(MAKE) -f Makefile.gcc -C ./round_robin all

round_robin_qemu:
	$(MAKE) -f Makefile.gcc -C ./round_robin all qemu

pie_globals:
	$(MAKE) -f Makefile.gcc -C ./pie_globals all

pie_globals_qemu:
	$(MAKE) -f Makefile.gcc -C ./pie_globals all qemu

clean:
	$(MAKE) -f Makefile.gcc -C ./pie_globals clean
	$(MAKE) -f Makefile.gcc -C ./priority_and_sleep clean
	$(MAKE) -f Makefile.gcc -C ./round_robin clean
