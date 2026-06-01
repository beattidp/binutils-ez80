/**********************************  tc-ez80.c  ******************************************************************* 
* File Name					:	tc-ez80.c
*
* Modified by				:	Pradeep & Sowmya
*
* Project					:	binutils port for Ez80
*
* Purpose					:	To port z80 assembler to Ez80 

* Notes			    		:	Taken 'binutils2.24' (https://bitbucket.org/aeon-vantis/binutils-ez80) as the base line for developing the assembler for Ez80
*
* Modification History		:
*		        Released    :   19/03/2014
*       		Version		:   1.1

* Description       		:   Initial version released to ZiLOG, San Jose.
*
*
******************************************************************************************************************/
/* tc-z80.c -- Assemble code for the Zilog eZ80
   Copyright 2005, 2006, 2007, 2008, 2009, 2012 Free Software Foundation, Inc.
   Based on work by Arnold Metselaar <arnold_m@operamail.com>

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"

/*SVES START*/
/* This array holds the Special opcode suffixes that are added to the
instruction set of Ez80 to assist with memory mode switching operations. */
char suffix[5];
/*check_suffix is the flag in order to avoid adding of opcode suffixes more than one. */
char check_suffix = 0;
int cpu_eZ80 = 1 ;		/* selects cpu type */
int adl_mode;			/* selects adl mode 0 or 1 */
int zmasm_syntax = 0;

/*SVES END*/

/* Exported constants.  */
const char comment_chars[] = ";\0";
const char line_comment_chars[] = "#;\0";
const char line_separator_chars[] = "\0";
const char EXP_CHARS[] = "eE\0";
const char FLT_CHARS[] = "RrFf\0";


/* For machine specific options.  */
const char md_shortopts[] = ""; /* None yet.  */

enum options
{
  OPTION_MACH_EZ80 = OPTION_MD_BASE,
  OPTION_MACH_R800,
  OPTION_MACH_IUD,
  OPTION_MACH_WUD,
  OPTION_MACH_FUD,
  OPTION_MACH_IUP,
  OPTION_MACH_WUP,
  OPTION_MACH_FUP,
  OPTION_ZMASM
};

#define INS_EZ80    1
#define INS_UNDOC  2
#define INS_UNPORT 4
#define INS_R800   8

const struct option md_longopts[] =
{
  { "ez80",       no_argument, NULL, OPTION_MACH_EZ80},
  { "r800",      no_argument, NULL, OPTION_MACH_R800},
  { "ignore-undocumented-instructions", no_argument, NULL, OPTION_MACH_IUD },
  { "Wnud",  no_argument, NULL, OPTION_MACH_IUD },
  { "warn-undocumented-instructions",  no_argument, NULL, OPTION_MACH_WUD },
  { "Wud",  no_argument, NULL, OPTION_MACH_WUD },
  { "forbid-undocumented-instructions", no_argument, NULL, OPTION_MACH_FUD },
  { "Fud",  no_argument, NULL, OPTION_MACH_FUD },
  { "ignore-unportable-instructions", no_argument, NULL, OPTION_MACH_IUP },
  { "Wnup",  no_argument, NULL, OPTION_MACH_IUP },
  { "warn-unportable-instructions",  no_argument, NULL, OPTION_MACH_WUP },
  { "Wup",  no_argument, NULL, OPTION_MACH_WUP },
  { "forbid-unportable-instructions", no_argument, NULL, OPTION_MACH_FUP },
  { "Fup",  no_argument, NULL, OPTION_MACH_FUP },
  { "zmasm", no_argument, NULL, OPTION_ZMASM },

  { NULL, no_argument, NULL, 0 }
} ;

const size_t md_longopts_size = sizeof (md_longopts);

extern int coff_flags;
/* Instruction classes that silently assembled.  */
static int ins_ok = INS_EZ80 | INS_UNDOC;
/* Instruction classes that generate errors.  */
static int ins_err = INS_R800;
/* Instruction classes actually used, determines machine type.  */
static int ins_used = INS_EZ80;



//sves start
//Function Declarations to support instruction set of Ez80
static const char *
emit_logic (char prefix, char opcode, const char * args);
static const char *
emit_lea (char prefix, char opcode, const char * args);
static const char *
emit_test (char prefix, char opcode, const char * args);
unsigned int displacement(const char  *args );
unsigned int disp_lea(const char *args);
int add_suffix_call( void );
int add_suffix( void );
int add_suffix_ret( void );
int add_suffix_ld( void );
unsigned int disp_lea(const char *args);
static void
error_report (void);


// Function Definition

//******************************************************************************************
// disp_lea  	 : Provides Displacement Value
// Parameters 	 : args
// Return     	 : Displacement Value
//******************************************************************************************
unsigned int disp_lea(const char *args)
{
	char array[16] = { 0 } ;
	unsigned int c = 0;
	int i = 0, flag = 0, hexa_flag = 0, sign = 1;
	unsigned int number = 0;
	while(*args && *args != '+' && *args != '-')
	{
		args++;
	}
	if (*args == '-')
	{
		sign = -1;
		args++;
	}
	else if (*args == '+')
	{
		args++;
	}
	while(*args)
	{
		c = *args & CHAR_MASK;
		if (*args =='h' || *args =='H')
		{
			args++;
			hexa_flag = 1;
			continue;
		}
		else if (ISXDIGIT(c))
		{
			if (i < 15)
				array[i++] = c ;
			flag = 1; 
		}
		++args;
	}
	array[i] = 0 ;
	if(hexa_flag == 1  )
		sscanf(array,"%x",&number);
	else if(flag == 1)
		sscanf(array,"%d",&number);

return number * sign ;
}
//******************************************************************************************
// add_suffix_call  	: supports ez80 suffix for call instruction
// Parameters 	 		: none
// Return     	 		: error value
//******************************************************************************************
int add_suffix_call( void )
{
	char *q;
	int error_value ;
	error_value = 0;
	if (cpu_eZ80 == 1)
	{
		if((strncmp(".lil",suffix,4) == 0)&& adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x5B;
		}		
		else if((strncmp(".lil",suffix,4) == 0)&& adl_mode == 1)
		{
			error_value = 1;
		}	
		else if((strncmp(".sis",suffix,4) == 0)&& adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x40;
		}
		else if((strncmp(".sis",suffix,4) == 0)&& adl_mode == 0)
		{
			error_value = 1;
		}
		else if((strncmp(".lis",suffix,4) == 0) && adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x40;
		}
		else if((strncmp(".lis",suffix,4) == 0) && adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x49;
		}
		else if((strncmp(".sil",suffix,4) == 0) && adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x49;
		}
		else if((strncmp(".sil",suffix,4) == 0) && adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x52;
		}
		else if((strncmp(".is",suffix,3) == 0) && adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x40;
		}
		else if((strncmp(".is",suffix,3) == 0) && adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x49;
		}
		else if((strncmp(".il",suffix,3) == 0)  && adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x52;
		}			
		else if((strncmp(".il",suffix,3) == 0) && adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x5B;
		}
		else if((strncmp(".",suffix,1) == 0))
		{
			error_value = 1 ;
		}
	}
	else if (cpu_eZ80 == 0)
	{
		if (suffix[0] =='.' )
			error_value = 1 ;
	}		
	return error_value;
}

//******************************************************************************************
// add_suffix  			: supports suffix for ez80 instructions
// Parameters 	 		: none
// Return     	 		: error value
//******************************************************************************************
int add_suffix( void )
{
	char *q;
	int error_value ;
	error_value = 0;
	if 	(check_suffix == 1)
		check_suffix = 0;
	else if (cpu_eZ80 == 1)
	{
		if((strncmp(".s",suffix,2) == 0)&& adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x52;
		}			
		else if((strncmp(".l",suffix,2) == 0)&& adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x49;
		}	
		else if((strncmp(".s",suffix,2) == 0)&& adl_mode == 0)
		{
			error_value = 1;
		}
		else if((strncmp(".l",suffix,2) == 0)&& adl_mode == 1)
		{
			error_value = 1;
		}
	}		
	else if (cpu_eZ80 == 0)
	{
		if (suffix[0] =='.' )
			error_value = 1 ;
	}		
return error_value;
}

//******************************************************************************************
// add_suffix_ret  		: supports suffix for return instructions
// Parameters 	 		: none
// Return     	 		: error value
//******************************************************************************************
int add_suffix_ret( void )
{
	char *q;
	int error_value;
	error_value = 0;
	if (cpu_eZ80 == 1)
	{	
		if((strncmp(".l",suffix,2) == 0)&& adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x49;
		}		
		else if((strncmp(".l",suffix,2) == 0)&& adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x5B;
		}
		else if((strncmp(".s",suffix,2) == 0)&& adl_mode == 1)
		{
			error_value = 1;
		}
		else if((strncmp(".s",suffix,2) == 0)&& adl_mode == 0)
		{
			error_value = 1;
		}	
	}		
	else if (cpu_eZ80 == 0)
	{
		if (suffix[0] =='.' )
			error_value = 1 ;
	}		
	return error_value;
}
//******************************************************************************************
// add_suffix_ld  		: supports suffix for load instructions
// Parameters 	 		: none
// Return     	 		: error value
//******************************************************************************************
int add_suffix_ld( void )
{
	char *q;
	int error_value ;
	error_value = 0;
	if (cpu_eZ80 == 1)
	{
		if((strncmp(".lil",suffix,4) == 0)&& adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x5B;
		}		
		else if((strncmp(".lil",suffix,4) == 0)&& adl_mode == 1)
		{
			error_value = 1;
		}	
		else if((strncmp(".sis",suffix,4) == 0)&& adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x40;
		}
		else if((strncmp(".sis",suffix,4) == 0)&& adl_mode == 0)
		{
			error_value = 1;
		}
		else if((strncmp(".lis",suffix,4) == 0) && adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x40;
		}
		else if((strncmp(".lis",suffix,4) == 0) && adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x49;
		}
		else if((strncmp(".sil",suffix,4) == 0) && adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x49;
		}
		else if((strncmp(".sil",suffix,4) == 0) && adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x52;
		}
		else if((strncmp(".is",suffix,3) == 0) && adl_mode == 0)
		{
			error_value = 1;
		}
		else if((strncmp(".is",suffix,3) == 0) && adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x40;
		}
		else if((strncmp(".il",suffix,3) == 0)  && adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x5B;
		}			
		else if((strncmp(".il",suffix,3) == 0) && adl_mode == 1)
		{
			error_value = 1;
		}
		else if((strncmp(".s",suffix,2) == 0) && adl_mode == 1)
		{
			q = frag_more (1);
			*q++ = 0x52;
		}	
		else if((strncmp(".l",suffix,2) == 0) && adl_mode == 0)
		{
			q = frag_more (1);
			*q++ = 0x49;
		}
		else if((strncmp(".l",suffix,2) == 0) && adl_mode == 1 )
		{
			q = frag_more (1);
			*q++ = 0x5B;
		}	
		else if((strncmp(".s",suffix,2) == 0) && adl_mode == 0 )
		{
			error_value = 1 ;
		}			
	}
	else if (cpu_eZ80 == 0)
	{
		if (suffix[0] =='.' )
			error_value = 1 ;
	}		
	return error_value;
}

//******************************************************************************************
// displacement  : Provide Displacement Value
// Parameters 	 : args
// Return     	 : Displacement Value
//******************************************************************************************
unsigned int displacement(const char  *args )
{
	unsigned int number = 0;
	char array[16] = { 0 } ;
	unsigned int c = 0;
	int i = 0, flag = 0, hexa_flag = 0, sign = 1;
	while(*args)
	{
		if(*args != '+' && *args != '-')
		{
			args++;
			continue;
		}
		else
		{	
			if (*args == '-')
				sign = -1;
			args++;
			while(*args && *args != ')')
			{
				c = *args & CHAR_MASK;
				if (*args =='h' || *args =='H')
				{
					args++;
					hexa_flag = 1;
					continue;
				}
				else if (ISXDIGIT(c))
				{
					if (i < 15)
						array[i++] = c ;
					flag = 1; 
				}
				++args;
			}
			array[i] = 0 ;
			if(hexa_flag == 1  )
				sscanf(array,"%x",&number);
			else if(flag == 1)
				sscanf(array,"%d",&number);
		}
	break;
	}
	return number * sign ;
}
//SVES END

int
md_parse_option (int c, const char* arg ATTRIBUTE_UNUSED)
{
  switch (c)
    {
    default:
      return 0;
    case OPTION_MACH_EZ80:
      ins_ok &= ~INS_R800;
      ins_err |= INS_R800;
      break;
    case OPTION_MACH_R800:
      ins_ok = INS_EZ80 | INS_UNDOC | INS_R800;
      ins_err = INS_UNPORT;
      break;
    case OPTION_MACH_IUD:
      ins_ok |= INS_UNDOC;
      ins_err &= ~INS_UNDOC;
      break;
    case OPTION_MACH_IUP:
      ins_ok |= INS_UNDOC | INS_UNPORT;
      ins_err &= ~(INS_UNDOC | INS_UNPORT);
      break;
    case OPTION_MACH_WUD:
      if ((ins_ok & INS_R800) == 0)
	{
	  ins_ok &= ~(INS_UNDOC|INS_UNPORT);
	  ins_err &= ~INS_UNDOC;
	}
      break;
    case OPTION_MACH_WUP:
      ins_ok &= ~INS_UNPORT;
      ins_err &= ~(INS_UNDOC|INS_UNPORT);
      break;
    case OPTION_MACH_FUD:
      if ((ins_ok & INS_R800) == 0)
	{
	  ins_ok &= (INS_UNDOC | INS_UNPORT);
	  ins_err |= INS_UNDOC | INS_UNPORT;
	}
      break;
    case OPTION_MACH_FUP:
      ins_ok &= ~INS_UNPORT;
      ins_err |= INS_UNPORT;
      break;
    case OPTION_ZMASM:
      zmasm_syntax = 1;
      break;
    }

  return 1;
}

void
md_show_usage (FILE * f)
{
  fprintf (f, "\n\
CPU model/instruction set options:\n\
\n\
  -ez80\t\t  assemble for EZ80\n\
  -ignore-undocumented-instructions\n\
  -Wnud\n\
\tsilently assemble undocumented EZ80-instructions that work on R800\n\
  -ignore-unportable-instructions\n\
  -Wnup\n\
\tsilently assemble all undocumented EZ80-instructions\n\
  -warn-undocumented-instructions\n\
  -Wud\n\
\tissue warnings for undocumented EZ80-instructions that work on R800\n\
  -warn-unportable-instructions\n\
  -Wup\n\
\tissue warnings for other undocumented EZ80-instructions\n\
  -forbid-undocumented-instructions\n\
  -Fud\n\
\ttreat all undocumented ez80-instructions as errors\n\
  -forbid-unportable-instructions\n\
  -Fup\n\
\ttreat undocumented ez80-instructions that do not work on R800 as errors\n\
  -r800\t  assemble for R800\n\n\
Default: -ez80 -ignore-undocument-instructions -warn-unportable-instructions.\n");
}

