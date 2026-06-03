#objdump: -d
#as: --zmasm
#name: ZMASM SET instruction and pseudo-op test

.*: .*

Disassembly of section \.text:

0+ <\.text>:
[ 	]+[0-9a-f]+:[ 	]+cb fe[ 	]+set 7,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+cb fe[ 	]+set 7,\(hl\)
[ 	]+[0-9a-f]+:[ 	]+23[ 	]+inc hl
[ 	]+[0-9a-f]+:[ 	]+3c[ 	]+inc a
[ 	]+[0-9a-f]+:[ 	]+3e 0a[ 	]+ld a,0x0?a
