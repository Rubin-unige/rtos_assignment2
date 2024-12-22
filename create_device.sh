#!/bin/bash

DEVICE_NAME="special_device"
DEVICE_PATH="/dev/$DEVICE_NAME"

# Get the major number from /proc/devices
MAJOR=$(grep -i "$DEVICE_NAME" /proc/devices | awk '{print $1}')

if [ -z "$MAJOR" ]; then
    exit 1
fi

# Specify the minor number
MINOR=0

# Check if the device file already exists
if [ -e "$DEVICE_PATH" ]; then
    echo "Device file '$DEVICE_PATH' already exists."
    exit 0
fi

# Create the device file if it does not exist
if sudo mknod "$DEVICE_PATH" c "$MAJOR" "$MINOR"; then
    sudo chmod +x "$DEVICE_PATH"
    echo "Device created at $DEVICE_PATH with major $MAJOR and minor $MINOR"
else
    echo "Error: Failed to create device file."
    exit 2
fi
