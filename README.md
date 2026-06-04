# Binutils eZ80 Port

 _Please see also the default [README for GNU development tools](README) in this directory._

## Upstream Modifications Summary
 
This document summarizes the core modifications made to the upstream `binutils` files (outside of the newly introduced eZ80-specific files in `bfd/`, `opcodes/`, and `gas/config/`). These changes integrate the new `ez80` architecture into the GNU build system, linker, and assembler core.

## 1. Top-Level and Core BFD Configuration
The BFD library and standard `configure` scripts were modified to natively recognize the new architecture and map it to the ELF binary format.

* **`config.sub`**: Added `ez80` to the list of recognized architectures, allowing `./configure --target=ez80-none-elf` to succeed.
* **`include/elf/common.h`**: Registered a new ELF machine constant: `#define EM_EZ80`.
* **`bfd/archures.c`**: Added `bfd_ez80_arch` to the architecture enums, along with specific machine numbers for the `Z80` and `ADL` modes (`bfd_mach_ez80_z80`, `bfd_mach_ez80_adl`).
* **`bfd/targets.c`**: Registered the `ez80_elf32_vec` vector to BFD's list of recognized targets.
* **`bfd/config.bfd`**: Mapped the `ez80-*-*` target string to the `ez80_elf32_vec` vector.
* **`bfd/Makefile.am` / `bfd/Makefile.in`**: Added `cpu-ez80.c` and `elf32-ez80.c` to the build targets.

## 2. Linker (LD) Integration
* **`ld/configure.tgt`**: Added the target mapping for `ez80-*-*`, instructing the linker to use the newly created `elf32ez80` emulation profile.

## 3. Disassembler (Opcodes) Integration
* **`opcodes/configure.ac`**: Included `bfd_ez80_arch` in the case statement to pull in the `ez80-dis.c` disassembler.
* **`opcodes/disassemble.c`**: Hooked up the generic BFD print function by checking for `bfd_arch_ez80` and routing it to our new `print_insn_ez80` function.
* **`opcodes/disassemble.h`**: Added the `print_insn_ez80` prototype.
* **`opcodes/Makefile.am`**: Appended `ez80-dis.c` to the build recipes.

## 4. Assembler Core (`gas/`) Modifications
The core GNU assembler parsing routines (`read.c`, `app.c`, `macro.c`, `listing.c`) were modified to accommodate the legacy ZiLOG Macro Assembler (ZMASM) syntax. These changes are conditionally guarded where appropriate so standard GNU assembly remains unaffected.

* **`gas/app.c` (The Scrubber)**:
  * **Prime Registers**: In legacy syntax, prime registers are denoted with a trailing single quote (`AF'`, `HL'`). Standard GAS parses a single quote as the opening of a string literal. `do_scrub_chars` was modified to intercept single quotes immediately following register names, converting them into backticks (`AF\``) internally. This cleanly bypasses string tokenization while allowing the backend to map backticks correctly.
  * **Macro Blocks**: Added logic to recognize curly braces `{` and `}` as valid string-quote boundaries. This allows ZMASM macro definitions (which encapsulate strings in braces) to be passed through without having internal commas stripped or spaces collapsed.
* **`gas/read.c`**:
  * **Pseudo-Op Parsing**: In ZMASM mode, pseudo-ops frequently appear at the start of a line without a leading dot (e.g., `XDEF` instead of `.xdef`). Modified the `read_a_source_file` token loop (when `NO_PSEUDO_DOT` is defined) to intercept start-of-line tokens before they default to being parsed as "labels." If the token hits in the pseudo-op hash table, execution jumps immediately to the directive handler.
* **`gas/macro.c`**:
  * **`ENDMACRO` Alias**: Modified the macro parsing loop to safely recognize `ENDMACRO` identically to `.endm` or `ENDM`, ending the macro definition cleanly.
  * **Brace Escaping**: Added exceptions inside the `getstring` token parser so that when processing curly-brace bounded strings for ZMASM macros, standard `\` character escapes are not automatically resolved or mangled.
* **`gas/listing.c`**:
  * **24-bit Output Formatting**: By default, `gas` hardcoded its listing address output format to `%04x` (16-bit). Modified `print_lines` to use a `LISTING_ADDRESS_FORMAT` and `LISTING_ERROR_FORMAT` macro, which is supplied by `tc-ez80.h` as `%06x`, enabling clean 24-bit address output generation in the `.lst` files.
* **`gas/configure.tgt`**: Mapped `ez80-*-*` to the `ez80` target format profile.
* **`gas/Makefile.am`**: Appended `tc-ez80.c` to the target format build sources.

### Credit where Credit is Due:
Although this port was started in 2014 by me initially paying an offshore development firm, I brought it to fruition in 2026 with the assistance of [Google Antigravity](http://antigravity.google/).  _Thank you!_

