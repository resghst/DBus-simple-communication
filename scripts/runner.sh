#!/bin/bash

sudo killall -9 dbus_server dbus_client
rm -rf build
cmake -S . -B build
cmake --build build
if [[ -f "dbus_server" && -f "dbus_client" ]]; then
    echo "Compilation successful."
else
    echo "Compilation failed."
fi