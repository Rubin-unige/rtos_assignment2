Please read before doing anything.

Follow these steps to run the program properly:
1. Switch to super user
		sudo su
2. Compile the module
		make
3. Check if the module has been installed properly
		cat /proc/device
4. Execute the task scheduler 
		./task_scheduler
5. View the kernel log
		dmesg --notime | grep -n 200

To uninstall the module, follow these steps:
1. Remove the kernel module
		make uninstall
2. Clean up build files and devices
		make clean

Note: the Makefile automates all the in-between processes like loading module and device creation. 


Folder Structure:
* task_scheduler.cpp
	This POSIX thread program creates periodic and aperiodic threads and opens a device file to write messages for each task while simulating real-time execution and real-time scheduling policies.

* special_device.c
	This file implements a Linux kernel module for a simple character device that logs data written to it into the kernel log.

* create_device.sh
	A Bash script to create a character device file using mknod, retrieving its major number dynamically from /proc/devices.

* Makefile
	This Makefile automates the build, installation, and cleanup of a kernel module(special_device), a user-level task scheduler (task_scheduler), and the creation of a device file for the kernel module.