static symbolS * zero;

struct reg_entry
{
  char* name;
  int number;
};
#define R_STACKABLE (0x80)
#define R_ARITH     (0x40)
#define R_IX        (0x20)
#define R_IY        (0x10)
#define R_INDEX     (R_IX | R_IY)

#define REG_A (7)
#define REG_B (0)
#define REG_C (1)
#define REG_D (2)
#define REG_E (3)
#define REG_H (4)
#define REG_L (5)
#define REG_F (6 | 8)
#define REG_I (9)
#define REG_R (10)
#define REG_MB (12)		//SVES ADDED to support eZ80 instructions.

#define REG_AF (3 | R_STACKABLE)
#define REG_BC (0 | R_STACKABLE | R_ARITH)
#define REG_DE (1 | R_STACKABLE | R_ARITH)
#define REG_HL (2 | R_STACKABLE | R_ARITH)
#define REG_IX (REG_HL | R_IX)
#define REG_IY (REG_HL | R_IY)
#define REG_SP (3 | R_ARITH)

static const struct reg_entry regtable[] =
{
  {"a",  REG_A },
  {"af", REG_AF },
  {"b",  REG_B },
  {"bc", REG_BC },
  {"c",  REG_C },
  {"d",  REG_D },
  {"de", REG_DE },
  {"e",  REG_E },
  {"f",  REG_F },
  {"h",  REG_H },
  {"hl", REG_HL },
  {"i",  REG_I },
  {"ix", REG_IX },
  {"ixh",REG_H | R_IX },
  {"ixl",REG_L | R_IX },
  {"iy", REG_IY },
  {"iyh",REG_H | R_IY },
  {"iyl",REG_L | R_IY },
  {"l",  REG_L },
  {"r",  REG_R },
  {"sp", REG_SP },
  {"mb", REG_MB },			//SVES ADDED to support eZ80 instructions.
} ;

#define BUFLEN 16 /* Large enough for any keyword.  */

void
md_begin (void)
{
  expressionS nul, reg;
  char * p;
  unsigned int i, j, k;
  char buf[BUFLEN];
  reg.X_op = O_register;
  reg.X_md = 0;
  reg.X_add_symbol = reg.X_op_symbol = 0;
  for ( i = 0 ; i < ARRAY_SIZE ( regtable ) ; ++i )
    {
      reg.X_add_number = regtable[i].number;
      k = strlen ( regtable[i].name );
      buf[k] = 0;
      if ( k+1 < BUFLEN )
        {
          for ( j = ( 1<<k ) ; j ; --j )
            {
              for ( k = 0 ; regtable[i].name[k] ; ++k )
                {
                  buf[k] = ( j & ( 1<<k ) ) ? TOUPPER ( regtable[i].name[k] ) : regtable[i].name[k];
                }
              symbolS * psym = symbol_find_or_make(buf);
	      S_SET_SEGMENT(psym, reg_section);
	      symbol_set_value_expression(psym, &reg);
            }
        }
    }
  p = input_line_pointer;
  input_line_pointer = "0";
  nul.X_md=0;
  expression (& nul);
  input_line_pointer = p;
  zero = make_expr_symbol (& nul);
  /* We do not use relaxation (yet).  */
  linkrelax = 0;
}

void
ez80_md_end (void)
{
  int mach_type;
  if (ins_used & (INS_UNPORT | INS_R800))
    ins_used |= INS_UNDOC;

  switch (ins_used)
    {
    case INS_EZ80:
      mach_type = bfd_mach_ez80strict;
      break;
    case INS_EZ80|INS_UNDOC:
      mach_type = bfd_mach_ez80;
      break;
    case INS_EZ80|INS_UNDOC|INS_UNPORT:
      mach_type = bfd_mach_ez80full;
      break;
    case INS_EZ80|INS_UNDOC|INS_R800:
      mach_type = bfd_mach_r800;
      break;
    default:
      mach_type = 0;
    }

  if (stdoutput)
    bfd_set_arch_mach (stdoutput, TARGET_ARCH, mach_type);
}

static const char *
skip_space (const char *s)
{
  while (*s == ' ' || *s == '\t' || *s == '\r')
    ++s;
  return s;
}

/* A non-zero return-value causes a continue in the
   function read_a_source_file () in ../read.c.  */
