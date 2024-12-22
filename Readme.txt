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
5. View the kernel log, long tail to see preemtion in earlier task
		dmesg --notime | tail -n 300

To uninstall the modules, follow these steps:
1. Remove the kernel module
		make uninstall
2. Clean up build files and devices
		make clean

Note: the Makefile automates all the in-between processes like loading module and device creation. 

Issue:
In POSIX thread file, while executing the task, the tasks were holding the CPU hostage while it was wasting time. 
Hence there was no premption. 
I tried to increase the loop time of task 2 and 3 but still there was no preemtion.

Solution:
I used sched_yield() fucntion to free the CPU while the waste time loops were running. 
Doing this other task could prempt the task based on their priorities.

Folder Structure:
* task_scheduler.cpp
	This POSIX thread program creates periodic and aperiodic threads and 
	opens a device file to write messages for each task while simulating real-time execution and real-time scheduling policies.

* special_device.c
	This file implements a Linux kernel module for a simple character device that logs data written to it into the kernel log.

* create_device.sh
	A Bash script to create a character device file using mknod, retrieving its major number dynamically from /proc/devices.

* Makefile
	This Makefile automates the build, installation, and cleanup of a kernel module(special_device), 
	a user-level task scheduler (task_scheduler), and the creation of a device file for the kernel module.
