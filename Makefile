# Kernel Module Section
KERNELDIR ?= /lib/modules/$(shell uname -r)/build  # Kernel directory
PWD := $(shell pwd)  # Current working directory

# Kernel module target
obj-m := special_device_driver.o  # The kernel module object file

# User-Space Application Section
CC = gcc
CXX = g++  # Use g++ for C++ code
CFLAGS = -Wall -pthread
LDFLAGS =

USER_APP = tasks_scheduler  # Name of the user-space app
USER_SRC = tasks_scheduler.cpp  # User-space application source file

# Default target is to build both the kernel module and the user app
all: build_kernel_module build_user_app

# Build the kernel module
build_kernel_module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

# Build the user-space application
build_user_app: $(USER_APP)

$(USER_APP): $(USER_SRC)
	$(CXX) $(CFLAGS) $(USER_SRC) -o $(USER_APP) $(LDFLAGS)

# Clean the kernel module build files
clean_kernel_module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

# Clean the user-space application build files
clean_user_app:
	rm -f $(USER_APP)

# Clean everything (kernel module + user-space app)
clean: clean_kernel_module clean_user_app

# Install the kernel module (requires root)
install_kernel_module:
	sudo insmod special_device_driver.ko

# Uninstall the kernel module (requires root)
uninstall_kernel_module:
	sudo rmmod special_device_driver

.PHONY: all build_kernel_module build_user_app clean_kernel_module clean_user_app clean install_kernel_module uninstall_kernel_module