int
ez80_start_line_hook (void)
{
  char *p, quote;
  char buf[4];

  /* Convert one character constants.  */
  for (p = input_line_pointer; *p && *p != '\n'; ++p)
    {
      switch (*p)
	{
	case '\'':
	  if (p[1] != 0 && p[1] != '\'' && p[2] == '\'')
	    {
	      snprintf (buf, 4, "%3d", (unsigned char)p[1]);
	      *p++ = buf[0];
	      *p++ = buf[1];
	      *p++ = buf[2];
	    }
	  break;
	case '"':
	  for (quote = *p++; quote != *p && '\n' != *p; ++p)
	    /* No escapes.  */ ;
	  if (quote != *p)
	    {
	      as_bad (_("-- unterminated string"));
	      ignore_rest_of_line ();
	      return 1;
	    }
	  break;
	}
    }
  /* Check for <label>[:] [.](EQU|DEFL) <value>.  */
  if (is_name_beginner (*input_line_pointer))
    {
      char c, *rest, *line_start;
      int len;

      if (ignore_input ())
	return 0;

      c = get_symbol_name (&line_start);
      rest = input_line_pointer + 1;

      if (*rest == ':')
	++rest;
      if (*rest == ' ' || *rest == '\t')
	++rest;
      if (*rest == '.')
	++rest;
      if (strncasecmp (rest, "EQU", 3) == 0)
	len = 3;
      else if (strncasecmp (rest, "DEFL", 4) == 0)
	len = 4;
      else
	len = 0;
      if (len && (!ISALPHA(rest[len]) ) )
	{
	  /* Handle assignment here.  */
	  if (line_start[-1] == '\n')
	    {
	      bump_line_counters ();
	      LISTING_NEWLINE ();
	    }
	  input_line_pointer = rest + len - 1;
	  /* Allow redefining with "DEFL" (len == 4), but not with "EQU".  */
	  equals (line_start, len == 4);
	  return 1;
	}
      else
	{
	  /* Restore line and pointer.  */
	  restore_line_pointer (c);
	  input_line_pointer = line_start;
	}
    }
  return 0;
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

const char *
md_atof (int type ATTRIBUTE_UNUSED, char *litP ATTRIBUTE_UNUSED,
	 int *sizeP ATTRIBUTE_UNUSED)
{
  return _("floating point numbers are not implemented");
}

valueT
md_section_align (segT seg ATTRIBUTE_UNUSED, valueT size)
{
  return size;
}

long
md_pcrel_from (fixS * fixp)
{
  return fixp->fx_where +
    fixp->fx_frag->fr_address + 1;
}

typedef const char * (asfunc)(char, char, const char*);

typedef struct _table_t
{
  char* name;
  char prefix;
  char opcode;
  asfunc * fp;
} table_t;

/* Compares the key for structs that start with a char * to the key.  */
static int
key_cmp (const void * a, const void * b)
{
  const char *str_a, *str_b;

  str_a = *((const char**)a);
  str_b = *((const char**)b);
  return strcmp (str_a, str_b);
}

char buf[BUFLEN];
const char *key = buf;

/* Prevent an error on a line from also generating
   a "junk at end of line" error message.  */
static char err_flag;

static void
error (const char * message)
{
  as_bad ("%s", message);
  err_flag = 1;
}

static void
ill_op (void)
{
  error (_("illegal operand"));
}
//SVES START
//******************************************************************************************
// error_report  		: recognises invalid instructions
// Parameters 	 		: none
// Return     	 		: none
//******************************************************************************************
static void
error_report (void)
{
  error (_("Invalid instruction"));
}
//SVES END


static void
wrong_mach (int ins_type)
{
	(void) ins_type;
#if 0 
  const char *p;

  switch (ins_type)
    {
    case INS_UNDOC:
      p = "undocumented instruction";
      break;
    case INS_UNPORT:
      p = "instruction does not work on R800";
      break;
    case INS_R800:
      p = "instruction only works R800";
      break;
    default:
      p = 0; /* Not reachable.  */
    }

  if (ins_type & ins_err)
    error (_(p));
  else
    as_warn (_(p));
#endif
}

static void
check_mach (int ins_type)
{
  if ((ins_type & ins_ok) == 0)
    wrong_mach (ins_type);
  ins_used |= ins_type;
}

/* Check whether an expression is indirect.  */
static int
is_indir (const char *s)
{
  char quote;
  const char *p;
  int indir, depth;

  /* Indirection is indicated with parentheses.  */
  indir = (*s == '(');
  for (p = s, depth = 0; *p && *p != ','; ++p)
    {
      switch (*p)
	{
	case '"':
	case '\'':
	  for (quote = *p++; quote != *p && *p != '\n'; ++p)
	    if (*p == '\\' && p[1])
	      ++p;
	  break;
	case '(':
	  ++ depth;
	  break;
	case ')':
	  -- depth;
	  if (depth == 0)
	    {
	      p = skip_space (p + 1);
	      if (*p && *p != ',')
		indir = 0;
	      --p;
	    }
	  if (depth < 0)
	    error (_("mismatched parentheses"));
	  break;
	}
    }

  if (depth != 0)
    error (_("mismatched parentheses"));

  return indir;
}

/* Check whether a symbol involves a register.  */
static int 
contains_register(symbolS *sym)
{
  if (sym)
    {
      expressionS * ex = symbol_get_value_expression (sym);

      switch (ex->X_op)
	{
	case O_register:
	  return 1;

	case O_add:
	case O_subtract:
	  if (ex->X_op_symbol && contains_register (ex->X_op_symbol)) {
	    return 1;
      }
	  /* Fall through.  */
	case O_uminus:
	case O_symbol:
	  if (ex->X_add_symbol && contains_register (ex->X_add_symbol)) {
	    return 1;
      }
	  break;

	default:
	  break;
	}
    }

  return 0;
}


//Sves Start
//*************************************************************************************************************
//parse_exp_not_indexed : Parse general expression to check for missing operand,bad expression. 
//parameters 			: Operand as a string and op is a pointer to a structure expressionS 
//						: op->X_md -> status of legal operand,
//  					: op->X_add_number -> to add Operand,op->X_op -> to identify Operand.
//Return 	  			: Returns a Pointer pointing to the end of operand.
//*************************************************************************************************************
//Sves End
/* Parse general expression, not looking for indexed addressing.  */
static const char *
parse_exp_not_indexed (const char *s, expressionS *op)
{
  const char *p;
  int indir;
  segT dummy;
  memset (op, 0, sizeof (*op));
  p = skip_space (s);
  op->X_md = indir = is_indir (p);
  input_line_pointer = (char*) s ;
  dummy = expression (op);
  op->X_md = indir; // Restore X_md since expr_copy may have clobbered it
  switch (op->X_op)
    {
    case O_absent:
      error (("missing operand"));
      break;
    case O_illegal:
		error (("bad expression syntax"));
		break;
	default:		//SVES added, to avoid compiler warnings
		break;		//SVES added
	}
  return input_line_pointer;
}

//Sves Start
//*************************************************************************************************************
//parse_exp 	: Parse expression.
//parameters 	: Operand as a string and  op is a pointer to a structure expressionS
//				: op->X_md -> status of legal operand,
//				: op->X_add_number -> to add Operand,op->X_op -> to identify Operand.
//Return 	  	: Returns a Pointer pointing to the end of operand.
//*************************************************************************************************************
//Sves End

/* Parse expression, change operator to O_md1 for indexed addressing*/
static const char *
parse_exp (const char *s, expressionS *op)
{
  const char* res = parse_exp_not_indexed (s, op);
  switch (op->X_op)
    {
    case O_add:
    case O_subtract:
      if (op->X_md && (O_register == symbol_get_value_expression(op->X_add_symbol)->X_op))
        {
	  int rnum = symbol_get_value_expression(op->X_add_symbol)->X_add_number;
		if ( ((REG_IX != rnum) && (REG_IY != rnum)) || contains_register(op->X_op_symbol) )
		{
			ill_op();
		}
	  else
	    {
	      if (O_subtract == op->X_op)
	      {
		  expressionS minus;
		  minus.X_op = O_uminus;
		  minus.X_add_number = 0;
		  minus.X_add_symbol = op->X_op_symbol;
		  minus.X_op_symbol = 0;
		  op->X_op_symbol = make_expr_symbol(&minus);
		  op->X_op = O_add;
	       }
	      symbol_get_value_expression(op->X_op_symbol)->X_add_number += op->X_add_number;
	      op->X_add_number = rnum;
	      op->X_add_symbol = op->X_op_symbol;
	      op->X_op_symbol = 0;
	      op->X_op = O_md1;
	    }
		}
      break;

    case O_symbol:
      if (op->X_md && op->X_add_symbol && symbol_get_value_expression(op->X_add_symbol)->X_op == O_register)
        {
          int rnum = symbol_get_value_expression(op->X_add_symbol)->X_add_number;
          if (rnum == REG_IX || rnum == REG_IY)
            {
              expressionS const_disp;
              memset(&const_disp, 0, sizeof(const_disp));
              const_disp.X_op = O_constant;
              const_disp.X_add_number = op->X_add_number;
              op->X_add_symbol = make_expr_symbol(&const_disp);
              op->X_add_number = rnum;
              op->X_op_symbol = 0;
              op->X_op = O_md1;
            }
        }
      break;

    case O_register:
		if ( op->X_md && ((REG_IX == op->X_add_number)||(REG_IY == op->X_add_number)) )
		{
			op->X_add_symbol = zero;
			op->X_op = O_md1;
		}
	break;
	default:	//SVES added, to avoid compiler warnings
	break;		//SVES added
    }
  return res;
}


/* Condition codes, including some synonyms provided by HiTech zas.  */
static const struct reg_entry cc_tab[] =
{
  { "age", 6 << 3 },
  { "alt", 7 << 3 },
  { "c",   3 << 3 },
  { "di",  4 << 3 },
  { "ei",  5 << 3 },
  { "lge", 2 << 3 },
  { "llt", 3 << 3 },
  { "m",   7 << 3 },
  { "nc",  2 << 3 },
  { "nz",  0 << 3 },
  { "p",   6 << 3 },
  { "pe",  5 << 3 },
  { "po",  4 << 3 },
  { "z",   1 << 3 },
} ;

/* Parse condition code.  */
static const char *
parse_cc (const char *s, char * op)
{
	const char *p;
	int i;
	struct reg_entry * cc_p;
	for (i = 0; i < BUFLEN; ++i)
	{
		if (!ISALPHA (s[i])) /* Condition codes consist of letters only.  */
		break;
		buf[i] = TOLOWER (s[i]);
	}
	if ((i < BUFLEN)
	&& ((s[i] == 0) || (s[i] == ',')))
	{
		buf[i] = 0;
		cc_p = bsearch (&key, cc_tab, ARRAY_SIZE (cc_tab),
		sizeof (cc_tab[0]), key_cmp);
	}
	else
		cc_p = NULL;

	if (cc_p)
	{
		*op = cc_p->number;
		p = s + i;
	}
	else
		p = NULL;

	return p;
}

//*************************************************************************************************************
//emit_retn 	: handles retn Instructions
//parameters 	: Prefix Value,Opcode Value and end of mnemonic.
//Return 	  	: Returns a Pointer pointing to the end of mnemonic to skip spaces thereafter.
//*************************************************************************************************************
//SVES START
static const char *
emit_retn (char prefix, char opcode, const char * args)
{
	
	char *p;
	//SVES START
	int error_value;
	error_value = add_suffix_ret();		//sves added, handles .s and .l suffix
	if(error_value)
		error_report();
	//SVES END
	if (prefix)
	{
		p = frag_more (2);
		*p++ = prefix;
	}
	else
	{
		p = frag_more (1);
	}
*p = opcode;
return args;
}
//SVES END
//*************************************************************************************************************
//emit insn 	: handles Instructions such as Input and Output group, Exchange, Block Transfer, Search Group.
//parameters 	: Prefix Value,Opcode Value and end of mnemonic.
//Return 	  	: Returns a Pointer pointing to the end of mnemonic to skip spaces thereafter.
//*************************************************************************************************************
static const char *
emit_insn (char prefix, char opcode, const char * args)
{
	
	char *p;
	//SVES START
	int error_value;
	error_value = add_suffix();		//sves added, handles .s and .l suffix
	if(error_value)
		error_report();
	//SVES END
	if (prefix)
	{
		p = frag_more (2);
		*p++ = prefix;
	}
	else
	{
		p = frag_more (1);
	}
*p = opcode;
return args;
}

void ez80_cons_fix_new (fragS *frag_p, int offset, int nbytes, expressionS *exp)
{
  bfd_reloc_code_real_type r[4] =
    {
      BFD_RELOC_8,
      BFD_RELOC_16,
      BFD_RELOC_24,
      BFD_RELOC_32
    };

  if (nbytes < 1 || nbytes > 4) 
    {
      as_bad (_("unsupported BFD relocation size %u"), nbytes);
    }
  else
    {
      fix_new_exp (frag_p, offset, nbytes, exp, 0, r[nbytes-1]);
    }
}

static void
emit_byte (expressionS * val, bfd_reloc_code_real_type r_type)
{
  char *p;
  int lo, hi;
  fixS * fixp;
 
  p = frag_more (1);
  *p = val->X_add_number;
  if ( contains_register(val->X_add_symbol) || contains_register(val->X_op_symbol) )
    {
      ill_op();
    }
  else if ((r_type == BFD_RELOC_8_PCREL) && (val->X_op == O_constant))
    {
      as_bad (_("cannot make a relative jump to an absolute location"));
    }
  else if (val->X_op == O_constant)
    {
      lo = -128;
      hi = (BFD_RELOC_8 == r_type) ? 255 : 127;

      if ((val->X_add_number < lo) || (val->X_add_number > hi))
	{
	  if (r_type == BFD_RELOC_Z80_DISP8)
	    as_bad (_("offset too large"));
	  else
	    as_warn (_("overflow"));
	}
    }
  else
    {
      fixp = fix_new_exp (frag_now, p - frag_now->fr_literal, 1, val,
			  (r_type == BFD_RELOC_8_PCREL) ? true : false, r_type);
      /* FIXME : Process constant offsets immediately.  */
    }
}
/**
 * @brief Emits a word or 24-bit long immediate/address value.
 *
 * Handles instructions such as Call, Jump, and the Load group. It allocates 
 * either 2 bytes (16-bit) or 3 bytes (24-bit) depending on the suffix or the 
 * active ADL mode, and registers a BFD relocation if the value is not a constant.
 *
 * @param val Pointer to the expression containing the address/immediate value.
 */
static void
emit_word (expressionS * val)
{
char *p;
//p = frag_more (2);	SVES Modified, fragmore argument depends on the suffix format. 
	if (   (val->X_op == O_register)
	|| (val->X_op == O_md1)
	|| contains_register(val->X_add_symbol)
	|| contains_register(val->X_op_symbol) )
		ill_op ();
	else
	{
	//SVES START
	//handles suffix .il, .lil, and .sil
		if((strncmp(".il",suffix,3) == 0) || (strncmp(".lil",suffix,4) == 0) || (strncmp(".sil",suffix,4) == 0))
		{
			p = frag_more (3);
			*p = val->X_add_number;
			p[1] = (val->X_add_number>>8);
			p[2] = (val->X_add_number>>16);
			if (val->X_op != O_constant)
			fix_new_exp (frag_now, p - frag_now->fr_literal, 3,
			val, false, BFD_RELOC_24);
		}
		else if(strncmp(".is",suffix,3) == 0) 
		{
			p = frag_more (2);
			*p = val->X_add_number;
			p[1] = (val->X_add_number>>8);
			if (val->X_op != O_constant)
			fix_new_exp (frag_now, p - frag_now->fr_literal, 2,
			val, false, BFD_RELOC_16);
		}
		else if((strncmp(".sis",suffix,4) == 0) || (strncmp(".lis",suffix,4) == 0)) 
		{
			p = frag_more (2);
			*p = val->X_add_number;
			p[1] = (val->X_add_number>>8);
			if (val->X_op != O_constant)
			fix_new_exp (frag_now, p - frag_now->fr_literal, 2,
			val, false, BFD_RELOC_16);
		}				
		else
		{
			if(adl_mode == 1)
			{
				p = frag_more (3);
				*p = val->X_add_number;
				p[1] = (val->X_add_number>>8);
				p[2] = (val->X_add_number>>16);
				if (val->X_op != O_constant)
				fix_new_exp (frag_now, p - frag_now->fr_literal, 3,
				val, false, BFD_RELOC_24);
			}	
			else if(adl_mode == 0)
			{
				//SVES END
				p = frag_more (2);
				*p = val->X_add_number;
				p[1] = (val->X_add_number>>8);
				if (val->X_op != O_constant)
				fix_new_exp (frag_now, p - frag_now->fr_literal, 2,
				val, false, BFD_RELOC_16);
			}
		}
	}
}

//********************************************************************************************************
//emit_mx			: handles Instructions such as adc,add,dec,inc,sbc,sub group.
//parameters 		: Operand as a string and  op is a pointer to a structure expressionS
//					: op->X_md -> status of legal operand,
//					: op->X_add_number -> to add Operand,op->X_op -> to identify Operand.
//Return 			: none.
//*********************************************************************************************************
static void
emit_mx (char prefix, char opcode, int shift, expressionS * arg)
     /* The operand m may be r, (hl), (ix+d),(iy+d),
	if 0 == prefix m may also be ixl, ixh, iyl, iyh.  */
{
char *q;
int rnum;
int error_value;							//SVES ADDED
rnum = arg->X_add_number;
	switch (arg->X_op)
	{
	case O_register:
		if (arg->X_md)
		{
			if (rnum != REG_HL)
			{
				ill_op ();
				break;
			}
			else
				rnum = 6;
		}
		else
		{
			if ((prefix == 0) && (rnum & R_INDEX))
			{
				prefix = (rnum & R_IX) ? 0xDD : 0xFD;
				check_mach (INS_UNDOC);
				rnum &= ~R_INDEX;
			}
			if (rnum > 7)
			{
			//SVES START	
				if (rnum == REG_MB)         //to support MBASE register for load instruction.
				{
					q = frag_more(2);
					*q++ = 0xED;
					*q = 0x6E;
				}
			//SVES END
				else
					ill_op ();
				break;
			}
		}
		//SVES START
		error_value = add_suffix();		//sves added, handles .s and .l suffix
		if(error_value)
			error_report();
		//SVES END
		q = frag_more (prefix ? 2 : 1);
		if (prefix)
			* q ++ = prefix;
		* q ++ = opcode + (rnum << shift);
		break; 

	case O_md1:
			//SVES START
		error_value = add_suffix();		//sves added, handles .s and .l suffix
		if(error_value)
			error_report();
		//SVES END
		q = frag_more (2);
		*q++ = (rnum & R_IX) ? 0xDD : 0xFD;
		*q = (prefix) ? prefix : (opcode + (6 << shift));
		{
		expressionS offset = *arg;
		offset.X_op = O_symbol;
		offset.X_add_number = 0;
		emit_byte (&offset, BFD_RELOC_Z80_DISP8);
		}
		if (prefix)
		{
			q = frag_more (1);
			*q = opcode+(6<<shift);
		}
	break;
	default:
		abort ();
	}
}

/* The operand m may be r, (hl), (ix+d), (iy+d),
   if 0 = prefix m may also be ixl, ixh, iyl, iyh.  */
static const char *
emit_m (char prefix, char opcode, const char *args)
{
  expressionS arg_m;
  const char *p;

  p = parse_exp (args, &arg_m);
  switch (arg_m.X_op)
    {
    case O_md1:
    case O_register:
      emit_mx (prefix, opcode, 0, &arg_m);
      break;
    default:
      ill_op ();
    }
  return p;
}

/* The operand m may be as above or one of the undocumented
   combinations (ix+d),r and (iy+d),r (if unportable instructions
   are allowed).  */
static const char *
emit_mr (char prefix, char opcode, const char *args)
{
  expressionS arg_m, arg_r;
  const char *p;

  p = parse_exp (args, & arg_m);

  switch (arg_m.X_op)
    {
    case O_md1:
      if (*p == ',')
	{
	  p = parse_exp (p + 1, & arg_r);
	  if ((arg_r.X_md == 0)
	      && (arg_r.X_op == O_register)
	      && (arg_r.X_add_number < 8))
	    opcode += arg_r.X_add_number-6; /* Emit_mx () will add 6.  */
	  else
	    {
	      ill_op ();
	      break;
	    }
	  check_mach (INS_UNPORT);
	}
    case O_register:  
	emit_mx (prefix, opcode, 0, & arg_m);
      break;
    default:
      ill_op ();
    }
  return p;
}


static void
emit_sx (char prefix, char opcode, expressionS * arg_p)
{
  char *q;

  switch (arg_p->X_op)
    {
    case O_register:
    case O_md1:
      emit_mx (prefix, opcode, 0, arg_p);
      break;
    default:
      if (arg_p->X_md)
	ill_op ();
      else
	{
	  q = frag_more (prefix ? 2 : 1);
	  if (prefix)
	    *q++ = prefix;
	  *q = opcode ^ 0x46;
	  emit_byte (arg_p, BFD_RELOC_8);
	}
    }
}


//SVES START
//************************************************************************************************
//emit_test_sx		: handles Instructions such as Test group.
//parameter 		: Prefix Value,Opcode Value and operands.
//Return 			: Return Void.
//************************************************************************************************

static void
emit_test_sx (char prefix, char opcode, expressionS * arg_p)
{
char *q;
int rnum ;
rnum = arg_p->X_add_number;
	switch (arg_p->X_op)
	{ 
		case O_register:
			if (arg_p->X_md)
			{
				if(suffix[1] =='s')
				{
					q = frag_more (3);
					*q++ = 0x52;
					*q++ = prefix;
					*q = 0x34;
				}
				else if(suffix[1] =='l')
				{
					q = frag_more (3);
					*q++ = 0x49;
					*q++ = prefix;
					*q = 0x73;
				}
				else
				{
					q = frag_more (2);
					* q++ = prefix;
					* q++ = 0x34;
				}
			}
			else
			{
				q = frag_more (2);
				if (prefix)
				* q ++ = prefix;
				* q ++ = opcode + (rnum << 3);
			}
		break;

		default:
			if (arg_p->X_md)
				ill_op ();
			else
			{
				q = frag_more (2);
				*q++ = prefix;
				*q = 0x64;
				emit_byte (arg_p, BFD_RELOC_8);
			}
	}
}

//************************************************************************************************
//emit_ts			: handles Instructions such as Test group.
//parameter 		: Prefix Value,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operands to skip spaces thereafter.
//************************************************************************************************
static const char *
emit_ts (char prefix, char opcode, const char *args)
{
  expressionS arg_s;
  const char *p;
  p = parse_exp (args, & arg_s);
  emit_test_sx (prefix, opcode, & arg_s);
  return p;
}

//SVES END

/* The operand s may be r, (hl), (ix+d), (iy+d), n.  */
static const char *
emit_s (char prefix, char opcode, const char *args)
{
  expressionS arg_s;
  const char *p;
  p = parse_exp (args, & arg_s);
  emit_sx (prefix, opcode, & arg_s);
  return p;
}
//***********************************************************************************************
//emit_call			: handles Instructions such as Jump,Call group.
//parameter 		: Prefix ATTRIBUTE_UNUSED,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operand to skip spaces thereafter. 
//************************************************************************************************

static const char *
emit_call (char prefix ATTRIBUTE_UNUSED, char opcode, const char * args)
{
	expressionS addr;
	const char *p;  
	char *q;
	int error_value ;	//SVES ADDED recognises invalid instructions 
	p = parse_exp_not_indexed (args, &addr);
	if (addr.X_md)
		ill_op ();
	else
	{
		//#SVES START
		error_value = add_suffix_call();		//handles suffix .is,.il,.lil and .sis
		if(error_value)
			error_report();
		//#SVES END
		q = frag_more (1);
		*q = opcode;
		emit_word (& addr);
	}
	return p;
}
//***************************************************************************************************
//emit_incdec			: handles Instructions such as INC,DEC group.
//parameter 			: Prefix,Opcode Value and operands.
//Return 				: Returns a Pointer pointing to the end of operand to skip spaces thereafter.
//****************************************************************************************************
/* Operand may be rr, ir, r, (hl), ix/iy, (ix+d), (iy+d), SP.  */ //sves added
static const char *
emit_incdec (char prefix, char opcode, const char * args)
{
	expressionS operand;
	int rnum;
	const char *p;  
	char *q;
	int error_value;			//SVES ADDED recognises invalid instructions
	p = parse_exp (args, &operand);
	rnum = operand.X_add_number;
	if ((! operand.X_md)
	&& (operand.X_op == O_register)
	&& (R_ARITH&rnum))
	{
		//SVES START
		error_value = add_suffix();		//sves added, handles .s and .l suffix
		if(error_value)
			error_report();
		//SVES END
		q = frag_more ((rnum & R_INDEX) ? 2 : 1);
		if (rnum & R_INDEX)
			*q++ = (rnum & R_IX) ? 0xDD : 0xFD;
		*q = prefix + ((rnum & 3) << 4);
	}
	else
	{
		if ((operand.X_op == O_md1) || (operand.X_op == O_register))
			emit_mx (0, opcode, 3, & operand);
		else
			ill_op ();
	}
	return p;
}

static const char *
emit_jr (char prefix ATTRIBUTE_UNUSED, char opcode, const char * args)
{
  expressionS addr;
  const char *p;
  char *q;

  p = parse_exp_not_indexed (args, &addr);
  if (addr.X_md)
    ill_op ();
  else
    {
      q = frag_more (1);
      *q = opcode;
      emit_byte (&addr, BFD_RELOC_8_PCREL);
    }
  return p;
}
//***********************************************************************************************
//emit_jp			: handles Instructions such as Jump, Call group.
//parameter 		: Prefix ,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operand to skip spaces thereafter.
//***********************************************************************************************
static const char *
emit_jp (char prefix, char opcode, const char * args)
{
	expressionS addr;
	const char *p;
	char *q;
	int rnum;
	int error_value;							//SVES ADDED recognises invalid instructions
	p = parse_exp_not_indexed (args, & addr);
	if (addr.X_md)
	{
		rnum = addr.X_add_number;
		if ((O_register == addr.X_op) && (REG_HL == (rnum & ~R_INDEX)))
		{
			//SVES START
			error_value = add_suffix_ld();		//sves added, handles all suffixes
			if(error_value)
			error_report();
			//SVES END
			q = frag_more ((rnum & R_INDEX) ? 2 : 1);
			if (rnum & R_INDEX)
				*q++ = (rnum & R_IX) ? 0xDD : 0xFD;
				*q = prefix;
		}
		else
			ill_op ();
	}
	else
	{		
		//SVES START
		error_value = add_suffix_ld();		//sves added, handles all suffixes
		if(error_value)
			error_report();
		//SVES END
		q = frag_more (1);
		*q = opcode;
		emit_word (& addr);
	}
return p;
}

static const char *
emit_im (char prefix, char opcode, const char * args)
{
  expressionS mode;
  const char *p;
  char *q;

  p = parse_exp (args, & mode);
  if (mode.X_md || (mode.X_op != O_constant))
    ill_op ();
  else
    switch (mode.X_add_number)
      {
      case 1:
      case 2:
	++mode.X_add_number;
	/* Fall through.  */
      case 0:
	q = frag_more (2);
	*q++ = prefix;
	*q = opcode + 8*mode.X_add_number;
	break;
      default:
	ill_op ();
      }
  return p;
}
//************************************************************************************************
//emit_pop			: handles Instructions such as push and pop group.
//parameter 		: Prefix ATTRIBUTE_UNUSED,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operands to skip spaces thereafter.
//************************************************************************************************
static const char *
emit_pop (char prefix ATTRIBUTE_UNUSED, char opcode, const char * args)
{
	expressionS regp;
	const char *p;
	char *q;
	int error_value;     // sves added ,recognizes invalid instructions

	p = parse_exp (args, & regp);
	if ((!regp.X_md)
	&& (regp.X_op == O_register)
	&& (regp.X_add_number & R_STACKABLE))
	{
		int rnum;
		rnum = regp.X_add_number;
		if (rnum&R_INDEX)
		{
		//SVES START
		error_value = add_suffix();		//sves added, handles .s and .l suffix
		if(error_value)
			error_report();
		//SVES END
		//	add_suffix();	//sves added,handles suffixes .s and .l
			q = frag_more (2);
			*q++ = (rnum&R_IX)?0xDD:0xFD;
		}
		else
		{
			//SVES START
			error_value = add_suffix();		//sves added, handles .s and .l suffix
			if(error_value)
				error_report();
			//SVES END
			//add_suffix();	//sves added,handles suffixes .s and .l
			q = frag_more (1);
		}
		*q = opcode + ((rnum & 3) << 4);
	}
	else
		ill_op ();
	return p;
}

//************************************************************************************************
//emit_retcc		: handles return Instructions 
//parameter 		: Prefix ATTRIBUTE_UNUSED,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operands to skip spaces thereafter.
//************************************************************************************************
static const char *
emit_retcc (char prefix ATTRIBUTE_UNUSED, char opcode, const char * args)
{
	char cc, *q;
	const char *p;
	int error_value;		// sves added ,recognizes invalid instructions
	p = parse_cc (args, &cc);
	//SVES START
	error_value =  add_suffix_ret();		
	if(error_value)
		error_report();
	//SVES END
	q = frag_more (1);

	if (p)
	{
		*q = opcode + cc;	
	}
	else
	{
		*q = prefix;
	}
	return p ? p : args;
}

//SVES START
//************************************************************************************************
//emit_lea			: handles Instructions such as Load Effective Address group.
//parameter 		: Prefix Value,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operands to skip spaces thereafter.
//************************************************************************************************

static const char *
emit_lea (char prefix, char opcode, const char * args)
{
	char *q = NULL;
	expressionS term;
	const char *p;	
	unsigned int number;
	int error_value;		//recognizes invalid instructions
	opcode = 0;	
	p = parse_exp (args, &term);
	if (*p++ != ',')
    {
		error (_("bad instruction syntax"));
		return p;
    }
		error_value = add_suffix();		//sves added, handles .s and .l suffix
		if(error_value)
			error_report();
		number = disp_lea(args);
    switch (term.X_add_number)
    {
	case REG_IX:
		if (p[0] == 'i' || p[0] == 'I')
		{
			q = frag_more (3);
			*q++ = prefix;
			switch(p[1])
			{
				case 'x':
				case 'X': *q++ = 0x32; break;
				case 'y':
				case 'Y': *q++ = 0x54; break;
				default : ill_op();	break;
			}
		}
	break;

	case REG_IY:
		if (p[0] == 'i' || p[0] == 'I')
		{
			q = frag_more (3);
			*q++ = prefix;
			switch(p[1])
			{
				case 'x':
				case 'X': *q++ = 0x55; break;
				case 'y':
				case 'Y': *q++ = 0x33; break;
				default : ill_op();	break;
			}
		}
	break;

	case REG_BC:
		if (p[0] == 'i' || p[0] == 'I')
		{
			q = frag_more (3);
			*q++ = prefix;
			switch(p[1])
			{
				case 'x':
				case 'X': *q++ = 0x02; break;
				case 'y':
				case 'Y': *q++ = 0x03; break;
				default : ill_op();	break;
			}
		}
	break;

	case REG_DE:
		if (p[0] == 'i' || p[0] == 'I')
		{
			q = frag_more (3);
			*q++ = prefix;
			switch(p[1])
			{
				case 'x':
				case 'X': *q++ = 0x12; break;
				case 'y':
				case 'Y': *q++ = 0x13; break;
				default : ill_op();	break;
			}
		}
	break;
	case REG_HL:
		if (p[0] == 'i' || p[0] == 'I')
		{
			q = frag_more (3);
			*q++ = prefix;
			switch(p[1])
			{
				case 'x':
				case 'X': *q++ = 0x22; break;
				case 'y':
				case 'Y': *q++ = 0x23; break;
				default : ill_op();	break;
			}
		}
	break;
	/* Fall through.  */
	default:
		ill_op ();
    }	
		*q = number ;
  return args;
}

//************************************************************************************************
//emit_pea			: handles Instructions such as Push Effective Address group.
//parameter 		: Prefix Value,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operands to skip spaces thereafter.
//************************************************************************************************

static const char *
emit_pea (char prefix, char opcode, const char * args)
{
	const char *p;
	char *q;
	int error_value;					//SVES START recognizes invalid instructions
	unsigned int number = 0;
	p = args;
	if (p[0] == 'i' || p[0] == 'I')
	{
		error_value = add_suffix();		//sves added, handles .s and .l suffix
		if(error_value)
			error_report();
		q = frag_more (3);
		*q++ = prefix;
		switch(p[1])
		{
			case 'x':
			case 'X': *q++ = opcode; break;
			case 'y':
			case 'Y': *q++ = opcode + 1; break;
			default : ill_op();	break;
		}
		number = disp_lea(args);		
		*q = number ;
	}
	return args;
	}

//************************************************************************************************
//emit_mlt			: handles Instructions such as Multiplication group.
//parameter 		: Prefix Value,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operands to skip spaces thereafter.
//************************************************************************************************

static const char *
emit_mlt (char prefix, char opcode, const char * args)
{
expressionS term;
const char *p;
char *q;
int error_value;  //sves added,recognizes invalid instructions
p = parse_exp (args, &term);
	if ((term.X_md) || (term.X_op != O_register))
		ill_op ();
	else
		switch (term.X_add_number)
		{
		case REG_SP:
			//SVES START
			error_value = add_suffix();		//sves added, handles .s and .l suffix
			if(error_value)
				error_report();
			//SVES END
			//add_suffix();
			q = frag_more (2);
			*q++ = prefix;
			*q = opcode + 0x30;
			break;
		case REG_BC:
			if(suffix[0] == '.')
				ill_op ();
			q = frag_more (2);
			*q++ = prefix;
			*q = opcode;
			break;

		case REG_DE:
			if(suffix[0] == '.')
				ill_op ();
			q = frag_more (2);
			*q++ = prefix;
			*q = opcode + 0x10;
			break;

		case REG_HL:
			if(suffix[0] == '.')
				ill_op ();
			q = frag_more (2);
			*q++ = prefix;
			*q = opcode + 0x20;
			break;
/* Fall through.  */
		default:
			ill_op ();
		}
return p;
}

//************************************************************************************************
//emit_test_io		: handles Instructions such as Test I/O Byte group.
//parameter 		: Prefix Value,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operands to skip spaces thereafter.
//************************************************************************************************
static const char *
emit_test_io (char prefix, char opcode, const char * args)
{
	char *q;
	unsigned int number = 0;
	char array[5] = { 0 } ;
	unsigned int c = 0;
	int i = 0, flag = 0, hexa_flag = 0;
	while(*args)
	{
		c = *args & CHAR_MASK;
		if (*args =='h' || *args =='H')
		{
			args++;
			hexa_flag = 1;
			continue;
		}
		else if (ISXDIGIT(c))
		{
			array[i++] = c ;
			flag = 1; 
		}
		++args;
	}
	array[i] = 0;
	q = frag_more (3);
	*q++ = prefix;
	*q++ = opcode;
	if(hexa_flag == 1  )
		sscanf(array,"%x",&number);
	else if(flag == 1)
		sscanf(array,"%d",&number);
	*q = number;
	return args;
}

//************************************************************************************************
//emit_test			: handles Instructions such as Test group.
//parameter 		: Prefix Value,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of operands to skip spaces thereafter.
//************************************************************************************************

static const char *
emit_test (char prefix, char opcode, const char * args)
{
	expressionS term;
	const char *p;
	p = parse_exp (args, &term);
	if (*p++ != ',')
    {
		error (_("bad instruction syntax"));
		return p;
    }

	if ((term.X_md) || (term.X_op != O_register))
	{
		ill_op ();
	}
	else
    switch (term.X_add_number)
    {
	case REG_A:
		p = emit_ts (prefix, opcode, p);
		break;
	default:
		ill_op ();
    }
return p;
}
//SVES END

//***************************************************************************************************
//emit_adc			: handles Instructions such as adc,sbc group.
//parameter 		: Prefix Value,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of the operand to skip spaces thereafter.
//***************************************************************************************************
static const char *
emit_adc (char prefix, char opcode, const char * args)
{
	expressionS term;
	int rnum;
	const char *p;
	char *q;
	int error_value; 						//sves added recognizes invalid instructions
	p = parse_exp (args, &term);
	if (*p++ != ',')
    {
		error (_("bad instruction syntax"));
		return p;
    }
	if ((term.X_md) || (term.X_op != O_register))
		ill_op ();
	else
    switch (term.X_add_number)
    {
    case REG_A:
		p = emit_s (0, prefix, p);
		break;
    case REG_HL:
		p = parse_exp (p, &term);
		if ((!term.X_md) && (term.X_op == O_register))
		{
			rnum = term.X_add_number;
			if (R_ARITH == (rnum & (R_ARITH | R_INDEX)))
			{
			//SVES START
			error_value = add_suffix();				//sves added, handles .s and .l suffix
			if(error_value)
				error_report();
			//SVES END
				q = frag_more (2);
				*q++ = 0xED;
				*q = opcode + ((rnum & 3) << 4);
				break;
			}
		}

	/* Fall through.  */
	default:
		ill_op ();
      }
  return p;
}

/*SVES START*/
//**************************************************************************************************
//emit_logic		: handles Logical Instructions such as AND,CP,OR,XOR.
//Function Input 	: Prefix Value,Opcode Value and operands.
//Function Return 	: Returns a Pointer pointing to the end of the operand to skip spaces thereafter.
//***************************************************************************************************
static const char *
emit_logic (char prefix, char opcode, const char * args)
{
	expressionS term;
	const char *p;
	opcode =0;
	p = parse_exp (args, &term);
	if (*p++ != ',')
    {
		error (_("bad instruction syntax"));
		return p;
    }

	if ((term.X_md) || (term.X_op != O_register))
		ill_op ();
	else
    switch (term.X_add_number)
    {
    case REG_A:
		p = emit_s (0, prefix, p);
		break;
   
	/* Fall through.  */
    default:
		ill_op ();
    }
  return p;
}
//***************************************************************************************************
//emit_add			: handles Instructions such as add group.
//parameter 		: Prefix ,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of the operand to skip spaces thereafter.
//****************************************************************************************************
/*SVES END*/

static const char *
emit_add (char prefix, char opcode, const char * args)
{
	expressionS term;
	int lhs, rhs;
	const char *p;
	char *q;
	int error_value;	//sves added recognizes invalid instructions
	p = parse_exp (args, &term);
	if (*p++ != ',')
	{
		error (_("bad instruction syntax"));
		return p;
	}
	if ((term.X_md) || (term.X_op != O_register))
		ill_op ();
	else
		switch (term.X_add_number & ~R_INDEX)
		{
		case REG_A:
			p = emit_s (0, prefix, p);
			break;

		case REG_HL:
			lhs = term.X_add_number;
			p = parse_exp (p, &term);
			if ((!term.X_md) && (term.X_op == O_register))
			{
				rhs = term.X_add_number;
				if ((rhs & R_ARITH)
				&& ((rhs == lhs) || ((rhs & ~R_INDEX) != REG_HL)))
				{
					//SVES START
					error_value = add_suffix();		//sves added, handles .s and .l suffix
					if(error_value)
						error_report();
					//SVES END
					//add_suffix();							//sves added
					q = frag_more ((lhs & R_INDEX) ? 2 : 1);
					if (lhs & R_INDEX)
						*q++ = (lhs & R_IX) ? 0xDD : 0xFD;
						*q = opcode + ((rhs & 3) << 4);
						break;
				}
			}
/* Fall through.  */
		default:
			ill_op ();
		}
return p;
}


static const char *
emit_bit (char prefix, char opcode, const char * args)
{
  expressionS b;
  int bn;
  const char *p;

  p = parse_exp (args, &b);
  if (*p++ != ',')
    error (_("bad instruction syntax"));

  bn = b.X_add_number;
  if ((!b.X_md)
      && (b.X_op == O_constant)
      && (0 <= bn)
      && (bn < 8))
    {
      if (opcode == 0x40)
	/* Bit : no optional third operand.  */
	p = emit_m (prefix, opcode + (bn << 3), p);
      else
	/* Set, res : resulting byte can be copied to register.  */
	p = emit_mr (prefix, opcode + (bn << 3), p);
    }
  else
    ill_op ();
  return p;
}

static const char *
emit_jpcc (char prefix, char opcode, const char * args)
{
	char cc;
	const char *p;
	p = parse_cc (args, & cc);
	if (p && *p++ == ',')
	{
		p = emit_call (0, opcode + cc, p);
	}
	else
	{
		p = (prefix == (char)0xC3)
		? emit_jp (0xE9, prefix, args)
		: emit_call (0, prefix, args);
	}
	return p;
}

static const char *
emit_jrcc (char prefix, char opcode, const char * args)
{
  char cc;
  const char *p;

  p = parse_cc (args, &cc);
  if (p && *p++ == ',')
    {
      if (cc > (3 << 3))
	error (_("condition code invalid for jr"));
      else
	p = emit_jr (0, opcode + cc, p);
    }
  else
    p = emit_jr (0, prefix, args);

  return p;
}

static const char *
emit_ex (char prefix_in ATTRIBUTE_UNUSED,
	 char opcode_in ATTRIBUTE_UNUSED, const char * args)
{
  expressionS op;
  const char * p;
  char prefix, opcode;

  p = parse_exp_not_indexed (args, &op);
  if (op.X_op == O_register && (*p == '\'' || *p == '`'))
    p++;
  p = skip_space (p);
  if (*p++ != ',')
    {
      error (_("bad instruction syntax"));
      return p;
    }

  prefix = opcode = 0;
  if (op.X_op == O_register)
    switch (op.X_add_number | (op.X_md ? 0x8000 : 0))
      {
      case REG_AF:
	if (TOLOWER (*p++) == 'a' && TOLOWER (*p++) == 'f')
	  {
	    /* The scrubber changes '\'' to '`' in this context.  */
	    if (*p == '`' || *p == '\'')
	      ++p;
	    opcode = 0x08;
	  }
	break;
      case REG_DE:
	if (TOLOWER (*p++) == 'h' && TOLOWER (*p++) == 'l')
	  {
	    if (*p == '`' || *p == '\'')
	      ++p;
	    opcode = 0xEB;
	  }
	break;
      case REG_SP|0x8000:
	p = parse_exp (p, & op);
	if (op.X_op == O_register && (*p == '\'' || *p == '`'))
	  p++;
	if (op.X_op == O_register
	    && op.X_md == 0
	    && (op.X_add_number & ~R_INDEX) == REG_HL)
	  {
	    opcode = 0xE3;
	    if (R_INDEX & op.X_add_number)
	      prefix = (R_IX & op.X_add_number) ? 0xDD : 0xFD;
	  }
	break;
      }
  if (opcode)
    emit_insn (prefix, opcode, p);
  else
    ill_op ();

  return p;
}

//SVES START

//***************************************************************************************************
//emit_in0			: handles Instructions such as IN0 group.
//parameter 		: Prefix ATTRIBUTE_UNUSED,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of the operand to skip spaces thereafter.
//****************************************************************************************************

static const char *
emit_in0 (char prefix ATTRIBUTE_UNUSED, char opcode ATTRIBUTE_UNUSED,
	const char * args)
{
  expressionS reg, port;
  const char *p;
  char *q;

  p = parse_exp (args, &reg);
  if (*p++ != ',')
    {
      error (_("bad instruction syntax"));
      return p;
    }
  p = parse_exp (p, &port);
  if (reg.X_md == 0
      && reg.X_op == O_register
      && (reg.X_add_number <= 7 || reg.X_add_number == REG_F)
      && (port.X_md))
    {
      if (port.X_op != O_md1 && port.X_op != O_register)
	{
		  q = frag_more (2);
		  *q++ = 0xED;
		  *q =((reg.X_add_number&7)<<3);
	      emit_byte (&port, BFD_RELOC_8);
	}
	 
	}
  else
	    ill_op ();
  return p;
}
//SVES END

//***************************************************************************************************
//emit_in			: handles Instructions such as IN group.
//parameter 		: Prefix ATTRIBUTE_UNUSED,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of the operand to skip spaces thereafter.
//****************************************************************************************************
static const char *
emit_in (char prefix ATTRIBUTE_UNUSED, char opcode ATTRIBUTE_UNUSED,
	const char * args)
{
  expressionS reg, port;
  const char *p;
  char *q;

  p = parse_exp (args, &reg);
  if (*p++ != ',')
    {
      error (_("bad instruction syntax"));
      return p;
    }
  p = parse_exp (p, &port);
  if (reg.X_md == 0
      && reg.X_op == O_register
      && (reg.X_add_number <= 7 || reg.X_add_number == REG_F)
      && (port.X_md))
    {
      if (port.X_op != O_md1 && port.X_op != O_register)
	{
	  if (REG_A == reg.X_add_number)
	    {
	      q = frag_more (1);
	      *q = 0xDB;
	      emit_byte (&port, BFD_RELOC_8);
	    }
	  else
	    ill_op ();
	}
      else
	{
		if (port.X_add_number == REG_C)
	    {
	      if (reg.X_add_number == REG_F)
		check_mach (INS_UNDOC);
	      else
		{
		  q = frag_more (2);
		  *q++ = 0xEB;
		  *q = 0x40|((reg.X_add_number&7)<<3);
		}
	    }								//SVES START  ,to support IN r,(BC) type of instruction
	  else if (port.X_add_number == REG_BC)
	    {
	      if (reg.X_add_number == REG_F)
		check_mach (INS_UNDOC);
	      else
		{
		  q = frag_more (2);
		  *q++ = 0xED;
		  *q = 0x40|((reg.X_add_number&7)<<3);
		}
	    }								//SVES END
	  else
	    ill_op ();
	}
    }
  else
    ill_op ();
  return p;
}

//***************************************************************************************************
//emit_out			: handles Instructions such as OUT group.
//parameter 		: Prefix ATTRIBUTE_UNUSED,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of the operand to skip spaces thereafter.
//****************************************************************************************************
static const char *
emit_out (char prefix ATTRIBUTE_UNUSED, char opcode ATTRIBUTE_UNUSED,
	 const char * args)
{
  expressionS reg, port;
  const char *p;
  char *q;

  p = parse_exp (args, & port);
  if (*p++ != ',')
    {
      error (_("bad instruction syntax"));
      return p;
    }
  p = parse_exp (p, &reg);
  if (!port.X_md)
    { ill_op (); return p; }
  /* Allow "out (c), 0" as unportable instruction.  */
  if (reg.X_op == O_constant && reg.X_add_number == 0)
    {
      check_mach (INS_UNPORT);
      reg.X_op = O_register;
      reg.X_add_number = 6;
    }
  if (reg.X_md
      || reg.X_op != O_register
      || reg.X_add_number > 7)
    ill_op ();
  else
    if (port.X_op != O_register && port.X_op != O_md1)
      {
	if (REG_A == reg.X_add_number)
	  {
	    q = frag_more (1);
	    *q = 0xD3;
	    emit_byte (&port, BFD_RELOC_8);
	  }
	else
	  ill_op ();
      }
    else
      {
	if (REG_C == port.X_add_number)
	  {
	    q = frag_more (2);
	    *q++ = 0xED;
	    *q = 0x41 | (reg.X_add_number << 3);
	  }										//SVES START ,to support OUT (BC),r type of instruction
	else if (REG_BC == port.X_add_number)
	  {
	    q = frag_more (2);
	    *q++ = 0xED;
	    *q = 0x41 | (reg.X_add_number << 3);
	  }										//SVES END
	else
	  ill_op ();
      }
  return p;
}


//***************************************************************************************************
//emit_out0			: handles Instructions such as OUT0 group.
//parameter 		: Prefix ATTRIBUTE_UNUSED,Opcode Value and operands.
//Return 			: Returns a Pointer pointing to the end of the operand to skip spaces thereafter.
//****************************************************************************************************
//SVES START
static const char *
emit_out0 (char prefix ATTRIBUTE_UNUSED, char opcode ATTRIBUTE_UNUSED,
	 const char * args)
{
  expressionS reg, port;
  const char *p;
  char *q;

  p = parse_exp (args, & port);
  if (*p++ != ',')
    {
      error (_("bad instruction syntax"));
      return p;
    }
  p = parse_exp (p, &reg);
  if (!port.X_md)
    { 
	ill_op (); 
	return p; 
	}
  if (reg.X_op == O_constant && reg.X_add_number == 0)
    {
      check_mach (INS_UNPORT);
      reg.X_op = O_register;
      reg.X_add_number = 6;
    }
  if (reg.X_md
      || reg.X_op != O_register
      || reg.X_add_number > 7)
    ill_op ();
  else
    if (port.X_op != O_register && port.X_op != O_md1)
      {
	    q = frag_more (2);
	   *q++ = 0xED;
	    *q = 0x01 | (reg.X_add_number << 3);
	    emit_byte (&port, BFD_RELOC_8);
	  }
	else
	  ill_op ();
      
  return p;
}
//SVES END


//************************************************************************************************
//emit_rst			: handles Instructions such as reset.
//parameter 		: Prefix ATTRIBUTE_UNUSED,Opcode Value and Operands.
//Return 			: Returns a Pointer pointing to the end of Operand to skip spaces thereafter.
//*************************************************************************************************
static const char *
emit_rst (char prefix ATTRIBUTE_UNUSED, char opcode, const char * args)
{
	expressionS addr;
	const char *p;
	char *q;
	int error_value;   //sves added ,recognizes invalid instruction
	p = parse_exp_not_indexed (args, &addr);
	if (addr.X_op != O_constant)
	{
		error ("rst needs constant address");
		return p;
	}

	if (addr.X_add_number & ~(7 << 3))
		ill_op ();
	else
	{
		//SVES START
		error_value = add_suffix();		//sves added, handles .s and .l suffix
		if(error_value)
			error_report();
		//SVES END
		//add_suffix();			//sves added, handles .s and .l suffixes
		q = frag_more (1);
		*q = opcode + (addr.X_add_number & (7 << 3));
	}
	return p;
}


//************************************************************************************************
//emit_ldxhl		: handles Instructions such LDI.
//parameter 		: Prefix Value,Opcode Value , Operands,args as argument string.
//					:  src and d is a pointer to a structure expressionS
//Return 			: Returns a Pointer pointing to the end of Operand to skip spaces thereafter.
//*************************************************************************************************

static void
emit_ldxhl (char prefix, char opcode, expressionS *src, expressionS *d,const char * args)		//sves modified
{
	char *q;
	unsigned int x = 0;
	int error_value;							//sves added ,recognizes invalid instruction
	error_value = 0;
	char disp_flag = 1;
	
	if (src->X_md)
		ill_op ();
	else
	{
		//SVES START ,supports Load Indirect with Offset
		error_value = add_suffix_ld();
		if(error_value)
			error_report();			
		if ( src->X_op == O_constant )
		{
			if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
					&& ((args[2] == 'x') ||(args[2] == 'X')))
			{
				x = displacement(args);
				q = frag_more (4);
				*q++ = 0xDD;
				*q++ = 0x36;
				*q++ = x;
				*q	 = src->X_add_number;
				disp_flag = 0;
			}
			else if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
					&& ((args[2] == 'y') ||(args[2] == 'Y')))
			{
				x = displacement(args);
				q = frag_more (4);
				*q++ = 0xFD;
				*q++ = 0x36;
				*q++ = x;
				*q++ = src->X_add_number;
				disp_flag = 0;
			}
			else
			{
				q = frag_more (2);
				*q++ = 0x36;
				*q	 = src->X_add_number;
				disp_flag = 0;
			}
		}
		else if( src->X_op == O_register					
		&& (src->X_add_number == REG_IX ))
		{
			if (prefix)
			{
				if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
				&& ((args[2] == 'x') ||(args[2] == 'X')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xDD;
					*q++ = 0x3F;
					*q	 = x;
					disp_flag = 0;

				}
				else if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
				&& ((args[2] == 'y') ||(args[2] == 'Y')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xFD;
					*q++ = 0x3E;
					*q	 =	x ;
					disp_flag = 0;
				}
				else
				{
					q = frag_more (2);
					*q++ = 0xDD;
					*q	 = 0x3F;
				}
			}
			else
			{
				q = frag_more (2);
				*q++ = 0xED;
				*q	 = 0x3F;
			}
		}
		else if( src->X_op == O_register
		&& (src->X_add_number == REG_IY ))
		{
			if (prefix)
			{
				if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
				&& ((args[2] == 'x') ||(args[2] == 'X')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xDD;
					*q++ = 0x3E;
					*q	 = x ;
					disp_flag = 0;
				}
				else if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
				&& ((args[2] == 'y') ||(args[2] == 'Y')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xFD;
					*q++ = 0x3F;
					*q	 = x ;
					disp_flag = 0;
				}
				else
				{
					q = frag_more (2);
					*q++ = 0xFD;
					*q	 = 0x3E;
				}
			}
			else
			{
				q = frag_more (2);
				*q++ = 0xED;
				*q	 = 0x3E;
			}
		}
		else if( src->X_op == O_register
		&& (src->X_add_number == REG_BC ))
		{
			if (prefix)
			{
				if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
				&& ((args[2] == 'x') ||(args[2] == 'X')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xDD;
					*q++ = 0x0F;
					*q = x;
					disp_flag = 0;
				}
				else if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
				&& ((args[2] == 'y') ||(args[2] == 'Y')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xFD;
					*q++ = 0x0F;
					*q = x;
					disp_flag = 0;
				}
			}
			else
			{
				q = frag_more (2);
				*q++ = 0xED;
				*q	 = 0x0F;
			}
		}
		else if( src->X_op == O_register
		&& (src->X_add_number == REG_DE ))
		{
			if (prefix)
			{
					if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
					&& ((args[2] == 'x') ||(args[2] == 'X')))
					{
						x = displacement(args);
						q = frag_more (3);
						*q++ = 0xDD;
						*q++ = 0x1F;
						*q	 = x;
						disp_flag = 0;
					}
					else if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
					&& ((args[2] == 'y') ||(args[2] == 'Y')))
					{
						x = displacement(args);
						q = frag_more (3);
						*q++ = 0xFD;
						*q++ = 0x1F;
						*q	 = x;
						disp_flag = 0;
					}
			}
			else
			{
				q = frag_more (2);
				*q++ = 0xED;
				*q	 = 0x1F;
			}
		}
		else if( src->X_op == O_register
		&& (src->X_add_number == REG_HL ))
		{
			if (prefix)
			{
				if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
				&& ((args[2] == 'x') ||(args[2] == 'X')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xDD;
					*q++ = 0x2F;
					*q	 = x;
					disp_flag = 0;

				}
				else if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
				&& ((args[2] == 'y') ||(args[2] == 'Y')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xFD;
					*q++ = 0x2F;
					*q	 = x;
					disp_flag = 0;
				}
			}
			else 
			{
				q = frag_more (2);
				*q++ = 0xED;
				*q	 = 0x2F;
			}
		}
		else if (src->X_op == O_register)
		{
			if (src->X_add_number>7)
			{
				ill_op ();
			}
			if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
					&& ((args[2] == 'x') ||(args[2] == 'X')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xDD;
					*q++ = opcode + src->X_add_number;
					*q	 = x;
					disp_flag = 0;
				}
				else if ((args[0] == '(') && ((args[1] == 'i') ||(args[1] == 'I') )
					&& ((args[2] == 'y') ||(args[2] == 'Y')))
				{
					x = displacement(args);
					q = frag_more (3);
					*q++ = 0xFD;
					*q++ = opcode + src->X_add_number;
					*q	 = x ;
					disp_flag = 0;

				}
				//SVES END
				else if (prefix)
				{
					q = frag_more (2);
					*q++ = prefix;
				}
				else
				{
					q = frag_more (1);
					if (disp_flag == 0)								//sves added
					{
						*q = opcode + src->X_add_number;
					}
				}
			if (d)
			{
				if (disp_flag == 0)									//sves added
					emit_byte (d, BFD_RELOC_Z80_DISP8);
			}
		}
		else
		{
			if (prefix)
			{
				q = frag_more (2);
				*q++ = prefix;
			}
			else
			{
				q = frag_more (1);
			}
			*q = opcode^0x46;
			if (d)
			{
				if (disp_flag == 0)										//sves added
					emit_byte (d, BFD_RELOC_Z80_DISP8);
			}
			if (disp_flag == 0)											//sves added
				emit_byte (src, BFD_RELOC_8);
		}
	}
}


