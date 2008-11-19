#!/bin/sh

arm-elf-objdump -z -b binary -m arm7tdmi -D $*
