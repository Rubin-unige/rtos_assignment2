# Kernel Module Section
KERNELDIR ?= /lib/modules/$(shell uname -r)/build  # Kernel directory
PWD := $(shell pwd)  # Current working directory

# Directories
SRC_DIR := src
BUILD_DIR := build
SCRIPTS_DIR := scripts  # Updated to reflect the location of the bash script

# Kernel module target
obj-m := special_device_driver.o
KERNEL_SRC := $(SRC_DIR)/special_device_driver.c

# Task scheduler target
TASK_SCHED := $(BUILD_DIR)/tasks_scheduler
TASK_SCHED_SRC := $(SRC_DIR)/tasks_scheduler.c
TASK_SCHED_CFLAGS := -Wall -pthread

# Bash script to create the device file
BASH_SCRIPT := $(SCRIPTS_DIR)/create_device.sh 

.PHONY: all clean kernel user install uninstall

# Default target: Build both kernel module and task scheduler
all: kernel user

# Build kernel module (kernel level)
kernel:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)/$(SRC_DIR) modules
	mv $(SRC_DIR)/*.ko $(BUILD_DIR)

# Build task scheduler (user level)
user:
	gcc $(TASK_SCHED_CFLAGS) $(TASK_SCHED_SRC) -o $(TASK_SCHED)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Install kernel module and create the device
install: kernel
	# Install the kernel module with sudo
	sudo insmod $(BUILD_DIR)/special_device_driver.ko
	echo "Kernel module installed."
	# Run the bash script to create the device node with sudo
	sudo ./$(BASH_SCRIPT)

# Uninstall kernel module and remove the device file
uninstall:
	# Remove the kernel module with sudo
	sudo rmmod special_device_driver
	# Remove the device file with sudo
	sudo rm -f /dev/special_driver
	echo "Kernel module uninstalled and device file removed."
