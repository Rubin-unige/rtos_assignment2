# Kernel Module Section
KERNELDIR ?= /lib/modules/$(shell uname -r)/build  # Kernel directory
PWD := $(shell pwd)  # Current working directory

# Directories
SRC_DIR := src
BUILD_DIR := build

# Kernel module target
obj-m := special_device_driver.o
KERNEL_SRC := $(SRC_DIR)/special_device_driver.c

# Task scheduler target
TASK_SCHED := $(BUILD_DIR)/tasks_scheduler
TASK_SCHED_SRC := $(SRC_DIR)/tasks_scheduler.c
TASK_SCHED_CFLAGS := -Wall -pthread

.PHONY: all clean kernel user install uninstall

# Default target: Build both kernel module and task scheduler(user level)
all: kernel user

# Build kernel module
kernel:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)/$(SRC_DIR) modules
	mv $(SRC_DIR)/*.ko $(BUILD_DIR)

# Build task scheduler (user level)
user:
	gcc $(TASK_SCHED_CFLAGS) $(TASK_SCHED_SRC) -o $(TASK_SCHED)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Install kernel module
install:
	sudo insmod $(BUILD_DIR)/special_device_driver.ko
	echo "Kernel module installed."

# Uninstall kernel module
uninstall:
	sudo rmmod special_device_driver
	sudo rm -f /dev/special_driver
	echo "Kernel module uninstalled and device file removed."