//************************************************************************************************
//emit_ldreg		: handles Instructions such LDI.
//parameter 		: dest holding Register Value ,args as argument string,
//					: src is a pointer to a structure expressionS.
//Return 			: Returns a Pointer pointing to the end of Operand to skip spaces thereafter.
//*************************************************************************************************
static void
emit_ldreg (int dest, expressionS * src, const char * args)
{
	char *q;
	int rnum;
	//SVES START			//sves added ,recognizes invalid instruction
	unsigned int x = 0;
	int error_value;
	char disp_flag = 0;
	error_value = add_suffix_ld();
	if(error_value)
		error_report();	
	//SVES END
	switch (dest)
	{
		/* 8 Bit ld group:  */
	case REG_I:
	case REG_R:
		if (src->X_md == 0 && src->X_op == O_register && src->X_add_number == REG_A)
		{
			q = frag_more (2);
			*q++ = 0xED;
			*q = (dest == REG_I) ? 0x47 : 0x4F;
		}
		//SVES START ,supports Load Interrupt Vector
		else if (src->X_md == 0 && src->X_op == O_register && src->X_add_number == REG_HL)
		{
			q = frag_more (2);
			*q++ = 0xED;
			*q = 0xC7;
		}
		//SVES END
		else
			ill_op ();
	break;

	case REG_A:
		if ((src->X_md) && src->X_op != O_register && src->X_op != O_md1)
		{
			q = frag_more (1);
			*q = 0x3A;
			emit_word (src);
			break;
		}

		if ((src->X_md)
		&& src->X_op == O_register
		&& (src->X_add_number == REG_BC || src->X_add_number == REG_DE))
		{
			q = frag_more (1);
			*q = 0x0A + ((src->X_add_number & 1) << 4);
			break;
		}

		if ((!src->X_md)
		&& src->X_op == O_register
		&& (src->X_add_number == REG_R || src->X_add_number == REG_I))
		{
			q = frag_more (2);
			*q++ = 0xED;
			*q = (src->X_add_number == REG_I) ? 0x57 : 0x5F;
			break;
		}

		/* Fall through.  */
		case REG_B:
		case REG_C:
		case REG_D:
		case REG_E:
			check_suffix = 1;				//SVES ADDED to avoid checking for suffixes more than once. 	
			emit_sx (0, 0x40 + (dest << 3), src);
			break;

		case REG_H:
		case REG_L:
			if ((src->X_md == 0)
			&& (src->X_op == O_register)
			&& (src->X_add_number & R_INDEX))
				ill_op ();
			else
			{											
				check_suffix = 1;			//SVES ADDED to avoid checking for suffixes more than once. 	
				emit_sx (0, 0x40 + (dest << 3), src);
			}
			break;

		case R_IX | REG_H:
		case R_IX | REG_L:
		case R_IY | REG_H:
		case R_IY | REG_L:
			if (src->X_md)
			{
				ill_op ();
				break;
			}
			check_mach (INS_UNDOC);
			if (src-> X_op == O_register)
			{
				rnum = src->X_add_number;
				if ((rnum & ~R_INDEX) < 8
				&& ((rnum & R_INDEX) == (dest & R_INDEX)
				|| (   (rnum & ~R_INDEX) != REG_H
				&& (rnum & ~R_INDEX) != REG_L)))
				{
					q = frag_more (2);
					*q++ = (dest & R_IX) ? 0xDD : 0xFD;
					*q = 0x40 + ((dest & 0x07) << 3) + (rnum & 7);
				}
				else
					ill_op ();
			}
			else
			{
				q = frag_more (2);
				*q++ = (dest & R_IX) ? 0xDD : 0xFD;
				*q = 0x06 + ((dest & 0x07) << 3);
				emit_byte (src, BFD_RELOC_8);
			}
			break;

/* 16 Bit ld group:  */
		case REG_SP:
		if (src->X_md == 0
		&& src->X_op == O_register
		&& REG_HL == (src->X_add_number &~ R_INDEX))
		{
			q = frag_more ((src->X_add_number & R_INDEX) ? 2 : 1);
			if (src->X_add_number & R_INDEX)
				*q++ = (src->X_add_number & R_IX) ? 0xDD : 0xFD;
			*q = 0xF9;
			break;
		}
/* Fall through.  */
		case REG_BC:
		//SVES START  ,supports Load Indirect with Offset
		if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
					&& ((args[5] == 'x') ||(args[5] == 'X')))
		{
			x = displacement(args);
			q = frag_more (3);
			*q++ = 0xDD;
			*q++ = 0x07;
			*q   = x;
			disp_flag = 1 ;
		}
		else if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
					&& ((args[5] == 'y') ||(args[5] == 'Y')))
		{
			x = displacement(args);
			q = frag_more (3);
			*q++ = 0xFD;
			*q++ = 0x07;
			*q   = x;
			disp_flag = 1 ;
		}

		if (disp_flag != 0)
			break;
		if(src->X_op == O_register
		&& (src->X_add_number == REG_HL ))
		{
			q = frag_more (2);
			*q++ = 0xED;
			*q = 0x07;
			break;
		}
		//SVES END
		if (src->X_op == O_register || src->X_op == O_md1)
			ill_op ();
		q = frag_more (src->X_md ? 2 : 1);
		if (src->X_md)
		{
			*q++ = 0xED;
			*q = 0x4B + ((dest & 3) << 4);
		}
		else
			*q = 0x01 + ((dest & 3) << 4);
			emit_word (src);
		break;

		case REG_DE:
			//SVES START   ,supports Load Indirect with Offset
			if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
				&& ((args[5] == 'x') ||(args[5] == 'X')))
			{
				x = displacement(args);
				q = frag_more (3);
				*q++ = 0xDD;
				*q++ = 0x17;
				*q 	 = x;
				break;
			}
			else if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
					&& ((args[5] == 'y') ||(args[5] == 'Y')))
			{
				x = displacement(args);
				q = frag_more (3);
				*q++ = 0xFD;
				*q++ = 0x17;
				*q 	 = x;
				break;
			}
			if(src->X_op == O_register
			&& (src->X_add_number == REG_HL ))
			{
				q = frag_more (2);
				*q++ = 0xED;
				*q = 0x17;
				break;
			}
			//SVES END

		if (src->X_op == O_register || src->X_op == O_md1)
			ill_op ();
		q = frag_more (src->X_md ? 2 : 1);
		if (src->X_md)
		{
			*q++ = 0xED;
			*q = 0x4B + ((dest & 3) << 4);
		}
		else
			*q = 0x01 + ((dest & 3) << 4);
		emit_word (src);
		break;

		case REG_HL:
		case REG_HL | R_IX:
		case REG_HL | R_IY:
		
		//SVES START    ,supports Load Indirect with Offset
		if ((args[0] == 'h' || args[0] == 'H') &&  (args[1] == 'l' || args[1] == 'L') )
		{
			if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
				&& ((args[5] == 'x') ||(args[5] == 'X')))
			{
				x = displacement(args);
				q = frag_more (3);
				*q++ = 0xDD;
				*q++ = 0x27;
				*q	 = x;
				break;
			}
			else if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
				&& ((args[5] == 'y') ||(args[5] == 'Y')))
			{
				x = displacement(args);
				q = frag_more (3);
				*q++ = 0xFD;
				*q++ = 0x27;
				*q	 = x;
				break;
			}
		}
		if((args[0] == 'i' || args[0] == 'I'  ) && (args[1] == 'x' || args[1] == 'X'  ))
		{
			if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
						&& ((args[5] == 'x') ||(args[5] == 'X')))
			{
				x = displacement(args);
				q = frag_more (3);
				*q++ = 0xDD;
				*q++ = 0x37;
				*q	 = x;
				break;
			}
			else if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
						&& ((args[5] == 'y') ||(args[5] == 'Y')))
			{
				x = displacement(args);
				q = frag_more (3);
				*q++ = 0xFD;
				*q++ = 0x31;
				*q	 = x; 
				break;
			}
		}
		else if((args[0] == 'i' || args[0] == 'I'  ) && (args[1] == 'y' || args[1] == 'Y'  ))
		{
			if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
						&& ((args[5] == 'x') ||(args[5] == 'X')))
			{
				x = displacement(args);
				q = frag_more (3);
				*q++ = 0xDD;
				*q++ = 0x31;
				*q	 = x ;
				break;
			}
			else if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I') )
						&& ((args[5] == 'y') ||(args[5] == 'Y')))
			{
				x = displacement(args);
				q = frag_more (3);
				*q++ = 0xFD;
				*q++ = 0x37;
				*q 	 = x;
				break;
			}
		}

		if ((!src->X_md)
			&& src->X_op == O_register
			&& (src->X_add_number == REG_I))
			{
				q = frag_more (2);
				*q++ = 0xED;
				*q = 0xD7;
				break;
			}
			else if (src->X_op == O_register)
			{
				q = frag_more(2);
				*q++ = 0xED ;
				if (dest & R_INDEX)
					*q = (dest & R_IX) ? 0x37 : 0x31;
				else
					*q	= 0x27;
				break;

			}
			else if (src->X_op == O_md1)
			{
				q = frag_more(2);
				*q++ = (dest & R_IX) ? 0xDD : 0xFD;
				if (dest & R_INDEX)
				*q = (dest & R_IX) ? 0x37 : 0x31;
				if ((args[3] == '(' ) && ((args[4] == 'i') || (args[4] == 'I')))
				{
					if (args[5] == 'x' || args[5] == 'X')
					{
						q = frag_more (1);
						sscanf(&args[7],"%d",&x);
						*q = x ;
					}
				}
				if ((args[3] == '(') && ((args[4] == 'i') ||(args[4] == 'I')))
				{
					if (args[5] == 'y' || args[5] == 'Y')
					{
						q = frag_more (1);
						sscanf(&args[7],"%d",&x);
						*q = x ;
					}
				}
			break;
			}
