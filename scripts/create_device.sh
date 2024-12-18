#!/bin/bash

DEVICE_NAME="special_device_driver"
DEVICE_PATH="/dev/special_driver"

# Get the major number from /proc/devices
MAJOR=$(grep -i "$DEVICE_NAME" /proc/devices | awk '{print $1}')

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

# Create the device file using mknod
sudo mknod $DEVICE_PATH c $MAJOR $MINOR

# Change permissions if necessary
sudo chmod 666 $DEVICE_PATH

echo "Device created at $DEVICE_PATH with major $MAJOR and minor $MINOR"
