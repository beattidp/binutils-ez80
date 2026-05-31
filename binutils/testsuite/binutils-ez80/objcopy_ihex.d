#name: eZ80 objcopy to ihex
#as: 
#PROG: objcopy
#objcopy: -O ihex --change-section-address .text=0x0B0000
#objdump: -s -b ihex
#...
Contents of section .*
 b0000 00.*
#...