//SVES END

//sves commented else if (src->X_op == O_register || src->X_op == O_md1)
//sves commented ill_op ();
			else
			{
			q = frag_more ((dest & R_INDEX) ? 2 : 1);
			if (dest & R_INDEX)
				* q ++ = (dest & R_IX) ? 0xDD : 0xFD;
			*q = (src->X_md) ? 0x2A : 0x21;
			emit_word (src);
			}
			break;

		case REG_AF:
		case REG_F:
			ill_op ();
			break;
		//SVES START
		case REG_MB:              //sves added ,to support MBASE register for load instruction
			q = frag_more(2);
			*q++ = 0xED;
			*q = 0x6D;
			break;
		//SVES END
		default:
			abort ();
		}
}


//************************************************************************************************
//emit_ld			: handles Instructions such LDI.
//parameter 		: Operands	
//Return 			: Returns a Pointer pointing to the end of Operand to skip spaces thereafter.
//*************************************************************************************************

static const char *
emit_ld (char prefix_in ATTRIBUTE_UNUSED, char opcode_in ATTRIBUTE_UNUSED,
	const char * args)							//SVES Modified
{
	expressionS dst, src;
	const char *p;
	char *q;
	char prefix, opcode;
	int error_value;						//sves added ,recognizes invalid instruction
	p = parse_exp (args, &dst);
	if (*p++ != ',')
		error (_("bad instruction syntax"));
	p = parse_exp (p, &src);
	
	switch (dst.X_op)
	{
		case O_md1:
		{
			expressionS dst_offset = dst;
			dst_offset.X_op = O_symbol;
			dst_offset.X_add_number = 0;
			emit_ldxhl ((dst.X_add_number & R_IX) ? 0xDD : 0xFD, 0x70,
			&src, &dst_offset,args);			//SVES Modified
		}
		break;

		case O_register:
		if (dst.X_md)
		{
			switch (dst.X_add_number)
			{
				case REG_BC:
				case REG_DE:
					//SVES START				
					error_value = add_suffix();		
					if(error_value)					//recognizes invalid instruction
						error_report();				
					//SVES END	
					if (src.X_md == 0 && src.X_op == O_register && src.X_add_number == REG_A)
					{
						q = frag_more (1);
						*q = 0x02 + ( (dst.X_add_number & 1) << 4);
					}
					else
						ill_op ();
					break;

				case REG_HL: 
					emit_ldxhl (0, 0x70, &src, NULL,args);	//SVES Modified
					break;

				default:
					ill_op ();
			}
		}
		else
		{
			emit_ldreg (dst.X_add_number, &src, args);		//SVES ADDED
		}    
		break;

		default:
			if (src.X_md != 0 || src.X_op != O_register)
				ill_op ();
			prefix = opcode = 0;
			//SVES START				
			error_value = add_suffix_ld();	
			if(error_value)					//sves added ,recognizes invalid instruction
				error_report();				
			//SVES END	
			switch (src.X_add_number)
			{
				case REG_A:
				opcode = 0x32; break;
				case REG_BC: case REG_DE: case REG_SP:
				prefix = 0xED; opcode = 0x43 + ((src.X_add_number&3)<<4); break;
				case REG_HL:
				opcode = 0x22; break;
				case REG_HL|R_IX:
				prefix = 0xDD; opcode = 0x22; break;
				case REG_HL|R_IY:
				prefix = 0xFD; opcode = 0x22; break;
			}
			if (opcode)
			{
				q = frag_more (prefix?2:1);
				if (prefix)
				*q++ = prefix;
				*q = opcode;
				emit_word (&dst);
			}
			else
				ill_op ();
	}
	return p;
}


