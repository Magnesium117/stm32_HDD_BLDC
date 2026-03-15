#!/bin/zsh
pio run -e debug -t upload
openocd -f /usr/share/openocd/scripts/board/st_nucleo_f4.cfg&
arm-none-eabi-gdb .pio/build/debug/firmware.elf -x debug.gdb
echo "shutdown" | ncat localhost 4444      
