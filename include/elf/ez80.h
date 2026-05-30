/* eZ80 ELF support for BFD.
   Copyright 2014
   Free Software Foundation, Inc.
   Contributed by Douglas Beattie <beattidp@ieee.org>

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _ELF_EZ80_H
#define _ELF_EZ80_H

#include "elf/reloc-macros.h"

/* Processor specific flags for the ELF header e_flags field.  */
#define EF_EZ80_MACH 0x7F

/* If bit #7 is set, it is assumed that the elf file uses local symbols
   as reference for the relocations so that linker relaxation is possible.  */
#define EF_EZ80_LINKRELAX_PREPARED 0x80

#define E_EZ80_MACH_190     1
#define E_EZ80_MACH_L92     2
#define E_EZ80_MACH_F91     3

/* Relocations.  */
START_RELOC_NUMBERS (elf_ez80_reloc_type)
     RELOC_NUMBER (R_EZ80_NONE,         0)
     RELOC_NUMBER (R_EZ80_32,        1)
     RELOC_NUMBER (R_EZ80_7_PCREL,      2)
     RELOC_NUMBER (R_EZ80_13_PCREL,     3)
     RELOC_NUMBER (R_EZ80_16,        4)
     RELOC_NUMBER (R_EZ80_16_PM,     5)
     RELOC_NUMBER (R_EZ80_LO8_LDI,      6)
     RELOC_NUMBER (R_EZ80_HI8_LDI,      7)
     RELOC_NUMBER (R_EZ80_HH8_LDI,      8)
     RELOC_NUMBER (R_EZ80_LO8_LDI_NEG,     9)
     RELOC_NUMBER (R_EZ80_HI8_LDI_NEG,         10)
     RELOC_NUMBER (R_EZ80_HH8_LDI_NEG,         11)
     RELOC_NUMBER (R_EZ80_LO8_LDI_PM,          12)
     RELOC_NUMBER (R_EZ80_HI8_LDI_PM,          13)
     RELOC_NUMBER (R_EZ80_HH8_LDI_PM,          14)
     RELOC_NUMBER (R_EZ80_LO8_LDI_PM_NEG,       15)
     RELOC_NUMBER (R_EZ80_HI8_LDI_PM_NEG,       16)
     RELOC_NUMBER (R_EZ80_HH8_LDI_PM_NEG,       17)
     RELOC_NUMBER (R_EZ80_CALL,             18)
     RELOC_NUMBER (R_EZ80_LDI,                  19)
     RELOC_NUMBER (R_EZ80_6,                    20)
     RELOC_NUMBER (R_EZ80_6_ADIW,               21)
     RELOC_NUMBER (R_EZ80_MS8_LDI,              22)
     RELOC_NUMBER (R_EZ80_MS8_LDI_NEG,          23)
     RELOC_NUMBER (R_EZ80_LO8_LDI_GS,          24)
     RELOC_NUMBER (R_EZ80_HI8_LDI_GS,          25)
     RELOC_NUMBER (R_EZ80_8,             26)
     RELOC_NUMBER (R_EZ80_8_LO8,                27)
     RELOC_NUMBER (R_EZ80_8_HI8,                28)
     RELOC_NUMBER (R_EZ80_8_HLO8,               29)
     /** 24-bit absolute address relocation */
     RELOC_NUMBER (R_EZ80_24,                   30)
     /** 8-bit PC-relative offset relocation */
     RELOC_NUMBER (R_EZ80_8_PCREL,              31)
     /** 8-bit index register displacement relocation */
     RELOC_NUMBER (R_EZ80_DISP8,                32)
END_RELOC_NUMBERS (R_EZ80_max)

#endif /* _ELF_EZ80_H */