//SVES START
//************************************************************************************************
//cpu				: support pseudo instructions .CPU .
//parameter 		: none	
//Return 			: none 
//*************************************************************************************************
static void
cpu (int size ATTRIBUTE_UNUSED)
{
    if(strncmp("EZ80",input_line_pointer,4) == 0 )
		cpu_eZ80 = 1;
	else if(strncmp("Z80",input_line_pointer,3) == 0 )
		cpu_eZ80 = 0;
	ignore_rest_of_line ();
}

//************************************************************************************************
//assume			: support pseudo instructions .ASSUME.
//parameter 		: none	
//Return 			: none 
//*************************************************************************************************

static void
assume (int size ATTRIBUTE_UNUSED)
{
	char *p = input_line_pointer;
	while (*p == ' ' || *p == '\t')
		p++;
	if (strncasecmp ("ADL=0", p, 5) == 0)
		adl_mode = 0;
	else
		adl_mode = 1;
	while (*input_line_pointer && *input_line_pointer != '\n' && *input_line_pointer != '\r')
		input_line_pointer++;
}

//SVES END

static void
emit_data (int size ATTRIBUTE_UNUSED)
{
  const char *p, *q;
  char *u, quote;
  int cnt;
  expressionS exp;

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }
  p = skip_space (input_line_pointer);

  do
    {
      if (*p == '\"' || *p == '\'')
	{
	    for (quote = *p, q = ++p, cnt = 0; *p && quote != *p; ++p, ++cnt)
	      ;
	    u = frag_more (cnt);
	    memcpy (u, q, cnt);
	    if (!*p)
	      as_warn (_("unterminated string"));
	    else
	      p = skip_space (p+1);
	}
      else
	{
	  p = parse_exp (p, &exp);
	  if (exp.X_op == O_md1 || exp.X_op == O_register)
	    {
	      ill_op ();
	      break;
	    }
	  if (exp.X_md)
	    as_warn (_("parentheses ignored"));
	  emit_byte (&exp, BFD_RELOC_8);
	  p = skip_space (p);
	}
    }
  while (*p++ == ',') ;
  input_line_pointer = (char *)(p-1);
}

