#!/bin/bash

DEVICE_PATH="/dev/special_device"

# Get the major number from /proc/devices
MAJOR=$(grep -i "special_device" /proc/devices | awk '{print $1}')

if [ -z "$MAJOR" ]; then
    exit 1
fi

# Specify the minor number (this can be static or passed as an argument)
MINOR=0

# Check if the device file already exists
if [ -e "$DEVICE_PATH" ]; then
    echo "Device file '$DEVICE_PATH' already exists."
    exit 0
fi

# Create the device file
if sudo mknod "$DEVICE_PATH" c "$MAJOR" "$MINOR"; then
    sudo chmod +x "$DEVICE_PATH"
    echo "Device created at $DEVICE_PATH with major $MAJOR and minor $MINOR"
else
    echo "Error: Failed to create device file."
    exit 2
fi
