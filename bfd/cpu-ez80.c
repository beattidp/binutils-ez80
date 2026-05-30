/* BFD library support routines for the eZ80 architecture.
   Copyright 2005, 2007 Free Software Foundation, Inc.
   Contributed by Arnold Metselaar <arnold_m@operamail.com>

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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

const bfd_arch_info_type bfd_ez80_arch;

/* This routine is provided two arch_infos and
   returns whether they'd be compatible.  */

static const bfd_arch_info_type *
compatible (const bfd_arch_info_type *a, const bfd_arch_info_type *b)
{
  if (a->arch != b->arch)
    return NULL;

  if (a->mach == b->mach)
    return a;

  return (a->arch == bfd_arch_ez80) ? & bfd_ez80_arch : NULL;
}

#define N(name,print,default,next)  \
{ 16, 16, 8, bfd_arch_ez80, name, "ez80", print, 0, default, \
  compatible, bfd_default_scan, bfd_arch_default_fill, next }

#define M(n) &arch_info_struct[n]

static const bfd_arch_info_type arch_info_struct[] =
{
  N (bfd_mach_ez80,       "ez80",        false, NULL)
};

const bfd_arch_info_type bfd_ez80_arch = N (0, "ez80-any", true, M(0));