static const char *
emit_mulub (char prefix ATTRIBUTE_UNUSED, char opcode, const char * args)
{
  const char *p;

  p = skip_space (args);
  if (TOLOWER (*p++) != 'a' || *p++ != ',')
    ill_op ();
  else
    {
      char *q, reg;

      reg = TOLOWER (*p++);
      switch (reg)
	{
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	  check_mach (INS_R800);
	  if (!*skip_space (p))
	    {
	      q = frag_more (2);
	      *q++ = prefix;
	      *q = opcode + ((reg - 'b') << 3);
	      break;
	    }
	default:
	  ill_op ();
	}
    }
  return p;
}

static const char *
emit_muluw (char prefix ATTRIBUTE_UNUSED, char opcode, const char * args)
{
  const char *p;

  p = skip_space (args);
  if (TOLOWER (*p++) != 'h' || TOLOWER (*p++) != 'l' || *p++ != ',')
    ill_op ();
  else
    {
      expressionS reg;
      char *q;

      p = parse_exp (p, & reg);

      if ((!reg.X_md) && reg.X_op == O_register)
	switch (reg.X_add_number)
	  {
	  case REG_BC:
	  case REG_SP:
	    check_mach (INS_R800);
	    q = frag_more (2);
	    *q++ = prefix;
	    *q = opcode + ((reg.X_add_number & 3) << 4);
	    break;
	  default:
	    ill_op ();
	  }
    }
  return p;
}

operatorT
ez80_operator (char *name, int args, char *next_p ATTRIBUTE_UNUSED)
{
  if (!zmasm_syntax)
    return O_absent;

  if (args == 2)
    {
      if (strcasecmp (name, "AND") == 0)
	return O_bit_and;
      else if (strcasecmp (name, "OR") == 0)
	return O_bit_inclusive_or;
      else if (strcasecmp (name, "XOR") == 0)
	return O_bit_exclusive_or;
    }

  return O_absent;
}

static void s_zmasm (int ignore ATTRIBUTE_UNUSED)
{
  zmasm_syntax = 1;
}

/* Port specific pseudo ops.  */
const pseudo_typeS md_pseudo_table[] =
{
  { "assume", assume, 0},				//SVES ADDED ,support pseudo instructions .ASSUME
  { "cpu", cpu, 0},						//SVES ADDED ,support pseudo instructions .CPU
  { "zmasm", s_zmasm, 0},
  { "db" , emit_data, 1},
  { "d24", cons, 3},
  { "d32", cons, 4},
  { "def24", cons, 3},
  { "def32", cons, 4},
  { "defb", emit_data, 1},  
  { "defs", s_space, 1}, /* Synonym for ds on some assemblers.  */
  { "defw", cons, 2},
  { "ds",   s_space, 1}, /* Fill with bytes rather than words.  */
  { "dw", cons, 2},
  { "dl", cons, 4},
  { "equ", s_set, 0},
  { "section", obj_elf_section, 0},
#if 0
  { "psect", obj_coff_section, 0}, /* TODO: Translate attributes.  */
#endif
  { "set", 0, 0}, 		/* Real instruction on ez80.  */
  { NULL, 0, 0 }
} ;

/*
New Instructions that support the instruction set of eZ80
are added to the existing z80 instruction set. sves
*/	//SVES ADDED
static table_t instab[] =
{
  { "adc",  0x88, 0x4A, emit_adc },
  { "adc.l",0x88, 0x4A, emit_adc },				//sves added
  { "adc.s",0x88, 0x4A, emit_adc },				//sves added
  { "add",  0x80, 0x09, emit_add },
  { "add.l",  0x80, 0x09, emit_add },			//sves added
  { "add.s",  0x80, 0x09, emit_add },			//sves added
  //{ "and",  0x00, 0xA0, emit_s },				//sves modified
  { "and",  0xA0, 0x00, emit_logic },			//sves added
  { "and.l",  0xA0, 0x00, emit_logic },			//sves added
  { "and.s",  0xA0, 0x00, emit_logic },			//sves added
  { "bit",  0xCB, 0x40, emit_bit },
  { "bit.l",  0xCB, 0x40, emit_bit },			//sves added
  { "bit.s",  0xCB, 0x40, emit_bit },			//sves added
  { "call", 0xCD, 0xC4, emit_jpcc },
  { "call.il", 0xCD, 0xC4, emit_jpcc },			//sves added
  { "call.is", 0xCD, 0xC4, emit_jpcc },			//sves added
  { "call.lil", 0xCD, 0xC4, emit_jpcc },
  { "call.lis", 0xCD, 0xC4, emit_jpcc },
  { "call.sil", 0xCD, 0xC4, emit_jpcc },
  { "call.sis", 0xCD, 0xC4, emit_jpcc },
  { "ccf",  0x00, 0x3F, emit_insn },
  //{ "cp",   0x00, 0xB8, emit_s },				//sves modified
  { "cp",  0xB8 , 0x00, emit_logic },			//sves added
  { "cp.l",  0xB8 , 0x00, emit_logic },			//sves added
  { "cp.s",  0xB8 , 0x00, emit_logic },			//sves added
  { "cpd",  0xED, 0xA9, emit_insn },
  { "cpd.l",  0xED, 0xA9, emit_insn },			//sves added
  { "cpd.s",  0xED, 0xA9, emit_insn },			//sves added
  { "cpdr", 0xED, 0xB9, emit_insn },
  { "cpdr.l", 0xED, 0xB9, emit_insn },			//sves added
  { "cpdr.s", 0xED, 0xB9, emit_insn },			//sves added
  { "cpi",  0xED, 0xA1, emit_insn },
  { "cpi.l",  0xED, 0xA1, emit_insn },			//sves added
  { "cpi.s",  0xED, 0xA1, emit_insn },			//sves added
  { "cpir", 0xED, 0xB1, emit_insn },
  { "cpir.l", 0xED, 0xB1, emit_insn },			//sves added
  { "cpir.s", 0xED, 0xB1, emit_insn },			//sves added
  { "cpl",  0x00, 0x2F, emit_insn },
  { "daa",  0x00, 0x27, emit_insn },
  { "dec",  0x0B, 0x05, emit_incdec },
  { "dec.l",0x0B, 0x05, emit_incdec },  		//sves added
  { "dec.s",0x0B, 0x05, emit_incdec },  		//sves added
  { "di",   0x00, 0xF3, emit_insn },
  { "djnz", 0x00, 0x10, emit_jr },
  { "ei",   0x00, 0xFB, emit_insn },
  { "ex",   0x00, 0x00, emit_ex},
  { "ex.l",   0x00, 0x00, emit_ex},				//sves added
  { "ex.s",   0x00, 0x00, emit_ex},				//sves added
  { "exx",  0x00, 0xD9, emit_insn },
  { "halt", 0x00, 0x76, emit_insn },
  { "im",   0xED, 0x46, emit_im },
  { "in",   0x00, 0x00, emit_in },
  { "in0",   0x00, 0x00, emit_in0 },			//sves added
  { "inc",  0x03, 0x04, emit_incdec },
  { "inc.l",  0x03, 0x04, emit_incdec },		//sves added
  { "inc.s",  0x03, 0x04, emit_incdec },		//sves added
  { "ind",  0xED, 0xAA, emit_insn },
  { "ind.l",  0xED, 0xAA, emit_insn },			//sves added
  { "ind.s",  0xED, 0xAA, emit_insn },  		//sves added
  { "ind2",  0xED, 0x8C, emit_insn },  			//sves added
  { "ind2.l",  0xED, 0x8C, emit_insn },  		//sves added
  { "ind2.s",  0xED, 0x8C, emit_insn },  		//sves added
  { "ind2r",  0xED, 0x9C, emit_insn },  		//sves added
  { "ind2r.l",  0xED, 0x9C, emit_insn },  		//sves added
  { "ind2r.s",  0xED, 0x9C, emit_insn },  		//sves added
  { "indm",  0xED, 0x8A, emit_insn },  			//sves added
  { "indm.l",  0xED, 0x8A, emit_insn },  		//sves added
  { "indm.s",  0xED, 0x8A, emit_insn },  		//sves added
  { "indmr",  0xED, 0x9A, emit_insn },  		//sves added
  { "indmr.l",  0xED, 0x9A, emit_insn },  		//sves added
  { "indmr.s",  0xED, 0x9A, emit_insn },  		//sves added  
  { "indr", 0xED, 0xBA, emit_insn },
  { "indr.l", 0xED, 0xBA, emit_insn },			//sves added
  { "indr.s", 0xED, 0xBA, emit_insn },			//sves added  
  { "indrx", 0xED, 0xCA, emit_insn },			//sves added
  { "indrx.l", 0xED, 0xCA, emit_insn },			//sves added
  { "indrx.s", 0xED, 0xCA, emit_insn },			//sves added
  { "ini",  0xED, 0xA2, emit_insn },
  { "ini.l",  0xED, 0xA2, emit_insn },			//sves added
  { "ini.s",  0xED, 0xA2, emit_insn },			//sves added  
  { "ini2", 0xED, 0x84, emit_insn },			//sves added
  { "ini2.l", 0xED, 0x84, emit_insn },			//sves added
  { "ini2.s", 0xED, 0x84, emit_insn },			//sves added  
  { "ini2r", 0xED, 0x94, emit_insn },			//sves added
  { "ini2r.l", 0xED, 0x94, emit_insn },			//sves added
  { "ini2r.s", 0xED, 0x94, emit_insn },			//sves added  
  { "inim", 0xED, 0x82, emit_insn },			//sves added
  { "inim.l", 0xED, 0x82, emit_insn },			//sves added
  { "inim.s", 0xED, 0x82, emit_insn },			//sves added  
  { "inimr", 0xED, 0x92, emit_insn },			//sves added
  { "inimr.l", 0xED, 0x92, emit_insn },			//sves added
  { "inimr.s", 0xED, 0x92, emit_insn },			//sves added  
  { "inir", 0xED, 0xB2, emit_insn },
  { "inir.l", 0xED, 0xB2, emit_insn },			//sves added
  { "inir.s", 0xED, 0xB2, emit_insn },			//sves added  
  { "inirx", 0xED, 0xC2, emit_insn },			//sves added
  { "inirx.l", 0xED, 0xC2, emit_insn },			//sves added
  { "inirx.s", 0xED, 0xC2, emit_insn },			//sves added  
  { "jp",   0xC3, 0xC2, emit_jpcc },
  { "jp.l",   0xC3, 0xC2, emit_jpcc },			//sves added
  { "jp.lil",   0xC3, 0xC2, emit_jpcc },		//sves added
  { "jp.lis",   0xC3, 0xC2, emit_jpcc },
  { "jp.s",   0xC3, 0xC2, emit_jpcc },			//sves added
  { "jp.sil",   0xC3, 0xC2, emit_jpcc },
  { "jp.sis",   0xC3, 0xC2, emit_jpcc },		//sves added
  { "jr",   0x18, 0x20, emit_jrcc },
  { "ld",   0x00, 0x00, emit_ld },
  { "ld.il",   0x00, 0x00, emit_ld },			//sves added  
  { "ld.is",   0x00, 0x00, emit_ld },			//sves added  
  { "ld.l",   0x00, 0x00, emit_ld },			//sves added  
  { "ld.lil",   0x00, 0x00, emit_ld },			//sves added  
  { "ld.lis",   0x00, 0x00, emit_ld },
  { "ld.s",   0x00, 0x00, emit_ld },			//sves added
  { "ld.sil",   0x00, 0x00, emit_ld },
  { "ld.sis",   0x00, 0x00, emit_ld },			//sves added  
  { "ldd",  0xED, 0xA8, emit_insn },
  { "ldd.l",  0xED, 0xA8, emit_insn },			//sves added 
  { "ldd.s",  0xED, 0xA8, emit_insn },			//sves added 
  { "lddr", 0xED, 0xB8, emit_insn },
  { "lddr.l", 0xED, 0xB8, emit_insn },			//sves added
  { "lddr.s", 0xED, 0xB8, emit_insn },			//sves added
  { "ldi",  0xED, 0xA0, emit_insn },
  { "ldi.l",  0xED, 0xA0, emit_insn },			//sves added
  { "ldi.s",  0xED, 0xA0, emit_insn },			//sves added
  { "ldir", 0xED, 0xB0, emit_insn },
  { "ldir.l", 0xED, 0xB0, emit_insn },			//sves added
  { "ldir.s", 0xED, 0xB0, emit_insn },			//sves added
  { "lea",   0xED, 0x00, emit_lea },			//sves added
  { "lea.l",   0xED, 0x00, emit_lea },			//sves added
  { "lea.s",   0xED, 0x00, emit_lea },			//sves added
  { "mlt", 0xED, 0x4C, emit_mlt },				//sves added
  { "mlt.l", 0xED, 0x4C, emit_mlt },			//sves added
  { "mlt.s", 0xED, 0x4C, emit_mlt },			//sves added
  { "mulub", 0xED, 0xC5, emit_mulub },			/* R800 only.  */
  { "muluw", 0xED, 0xC3, emit_muluw }, 			/* R800 only.  */
  { "neg",  0xee, 0x44, emit_insn },			//sves added  
  { "nop",  0x00, 0x00, emit_insn },
  //{ "or",   0x00, 0xB0, emit_s }, 			//sves modified
  { "or",   0xB0, 0x00, emit_logic }, 			//sves added 
  { "or.l",   0xB0, 0x00, emit_logic }, 		//sves added 
  { "or.s",   0xB0, 0xB0, emit_logic }, 		//sves added 
  { "otd2r", 0xED, 0xBC, emit_insn },			//sves added  
  { "otd2r.l", 0xED, 0xBC, emit_insn },			//sves added  
  { "otd2r.s", 0xED, 0xBC, emit_insn },			//sves added
  { "otdm", 0xED, 0x8B, emit_insn },			//sves added
  { "otdm.l", 0xED, 0x8B, emit_insn },			//sves added
  { "otdm.s", 0xED, 0x8B, emit_insn },			//sves added  
  { "otdmr", 0xED, 0x9B, emit_insn },			//sves added
  { "otdmr.l", 0xED, 0x9B, emit_insn },			//sves added
  { "otdmr.s", 0xED, 0x9B, emit_insn },			//sves added
  { "otdr", 0xED, 0xBB, emit_insn },
  { "otdr.l", 0xED, 0xBB, emit_insn },			//sves added
  { "otdr.s", 0xED, 0xBB, emit_insn },			//sves added
  { "otdrx", 0xED, 0xCB, emit_insn },			//sves added
  { "otdrx.l", 0xED, 0xCB, emit_insn },			//sves added
  { "otdrx.s", 0xED, 0xCB, emit_insn },			//sves added
  { "oti2r", 0xED, 0xB4, emit_insn },			//sves added
  { "oti2r.l", 0xED, 0xB4, emit_insn },			//sves added
  { "oti2r.s", 0xED, 0xB4, emit_insn },			//sves added
  { "otim", 0xED, 0x83, emit_insn },			//sves added
  { "otim.l", 0xED, 0x83, emit_insn },			//sves added
  { "otim.s", 0xED, 0x83, emit_insn },			//sves added
  { "otimr", 0xED, 0x93, emit_insn },			//sves added
  { "otimr.l", 0xED, 0x93, emit_insn },			//sves added
  { "otimr.s", 0xED, 0x93, emit_insn },			//sves added  
  { "otir", 0xED, 0xB3, emit_insn },
  { "otir.l", 0xED, 0xB3, emit_insn },			//sves added 
  { "otir.s", 0xED, 0xB3, emit_insn },			//sves added 
  { "otirx", 0xED, 0xC3, emit_insn },			//sves added 
  { "otirx.l", 0xED, 0xC3, emit_insn },			//sves added 
  { "otirx.s", 0xED, 0xC3, emit_insn },			//sves added 
  { "out",  0x00, 0x00, emit_out },
  { "out0",  0x00, 0x00, emit_out0 },
  { "outd", 0xED, 0xAB, emit_insn },
  { "outd.l", 0xED, 0xAB, emit_insn },			//sves added 
  { "outd.s", 0xED, 0xAB, emit_insn },			//sves added 
  { "outd2", 0xED, 0xAC, emit_insn },			//sves added 
  { "outd2.l", 0xED, 0xAC, emit_insn },			//sves added 
  { "outd2.s", 0xED, 0xAC, emit_insn },			//sves added 
  { "outi", 0xED, 0xA3, emit_insn },
  { "outi.l", 0xED, 0xA3, emit_insn },			//sves added 
  { "outi.s", 0xED, 0xA3, emit_insn },			//sves added 
  { "outi2", 0xED, 0xA4, emit_insn },			//sves added 
  { "outi2.l", 0xED, 0xA4, emit_insn },			//sves added 
  { "outi2.s", 0xED, 0xA4, emit_insn },			//sves added 
  { "pea",  0xED, 0x65, emit_pea },				//sves added
  { "pea.l",  0xED, 0x65, emit_pea },			//sves added
  { "pea.s",  0xED, 0x65, emit_pea },			//sves added  
  { "pop",  0x00, 0xC1, emit_pop },
  { "pop.l",0x00, 0xC1, emit_pop },				//sves added
  { "pop.s",0x00, 0xC1, emit_pop },				//sves added
  { "push", 0x00, 0xC5, emit_pop },
  { "push.l", 0x00, 0xC5, emit_pop },			//sves added
  { "push.s", 0x00, 0xC5, emit_pop },			//sves added
  { "res",  0xCB, 0x80, emit_bit },
  { "res.l",  0xCB, 0x80, emit_bit },			//sves added
  { "res.s",  0xCB, 0x80, emit_bit },			//sves added
  { "ret",  0xC9, 0xC0, emit_retcc },
  { "ret.l",  0xC9, 0xC0, emit_retcc },			//sves added
  { "reti", 0xED, 0x4D, emit_retn },			//sves changed
  { "reti.l", 0xED, 0x4D, emit_retn },			//sves added
  { "retn", 0xED, 0x45, emit_retn },			//sves changed
  { "retn.l", 0xED, 0x45, emit_retn },			//sves added
  { "rl",   0xCB, 0x10, emit_mr },
  { "rl.l",   0xCB, 0x10, emit_mr },			//sves added
  { "rl.s",   0xCB, 0x10, emit_mr },			//sves added
  { "rla",  0x00, 0x17, emit_insn },
  { "rlc",  0xCB, 0x00, emit_mr },
  { "rlc.l",  0xCB, 0x00, emit_mr },			//sves added
  { "rlc.s",  0xCB, 0x00, emit_mr },			//sves added
  { "rlca", 0x00, 0x07, emit_insn },
  { "rld",  0xED, 0x6F, emit_insn },
  { "rr",   0xCB, 0x18, emit_mr },
  { "rr.l",   0xCB, 0x18, emit_mr },			//sves added
  { "rr.s",   0xCB, 0x18, emit_mr },			//sves added
  { "rra",  0x00, 0x1F, emit_insn },
  { "rrc",  0xCB, 0x08, emit_mr },
  { "rrc.l",  0xCB, 0x08, emit_mr },			//sves added
  { "rrc.s",  0xCB, 0x08, emit_mr },			//sves added
  { "rrca", 0x00, 0x0F, emit_insn },
  { "rrd",  0xED, 0x67, emit_insn },
  { "rsmix",  0xED, 0x7E, emit_insn },
  { "rst",  0x00, 0xC7, emit_rst},
  { "rst.l",  0x00, 0xC7, emit_rst},			//sves added
  { "rst.s",  0x00, 0xC7, emit_rst},			//sves added
  { "sbc",  0x98, 0x42, emit_adc },
  { "sbc.l",  0x98, 0x42, emit_adc },			//sves added
  { "sbc.s",  0x98, 0x42, emit_adc },			//sves added
  { "scf",  0x00, 0x37, emit_insn },
  { "set",  0xCB, 0xC0, emit_bit },
  { "set.l",  0xCB, 0xC0, emit_bit },			//sves added
  { "set.s",  0xCB, 0xC0, emit_bit },			//sves added
  { "sla",  0xCB, 0x20, emit_mr },
  { "sla.l",  0xCB, 0x20, emit_mr },			//sves added
  { "sla.s",  0xCB, 0x20, emit_mr },			//sves added
  { "sli",  0xCB, 0x30, emit_mr },
  { "sll",  0xCB, 0x30, emit_mr },
  { "slp",  0xED, 0x76, emit_insn },
  { "sra",  0xCB, 0x28, emit_mr },
  { "sra.l",  0xCB, 0x28, emit_mr },			//sves added
  { "sra.s",  0xCB, 0x28, emit_mr },			//sves added
  { "srl",  0xCB, 0x38, emit_mr },
  { "srl.l",  0xCB, 0x38, emit_mr },			//sves added
  { "srl.s",  0xCB, 0x38, emit_mr },			//sves added
  { "stmix",  0xED, 0x7D, emit_insn },			//sves added
//{ "sub",  0x00, 0x90, emit_s },				//sves modified
  { "sub",  0x90, 0x00, emit_logic },			//sves added
  { "sub.l",  0x90, 0x00, emit_logic },			//sves added
  { "sub.s",  0x90, 0x00, emit_logic },			//sves added
  { "tst",  0xED, 0x04, emit_test },			//sves added
  { "tst.l",  0xED, 0x73, emit_test },			//sves added
  { "tst.s",  0xED, 0x34, emit_test },			//sves added
  { "tstio",  0xED, 0x74, emit_test_io },		//sves added
//{ "xor",  0x00, 0xA8, emit_s },				//sves modified
  { "xor",  0xA8, 0x00, emit_logic },			//sves added
  { "xor.l",  0xA8, 0x00, emit_logic },			//sves added
  { "xor.s",  0xA8, 0x00, emit_logic },			//sves added
} ;


