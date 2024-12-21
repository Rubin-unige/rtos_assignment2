KERNELDIR ?= /lib/modules/$(shell uname -r)/build  # Kernel directory
PWD := $(shell pwd)  # Current working directory

# Kernel module and task scheduler source files
obj-m := special_device_driver.o
TASK_SCHED_SRC := task_scheduler.cpp
TASK_SCHED_CFLAGS := -lpthread

# Bash script for creating device
BASH_SCRIPT := create_device.sh

# Device and module paths
DEVICE_PATH := /dev/special_device
MODULE_NAME := special_device_driver

# Targets
.PHONY: all clean kernel user install uninstall

# Default target: Build both kernel module and task scheduler
all: kernel user install

# Build kernel module (kernel level)
kernel:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

# Build task scheduler (user level)
user:
	gcc $(TASK_SCHED_SRC) $(TASK_SCHED_CFLAGS) -o task_scheduler

# Install kernel module, create device file
install:
	# Install the kernel module
	sudo insmod $(MODULE_NAME).ko
	echo "Kernel module installed."
	# Make executable
	chmod +x $(BASH_SCRIPT)
	# Run the bash script
	sudo ./$(BASH_SCRIPT)

# Uninstall kernel module and remove the device file
uninstall:
	# Remove the kernel module
	sudo rmmod $(MODULE_NAME)
	# Remove the device file
	sudo rm -f $(DEVICE_PATH)

# Clean up build files and device files
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	@rm -f task_scheduler