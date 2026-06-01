# /**
#  * @file elf32ez80.sh
#  * @brief Linker emulation parameters for the eZ80 ELF target.
#  */
TEMPLATE_NAME=elf
SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-ez80"
OUTPUT_ARCH="ez80"
TEXT_START_ADDR=0x100
MAXPAGESIZE="1"
ARCH=ez80
EMBEDDED=yes
ELFSIZE=32