//******************************************************************************************
//md_assemble		: Assemble one instruction at a time.
//parameter 		: Assembly instruction.
//Return 			: None.
//*******************************************************************************************
void
md_assemble (char* str)
{
	const char *p; 
	char * old_ptr;
	int i,j;
	table_t *insp;
	err_flag = 0;
	old_ptr = input_line_pointer;
	p = skip_space (str);
/*Scanning mnemonic for Alphanumeric Characters, '0', '2' and '.' and 
converting Alphanumeric Characters into lower case and storing mnemonic into 
buf array. */
	//sves start
	for (i = 0; (i < BUFLEN) && ((ISALPHA (*p))||(*p=='.')||(*p=='2')||(*p=='0'));)		//sves added
		buf[i++] = TOLOWER (*p++);

/*Clearing char suffix array every time for each instruction and Storing 
eZ80 suffix .s and .l into char suffix array to support Ez80 instruction . */
	memset(suffix,0,sizeof(suffix));
	for(j = 0;j < i;j++)
	{
		if(buf[j] =='.')
		break;
	}
/*Copying mnemonic  from char buf array to char suffix array inorder to store 
 eZ80 suffix */
	if (i - j < (int)sizeof(suffix))
	{
		strncpy(suffix,&buf[j],i-j);
		suffix[i-j] = 0;
	}
	else
	{
		strncpy(suffix,&buf[j],sizeof(suffix)-1);
		suffix[sizeof(suffix)-1] = 0;
	}
	//sves end
  if (i == BUFLEN)
    {
      buf[BUFLEN-3] = buf[BUFLEN-2] = '.'; /* Mark opcode as abbreviated.  */
      buf[BUFLEN-1] = 0;
      as_bad (_("Unknown instruction '%s'"), buf);
    }
  else if ((*p) && (!ISSPACE (*p)))
	{
		as_bad (_("syntax error"));
	}
  else 
    {
      buf[i] = 0;
      p = skip_space (p);
      key = buf;
      insp = bsearch (&key, instab, ARRAY_SIZE (instab),
		    sizeof (instab[0]), key_cmp);
     if (!insp){
		as_bad (_("Unknown instruction '%s'"), buf);}
      else
	{
	  p = insp->fp (insp->prefix, insp->opcode, p);
	  p = skip_space (p);
	/*if ((!err_flag) && *p)
	  as_bad (_("junk at end of line, first unrecognised character is `%c'"),
		  *p);*/
	}
    }
  input_line_pointer = old_ptr;
}

void
md_apply_fix (fixS * fixP, valueT* valP, segT seg ATTRIBUTE_UNUSED)
{
  long val = * (long *) valP;
  char *p_lit = fixP->fx_where + fixP->fx_frag->fr_literal;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_8_PCREL:
      if (fixP->fx_addsy)
        {
          fixP->fx_no_overflow = 1;
          fixP->fx_done = 0;
        }
      else
        {
	  fixP->fx_no_overflow = (-128 <= val && val < 128);
	  if (!fixP->fx_no_overflow)
            as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("relative jump out of range"));
	  *p_lit++ = val;
          fixP->fx_done = 1;
        }
      break;

    case BFD_RELOC_Z80_DISP8:
      if (fixP->fx_addsy)
        {
          fixP->fx_no_overflow = 1;
          fixP->fx_done = 0;
        }
      else
        {
	  fixP->fx_no_overflow = (-128 <= val && val < 128);
	  if (!fixP->fx_no_overflow)
            as_bad_where (fixP->fx_file, fixP->fx_line,
			  _("index offset  out of range"));
	  *p_lit++ = val;
          fixP->fx_done = 1;
        }
      break;

    case BFD_RELOC_8:
      if (val > 255 || val < -128)
	as_warn_where (fixP->fx_file, fixP->fx_line, _("overflow"));
      *p_lit++ = val;
      fixP->fx_no_overflow = 1; 
      if (fixP->fx_addsy == NULL)
	fixP->fx_done = 1;
      break;

    case BFD_RELOC_16:
      *p_lit++ = val;
      *p_lit++ = (val >> 8);
      fixP->fx_no_overflow = 1; 
      if (fixP->fx_addsy == NULL)
	fixP->fx_done = 1;
      break;

    case BFD_RELOC_24: /* Def24 may produce this.  */
      *p_lit++ = val;
      *p_lit++ = (val >> 8);
      *p_lit++ = (val >> 16);
      fixP->fx_no_overflow = 1; 
      if (fixP->fx_addsy == NULL)
	fixP->fx_done = 1;
      break;

    case BFD_RELOC_32: /* Def32 and .long may produce this.  */
      *p_lit++ = val;
      *p_lit++ = (val >> 8);
      *p_lit++ = (val >> 16);
      *p_lit++ = (val >> 24);
      if (fixP->fx_addsy == NULL)
	fixP->fx_done = 1;
      break;

    default:
      printf (_("md_apply_fix: unknown r_type 0x%x\n"), fixP->fx_r_type);
      abort ();
    }
}

/* GAS will call this to generate a reloc.  GAS will pass the
   resulting reloc to `bfd_install_relocation'.  This currently works
   poorly, as `bfd_install_relocation' often does the wrong thing, and
   instances of `tc_gen_reloc' have been written to work around the
   problems, which in turns makes it difficult to fix
   `bfd_install_relocation'.  */

/* If while processing a fixup, a reloc really
   needs to be created then it is done here.  */

arelent *
tc_gen_reloc (asection *seg ATTRIBUTE_UNUSED , fixS *fixp)
{
  arelent *reloc;

  if (! bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type))
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("reloc %d not supported by object file format"),
		    (int) fixp->fx_r_type);
      return NULL;
    }

  reloc               = xmalloc (sizeof (arelent));
  reloc->sym_ptr_ptr  = xmalloc (sizeof (asymbol *));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address      = fixp->fx_frag->fr_address + fixp->fx_where;
  reloc->howto        = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  reloc->addend       = fixp->fx_offset;

  return reloc;
}
