# If KERNELRELEASE is defined, we've been invoked from the kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
    obj-m := special_device_driver.o  # Kernel module object
    KERNEL_SRC := special_device_driver.c
    TASK_SCHED_SRC := task_scheduler.c
    TASK_SCHED_CFLAGS := -Wall -pthread

# Otherwise, we were called directly from the command line; invoke the kernel build system.
else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build  # Kernel directory
    PWD := $(shell pwd)  # Current working directory

    # Kernel module and task scheduler source files
    obj-m := special_device_driver.o
    TASK_SCHED_SRC := task_scheduler.c
    TASK_SCHED_CFLAGS := -Wall -pthread

    # Bash script for creating device
    BASH_SCRIPT := create_device.sh

    # Device and module paths
    DEVICE_PATH := /dev/special_driver
    MODULE_NAME := special_device_driver

    # Targets
    .PHONY: all clean kernel user install uninstall dmesg_log

    # Default target: Build both kernel module and task scheduler
    all: kernel user

    # Build kernel module (kernel level)
    kernel:
        $(MAKE) -C $(KERNELDIR) M=$(PWD) modules

    # Build task scheduler (user level)
    user:
        gcc $(TASK_SCHED_CFLAGS) $(TASK_SCHED_SRC) -o task_scheduler

    # Install kernel module, create device file
    install: kernel
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

    # View the kernel log (dmesg) related to the module
    dmesg_log:
        @dmesg | grep $(MODULE_NAME) || echo "No relevant kernel log found."

endif
