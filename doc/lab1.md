# Lab 1: Booting a PC

# Lab1: Booting a PC

- [ ] Exercise 1
- [ ] Exercise 2
- [ ] 


## Introduction
### Software Setup
### Hand-In Procedure
## Part 1: PC Bootstrap
### Getting Started with x86 assembly
---
>Exercise 1. Familiarize yourself with the assembly language materials available on [the 6.828 reference page](https://pdos.csail.mit.edu/6.828/2018/reference.html). You don't have to read them now, but you'll almost certainly want to refer to some of this material when reading and writing x86 assembly.
>
>We do recommend reading the section "The Syntax" in [Brennan's Guide to Inline Assembly](http://www.delorie.com/djgpp/doc/brennan/brennan_att_inline_djgpp.html). It gives a good (and quite brief) description of the AT&T assembly syntax we'll be using with the GNU assembler in JOS.
---
### Simulating the x86
### The PC's Physical Address Space
```text
+------------------+  <- 0xFFFFFFFF (4GB)
|      32-bit      |
|  memory mapped   |
|     devices      |
|                  |
/\/\/\/\/\/\/\/\/\/\

/\/\/\/\/\/\/\/\/\/\
|                  |
|      Unused      |
|                  |
+------------------+  <- depends on amount of RAM
|                  |
|                  |
| Extended Memory  |
|                  |
|                  |
+------------------+  <- 0x00100000 (1MB)
|     BIOS ROM     |
+------------------+  <- 0x000F0000 (960KB)
|  16-bit devices, |
|  expansion ROMs  |
+------------------+  <- 0x000C0000 (768KB)
|   VGA Display    |
+------------------+  <- 0x000A0000 (640KB)
|                  |
|    Low Memory    |
|                  |
+------------------+  <- 0x00000000
```
### The ROM BIOS
---
>Exercise 2. Use GDB's si (Step Instruction) command to trace into the ROM BIOS for a few more instructions, and try to guess what it might be doing. You might want to look at [Phil Storrs I/O Ports Description](http://web.archive.org/web/20040404164813/members.iweb.net.au/~pstorr/pcbook/book2/book2.htm), as well as other materials on the [6.828 reference materials page](https://pdos.csail.mit.edu/6.828/2018/reference.html). No need to figure out all the details - just the general idea of what the BIOS is doing first.
---
## Part 2: The Boot Loader
When the BIOS finds a bootable floppy or hard disk, it loads the 512-byte boot sector into memory at physical addresses `0x7c00` through `0x7dff`, and then uses a `jmp` instruction to set the `CS:IP` to `0000:7c00`, passing control to the boot loader.

---
>Exercise 3. Take a look at the [lab tools guide](https://pdos.csail.mit.edu/6.828/2018/labguide.html), especially the section on GDB commands. Even if you're familiar with GDB, this includes some esoteric GDB commands that are useful for OS work.
>
>Set a breakpoint at address `0x7c00`, which is where the boot sector will be loaded. Continue execution until that breakpoint. Trace through the code in `boot/boot.S`, using the source code and the disassembly file `obj/boot/boot.asm` to keep track of where you are. Also use the `x/i` command in GDB to disassemble sequences of instructions in the boot loader, and compare the original boot loader source code with both the disassembly in `obj/boot/boot.asm` and GDB.
>
>Trace into bootmain() in boot/main.c, and then into readsect(). Identify the exact assembly instructions that correspond to each of the statements in readsect(). Trace through the rest of readsect() and back out into bootmain(), and identify the begin and end of the for loop that reads the remaining sectors of the kernel from the disk. Find out what code will run when the loop is finished, set a breakpoint there, and continue to that breakpoint. Then step through the remainder of the boot loader.
---

Be able to answer the following questions:
-   At what point does the processor start executing 32-bit code? What exactly causes the switch from 16- to 32-bit mode?
```asm
0x7c2d: ljmp $0xb866,$0x87c32
-> 32-bit
0x7c32
```

-   What is the _last_ instruction of the boot loader executed, and what is the _first_ instruction of the kernel it just loaded?
```asm
0x7d63: call *0x10018
```
-   _Where_ is the first instruction of the kernel?
```asm
0x1000c
```
-   How does the boot loader decide how many sectors it must read in order to fetch the entire kernel from disk? Where does it find this information?
```text
Via reading ELF header Table, which contains the info of program segement header info. Within program header table, it contains the info about the length of one particular program header table. By reading all of lengthes of program segments, the boot loader then acknowledge how many sectors it has to read.
```

### Loading the Kernel
---
>Exercise 4. Read about programming with pointers in C. The best reference for the C language is _The C Programming Language_ by Brian Kernighan and Dennis Ritchie (known as 'K&R'). We recommend that students purchase this book (here is an [Amazon Link](http://www.amazon.com/C-Programming-Language-2nd/dp/0131103628/sr=8-1/qid=1157812738/ref=pd_bbs_1/104-1502762-1803102?ie=UTF8&s=books)) or find one of [MIT's 7 copies](http://library.mit.edu/F/AI9Y4SJ2L5ELEE2TAQUAAR44XV5RTTQHE47P9MKP5GQDLR9A8X-10422?func=item-global&doc_library=MIT01&doc_number=000355242&year=&volume=&sub_library=).
>
>Read 5.1 (Pointers and Addresses) through 5.5 (Character Pointers and Functions) in K&R. Then download the code for [pointers.c](https://pdos.csail.mit.edu/6.828/2018/labs/lab1/pointers.c), run it, and make sure you understand where all of the printed values come from. In particular, make sure you understand where the pointer addresses in printed lines 1 and 6 come from, how all the values in printed lines 2 through 4 get there, and why the values printed in line 5 are seemingly corrupted.
>
>There are other references on pointers in C (e.g., [A tutorial by Ted Jensen](https://pdos.csail.mit.edu/6.828/2018/readings/pointers.pdf) that cites K&R heavily), though not as strongly recommended.
>
>_Warning:_ Unless you are already thoroughly versed in C, do not skip or even skim this reading exercise. If you do not really understand pointers in C, you will suffer untold pain and misery in subsequent labs, and then eventually come to understand them the hard way. Trust us; you don't want to find out what "the hard way" is.

Take particular note of the "VMA" (or _link address_) and the "LMA" (or _load address_) of the .text section. The load address of a section is the memory address at which that section should be loaded into memory.

The link address of a section is the memory address from which the section expects to execute. The linker encodes the link address in the binary in various ways, such as when the code needs the address of a global variable, with the result that a binary usually won't work if it is executing from an address that it is not linked for. (It is possible to generate _position-independent_ code that does not contain any such absolute addresses. This is used extensively by modern shared libraries, but it has performance and complexity costs, so we won't be using it in 6.828.)

---
>Exercise 5. Trace through the first few instructions of the boot loader again and identify the first instruction that would "break" or otherwise do the wrong thing if you were to get the boot loader's link address wrong. Then change the link address in boot/Makefrag to something wrong, run make clean, recompile the lab with make, and trace into the boot loader again to see what happens. Don't forget to change the link address back and make clean again afterward!
---
```
change from 0x7c00 to 0x8c00
```

---
>Exercise 6. We can examine memory using GDB's x command. The [GDB manual](https://sourceware.org/gdb/current/onlinedocs/gdb/Memory.html) has full details, but for now, it is enough to know that the command x/_N_x _ADDR_ prints _N_ words of memory at _ADDR_. (Note that both 'x's in the command are lowercase.) _Warning_: The size of a word is not a universal standard. In GNU assembly, a word is two bytes (the 'w' in xorw, which stands for word, means 2 bytes).
>
>Reset the machine (exit QEMU/GDB and start them again). Examine the 8 words of memory at 0x00100000 at the point the BIOS enters the boot loader, and then again at the point the boot loader enters the kernel. Why are they different? What is there at the second breakpoint? (You do not really need to use QEMU to answer this question. Just think.)
---


## Part 3: The Kernel
### Using virtual memory to work around position dependence
---
>Exercise 7. Use QEMU and GDB to trace into the JOS kernel and stop at the `movl %eax, %cr0`. Examine memory at `0x00100000` and at `0xf0100000`. Now, single step over that instruction using the stepi GDB command. Again, examine memory at `0x00100000` and at `0xf0100000`. Make sure you understand what just happened.
>
>What is the first instruction after the new mapping is established that would fail to work properly if the mapping weren't in place? Comment out the movl %eax, %cr0 in kern/entry.S, trace into it, and see if you were right. 
---
```shell
(gdb) x/10x 0x00100000
0x100000:       0x1badb002      0x00000000      0xe4524ffe      0x7205c766
0x100010:       0x34000004      0x7000b812      0x220f0011      0xc0200fd8
0x100020:       0x0100010d      0xc0220f80
(gdb) x/10x 0xf0100000
0xf0100000 <_start-268435468>:  0x00000000      0x00000000      0x00000000      0x00000000
0xf0100010 <entry+4>:   0x00000000      0x00000000      0x00000000      0x00000000
0xf0100020 <entry+20>:  0x00000000      0x00000000
(gdb) si
=> 0x100028:    mov    $0xf010002f,%eax
0x00100028 in ?? ()
(gdb) x/10x 0xf0100000
0xf0100000 <_start-268435468>:  0x1badb002      0x00000000      0xe4524ffe      0x7205c766
0xf0100010 <entry+4>:   0x34000004      0x7000b812      0x220f0011      0xc0200fd8
0xf0100020 <entry+20>:  0x0100010d      0xc0220f80
(gdb) x/10x 0x00100000
0x100000:       0x1badb002      0x00000000      0xe4524ffe      0x7205c766
0x100010:       0x34000004      0x7000b812      0x220f0011      0xc0200fd8
0x100020:       0x0100010d      0xc0220f80
```

NOTE:
	GDB is using virtual mm, `0x1bad002` is ELF magic code

### Formatted Printing to the Console

Read through `kern/printf.c`, `lib/printfmt.c`, and `kern/console.c`, and make sure you understand their relationship. It will become clear in later labs why printfmt.c is located in the separate lib directory.

---
>Exercise 8. We have omitted a small fragment of code - the code necessary to print octal numbers using patterns of the form "%o". Find and fill in this code fragment.
---

Be able to answer the following questions:
1. Explain the interface between printf.c and console.c. Specifically, what function does console.c export? How is this function used by printf.c?
	```text
		kern/printf.c::cprintf()
				-> kern/printf.c::vcprintf() // prvide *putch function pointer
						-> lib/printfmt.c::vprintfmt()
								-> Arg:putch -> /kern/printf.c::putch()
										-> kern/console.c::cputchar()
												-> kern/console.c::cons_putc()
	```
	
	`static void putch(int ch, int *cnt)` is exported by console.c. 
	the printf.c put it into the argument of  `vprintfmt()` in `lib/printfmt()`, which is called by `vcprintf()` in `kern/printf.c`
   
2. Explain the following from console.c:
	```c
    1      if (crt_pos >= CRT_SIZE) {
    2              int i;
    3              memmove(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16_t));
    4              for (i = CRT_SIZE - CRT_COLS; i < CRT_SIZE; i++)
    5                      crt_buf[i] = 0x0700 | ' ';
    6              crt_pos -= CRT_COLS;
    7      }
	```
    __Anser:__
    if cursor? is out off the limit of CRT buffer size `CRT_SIZE`, copy the second line through the last line from the begining of the buffer, with the result that the new last line is ready to be cleared with `0x0700 | ' '` and output new contents.
	
3. For the following questions you might wish to consult the notes for Lecture 2. These notes cover GCC's calling convention on the x86.
   
   Trace the execution of the following code step-by-step:
	```c
    int x = 1, y = 3, z = 4;
    cprintf("x %d, y %x, z %d\n", x, y, z);
	```
	- In the call to `cprintf()`, to what does `fmt` point? To what does `ap` point?
	    ==Answer:==
	    `fmt` points to the string `"x %d, y %x, z %d\n"`.
	    `ap` points to the variable `x, y, z` in each time.
	- List (in order of execution) each call to `cons_putc`, `va_arg`, and `vcprintf`. For `cons_putc`, list its argument as well. For `va_arg`, list what `ap` points to before and after the call. For `vcprintf` list the values of its two arguments.
	  
	
4. Run the following code.
```c
	unsigned int i = 0x00646c72;
	cprintf("H%x Wo%s", 57616, &i);
```

  What is the output? Explain how this output is arrived at in the step-by-step manner of the previous exercise. [Here's an ASCII table](http://web.cs.mun.ca/~michael/c/ascii-table.html) that maps bytes to characters.
    
  The output depends on that fact that the x86 is little-endian. If the x86 were instead big-endian what would you set `i` to in order to yield the same output? Would you need to change `57616` to a different value?
    
  [Here's a description of little- and big-endian](http://www.webopedia.com/TERM/b/big_endian.html) and [a more whimsical description](http://www.networksorcery.com/enp/ien/ien137.txt).

  Answer:
```shell
	He110, World
```

5.  In the following code, what is going to be printed after `'y='`? (note: the answer is not a specific value.) Why does this happen?
        cprintf("x=%d y=%d", 3);
    
6.  Let's say that GCC changed its calling convention so that it pushed arguments on the stack in declaration order, so that the last argument is pushed last. How would you have to change `cprintf` or its interface so that it would still be possible to pass it a variable number of arguments?





# This is a test

hello

```plain
I want some test
```

1. list
   this is code?
	   this is code?


```
add code block in list
```

	ok that is fine
	so why

this is aliagned

1. this is a list
   I use shift + enter to create this line
	I use tab to create this line
   I want some aligned paragraph
   ```rust
   I use shift + enter and then add '```' to create a block then everything messup
	```
	ok
	That is fine
	
   
   ```rust
   I want some aligned code block
```

	I want some aligned paragraph
```

## Part 3: The Kernel

### Formatted Printing to the Console

#### Exercise 8

> Exercise 8. We have omitted a small fragment of code - the code necessary to print octal numbers using patterns of the form "%o". Find and fill in this code fragment.

1. Explain the interface between `printf.c` and `console.c`.  Specifically, what function does `console.c` export?  How is this function used by `printf.c`?

   > Answer:
   >
   > ```shell
   >                                		=> kern/printf.c::cprintf()
   >                                		=> kern/printf.c::vcprintf()
   >                                                                		|	=> lib/printfmt.c::vprintfmt()
   >                                		|	kern/printf.c::putch()	<=	|
   >                              		|	<=                                 
   > kern/console.c::cputchar()			|
   > =>	kern/console.c::cons_putchar()	|
   > 	=>	serial_putc()				|
   > 	=>	lpt_putc()
   > 	=>	cga_putc()
   > ```
   >
   > The `console.c` export `cputchar()` function, `print.c` use it by putting and calling it in a static function `putch()` which is passed as a function pointer into `lib/printfmt.c::vprintfmt()` function.

2. Explain the following from `console.c`:
   ```c
       1      if (crt_pos >= CRT_SIZE) {
       2              int i;
       3              memmove(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16_t));
       4              for (i = CRT_SIZE - CRT_COLS; i < CRT_SIZE; i++)
       5                      crt_buf[i] = 0x0700 | ' ';
       6              crt_pos -= CRT_COLS;
       7      }
   ```

   > Answer:
   >
   > 

3. For the following questions you might wish to consult the notes for Lecture 2. These notes cover GCC's calling convention on the x86.

   Trace the execution of the following code step-by-step:

   ```c
   int x = 1, y = 3, z = 4;
   cprintf("x %d, y %x, z %d\n", x, y, z);
   ```

   - In the call to `cprintf()`, to what does `fmt` point? To what does `ap` point?

     > Answer:
     >
     > ```shell
     > // Set up break point:
     > (gdb) br cprintf
     > Breakpoint 1 at 0xf010093a: file kern/printf.c, line 27.
     > // we don't break at printfmt() because it will take us a lot of time in printfmt. if we return from a function back to printfmt(), we will not reach at the breakpoint. But in printfmt, there is a infinite loop, we break here so that we can observe the change within printfmt funtion
     > (gdb) br printfmt.c:92
     > Breakpoint 2 at 0xf0100d69: file lib/printfmt.c, line 92.	
     > (gdb) br cputchar
     > Breakpoint 3 at 0xf0100678: file kern/console.c, line 457.
     > (gdb) br cons_putc
     > Breakpoint 4 at 0xf010026e: file kern/console.c, line 434.
     > 
     > // we first break at cprintf, and we get output from gdb:
     > (gdb) c
     > Continuing.
     > The target architecture is set to "i386".
     > => 0xf010093a <cprintf>:        push   %ebp
     > 
     > Breakpoint 1, cprintf (fmt=0xf0101837 "x %d, y %x, z %d\n") at kern/printf.c:27
     > 
     > // NOTE: at that time, we have not save $ebp, we use 'si' command let code save it and we get:
     > (gdb) si
     > => 0xf010093b <cprintf+1>:      mov    %esp,%ebp
     > // Previous step save the $ebp, then we will set new $ebp
     > (gdb) si
     > => 0xf010093d <cprintf+3>:      sub    $0x18,%esp
     > // Now, $ebp has the same value as $esp, then we will reserve space for local variable, which is done by sub $esp
     > // we can infer that in cprintf() we have 0x18/0x4 = 6 local variables. They are x, y, z, fmt, ap, cnt
     > (gdb) si
     > => 0xf0100940 <cprintf+6>:      lea    0xc(%ebp),%eax
     > 31              va_start(ap, fmt);
     > // Now we finish the frame change, we can check the frame info via info frame command
     > (gdb) info frame
     > Stack level 0, frame at 0xf0117fd0:
     >  eip = 0xf0100940 in cprintf (kern/printf.c:31); saved eip = 0xf01000ee
     >  called by frame at 0xf0118000
     >  source language c.
     >  Arglist at 0xf0117fc8, args: fmt=0xf0101837 "x %d, y %x, z %d\n"
     >  Locals at 0xf0117fc8, Previous frame's sp is 0xf0117fd0
     >  Saved registers:
     >   ebp at 0xf0117fc8, eip at 0xf0117fcc
     > (gdb)
     > // we can examine it via printing $ebp
     > (gdb) x/8xw $ebp
     > 0xf0117fc8:     0xf0117ff8      0xf01000ee      0xf0101837      0x00000001
     > 0xf0117fd8:     0x00000003      0x00000004      0x00000000      0x00000000
     > (gdb)
     > // here is the gcc calling convention
     > 		       +------------+   |
     > 		       | arg 2      |   \
     > 		       +------------+    >- previous function's stack frame
     > 		       | arg 1      |   /
     > 		       +------------+   |
     > 		       | ret %eip   |   /
     > 		       +============+   
     > 		       | saved %ebp |   \
     > 		%ebp-> +------------+   |
     > 		       |            |   |
     > 		       |   local    |   \
     > 		       | variables, |    >- current function's stack frame
     > 		       |    etc.    |   /
     > 		       |            |   |
     > 		       |            |   |
     > 		%esp-> +------------+   /
     > 
     > ```
     >
     > Now we have that `fmt` is pointing to the location of string `"x %d, y %x, z %d\n"`
     >
     > Let's examine local variables first:
     >
     > ```shell
     > (gdb) x/10i 0xf0100940
     >    0xf0100940 <cprintf+6>:      lea    0xc(%ebp),%eax
     > => 0xf0100943 <cprintf+9>:      mov    %eax,0x4(%esp)			// save 2nd arg 'x' into 2nd position
     >    0xf0100947 <cprintf+13>:     mov    0x8(%ebp),%eax
     >    0xf010094a <cprintf+16>:     mov    %eax,(%esp)				// save 1st arg 'fmt' into 1st postion
     >    0xf010094d <cprintf+19>:     call   0xf0100907 <vcprintf>	// call vcprintf(fmt, ap)
     >    0xf0100952 <cprintf+24>:     leave
     >    0xf0100953 <cprintf+25>:     ret
     > // 0xf0100940 is one step after reserving space for local variables
     > // list the corresponding c code below:
     > (gdb) l
     > 27      {
     > 28              va_list ap;
     > 29              int cnt;
     > 30
     > 31              va_start(ap, fmt);
     > 32              cnt = vcprintf(fmt, ap);
     > 33              va_end(ap);
     > 34
     > 35              return cnt;
     > 36      }
     > (gdb)
     > // we can find that the assembly code seems like skip the code of va_start(ap, fmt)
     > // this might because of the optimization of gcc, since it actually just save the 1st position of the variable length arguments
     > 
     > ```
     >
     > Now, we know that `ap` is pointing to 1st variable length arguments.

   - List (in order of execution) each call to `cons_putc`, `va_arg`, and `vcprintf`. For `cons_putc`, list its argument as well.  For `va_arg`, list what `ap` points to before and after the call.  For `vcprintf` list the values of its two arguments.

     > ```shell
     > // With the previous breakpoints
     > // we use 'n' command to stop at next line of !!c code!!, which is `printfmt:92`
     > (gdb) n
     > => 0xf0100d69 <vprintfmt+35>:   movzbl (%esi),%eax
     > 
     > Breakpoint 2, vprintfmt (putch=0xf01008f4 <putch>, putdat=0xf0117f9c, fmt=0xf0101837 "x %d, y %x, z %d\n", ap=0xf0117fd4 "\001") at lib/printfmt.c:92
     > 92                      while ((ch = *(unsigned char *) fmt++) != '%') {
     > (gdb)
     > 
     > // with several n command, we first get cputchar breakpoint
     > // we can find that it takes an int variable which is equal to 120, which is actually 'x' in ASCII table
     > // now we know that it will print 'x' to the console
     > (gdb) n
     > => 0xf0100678 <cputchar>:       push   %ebp
     > 
     > Breakpoint 3, cputchar (c=120) at kern/console.c:457
     > 457     {
     > (gdb)
     > 
     > // we resume the program with c command, then the program is stop at function cons_putc function
     > // we find its argument is 120, which as we said previously, is 'x' in ASCII
     > (gdb) c
     > Continuing.
     > => 0xf010026e <cons_putc+11>:   mov    $0x3201,%ebx
     > 
     > Breakpoint 4, cons_putc (c=120) at kern/console.c:434
     > 434     {
     > (gdb)
     > 
     > // we continue our program
     > // and find that the program back and stop at printfmt.c:92
     > (gdb) c
     > Continuing.
     > => 0xf0100d69 <vprintfmt+35>:   movzbl (%esi),%eax
     > 
     > Breakpoint 2, vprintfmt (putch=0xf01008f4 <putch>, putdat=0xf0117f9c, fmt=0xf0101837 "x %d, y %x, z %d\n", ap=0xf0117fd4 "\001") at lib/printfmt.c:92
     > 92                      while ((ch = *(unsigned char *) fmt++) != '%') {
     > (gdb)
     > 
     > // Now we can find that in qemu window, there is a 'x' character printed on the console. We finish the first time of printing.
     > // we can guess that the next one is `space` and it has hex code 32 in ASCII table
     > (gdb) c
     > Continuing.
     > => 0xf010026e <cons_putc+11>:   mov    $0x3201,%ebx
     > 
     > Breakpoint 4, cons_putc (c=32) at kern/console.c:434
     > 434     {
     > (gdb)
     > 
     > // continue the program, we get back to vprintfmt()
     > // we first check the fmt and ap via display command, then each time we step forward, gdb will print the value
     > // or we can use watch point to automatically observing the change of our two variables
     > (gdb) display fmt
     > 1: fmt = 0xf0101839 "%d, y %x, z %d\n"
     > (gdb) display ap
     > 2: ap = (va_list) 0xf0117fd4 "\001"
     > (gdb) watch fmt
     > Watchpoint 5: fmt
     > (gdb) watch ap
     > Hardware watchpoint 6: ap
     > 
     > // continue the program with n command, we should stop at a point that either fmt or ap changed
     > // At this time it should be fmt variable, we can check the changed value
     > (gdb) n
     > => 0xf0100d6d <vprintfmt+39>:   cmp    $0x25,%eax
     > 
     > Watchpoint 5: fmt
     > 
     > Old value = 0xf0101839 "%d, y %x, z %d\n"
     > New value = 0xf010183a "d, y %x, z %d\n"
     > 0xf0100d6d in vprintfmt (putch=0xf01008f4 <putch>, putdat=0xf0117f9c, fmt=0xf0101837 "x %d, y %x, z %d\n", ap=0xf0117fd4 "\001") at lib/printfmt.c:92
     > 92                      while ((ch = *(unsigned char *) fmt++) != '%') {
     > 1: fmt = 0xf010183a "d, y %x, z %d\n"
     > 2: ap = (va_list) 0xf0117fd4 "\001"
     > (gdb)
     > 
     > // go ahead with several n command, we find that the program will call va_arg() function
     > (gdb) n
     > => 0xf0100fce <vprintfmt+648>:  mov    0x14(%ebp),%eax
     > 75                      return va_arg(*ap, int);
     > 1: fmt = 0xf010183a "d, y %x, z %d\n"
     > 2: ap = (va_list) 0xf0117fd4 "\001"
     > (gdb)
     > // step one step forward
     > // watch point find that ap is now point to the next position of variable length arguments
     > // we can guess that next character printed should be '1', because the '%d' indicate that x should be interpreted as an intger. 
     > (gdb) n
     > => 0xf0100fd7 <vprintfmt+657>:  mov    (%eax),%esi
     > 
     > Hardware watchpoint 6: ap
     > 
     > Old value = (va_list) 0xf0117fd4 "\001"
     > New value = (va_list) 0xf0117fd8 "\003"
     > 0xf0100fd7 in vprintfmt (putch=0xf01008f4 <putch>, putdat=0xf0117f9c, fmt=0xf0101837 "x %d, y %x, z %d\n", ap=0xf0117fd8 "\003") at lib/printfmt.c:75
     > 75                      return va_arg(*ap, int);
     > 1: fmt = 0xf010183a "d, y %x, z %d\n"
     > 2: ap = (va_list) 0xf0117fd8 "\003"
     > (gdb)
     > 
     > // continue the program, we are right 49 is '1' in ASCII
     > (gdb) c
     > Continuing.
     > => 0xf010026e <cons_putc+11>:   mov    $0x3201,%ebx
     > 
     > Breakpoint 4, cons_putc (c=49) at kern/console.c:434
     > 434     {
     > (gdb)
     > 
     > // we will just put the args for function below.
     > fmt = 0xf010183b ", y %x, z %d\n"
     > ap = (va_list) 0xf0117fd8 "\003"
     > Breakpoint 4, cons_putc (c=44) at kern/console.c:434
     > fmt = 0xf010183c " y %x, z %d\n"
     > ap = (va_list) 0xf0117fd8 "\003"
     > Breakpoint 4, cons_putc (c=32) at kern/console.c:434
     > fmt = 0xf010183d "y %x, z %d\n"
     > ap = (va_list) 0xf0117fd8 "\003"
     > Breakpoint 4, cons_putc (c=121) at kern/console.c:434
     > fmt = 0xf010183e " %x, z %d\n"
     > ap = (va_list) 0xf0117fd8 "\003"
     > Breakpoint 4, cons_putc (c=32) at kern/console.c:434
     > fmt = 0xf010183f "%x, z %d\n"
     > ap = (va_list) 0xf0117fd8 "\003"
     > // NOTE: at that point ap should be changed to point 0xf0117fdc -> '\004'
     > Breakpoint 4, cons_putc (c=51) at kern/console.c:434
     > fmt = 0xf0101841 ", z %d\n"
     > ap = (va_list) 0xf0117fdc "\004"
     > Breakpoint 4, cons_putc (c=44) at kern/console.c:434
     > fmt = 0xf0101842 " z %d\n"
     > ap = (va_list) 0xf0117fdc "\004"
     > Breakpoint 4, cons_putc (c=32) at kern/console.c:434
     > fmt = 0xf0101843 "z %d\n"
     > ap = (va_list) 0xf0117fdc "\004"
     > Breakpoint 4, cons_putc (c=122) at kern/console.c:434
     > fmt = 0xf0101844 " %d\n"
     > ap = (va_list) 0xf0117fdc "\004"
     > Breakpoint 4, cons_putc (c=32) at kern/console.c:434
     > fmt = 0xf0101845 "%d\n"
     > ap = (va_list) 0xf0117fdc "\004"
     > // As that point, as you might expect, ap should be changed
     > // As a result va_arg just return the value of ap pointing to and step one step forward of ap
     > // Accessing the ap after variable length arguments number time calling va_arg should be a not defined behavior.
     > Breakpoint 4, cons_putc (c=52) at kern/console.c:434
     > fmt = 0xf0101847 "\n"
     > ap = (va_list) 0xf0117fe0 ""
     > Breakpoint 4, cons_putc (c=10) at kern/console.c:434
     > fmt = 0xf0101848 ""
     > ap = (va_list) 0xf0117fe0 ""
     > ```
     >
     > We forget to break at `vcprintf` function, it doesn't matter, we just call it once right now. Let's check its arguments from beginning:
     >
     > ```shell
     > // we can find that its argument is the same as cprintf function as we mentioned previously.
     > (gdb) break vcprintf
     > Breakpoint 1 at 0xf0100907: file kern/printf.c, line 18.
     > (gdb) c
     > Continuing.
     > The target architecture is set to "i386".
     > => 0xf0100907 <vcprintf>:       push   %ebp
     > 
     > Breakpoint 1, vcprintf (fmt=0xf0101837 "x %d, y %x, z %d\n", ap=0xf0117fd4 "\001") at kern/printf.c:18
     > 18      {
     > (gdb)
     > 
     > ```

4. Run the following code.

   ```c
    unsigned int i = 0x00646c72;
    cprintf("H%x Wo%s", 57616, &i);
   ```

   What is the output?  Explain how this output is arrived at in the step-by-step manner of the previous exercise. [Here's an ASCII   table](http://web.cs.mun.ca/~michael/c/ascii-table.html) that maps bytes to characters.

   The output depends on that fact that the x86 is little-endian.  If the x86 were instead big-endian what would you set `i` to in order to yield the same output?  Would you need to change `57616` to a different value?

   [Here's a description of little- and big-endian](http://www.webopedia.com/TERM/b/big_endian.html) and [a more whimsical description](http://www.networksorcery.com/enp/ien/ien137.txt).

    > Answer:
    >
    > ```shell
    > He110, World
    > ```
    >
    > ```
    > 57616 = 0xe110
    > ```
    >
    > ```
    > 0x00646c72 in memory:
    > little endian						big endian
    > address:	hex code:	char:		hex code:	char:
    > 0x3			0x00	=>	'\0'		0x72	=>	'r'
    > 0x2			0x64	=>	'd'			0x6c	=>	'l'
    > 0x1			0x6c	=>	'l'			0x64	=>	'd'
    > 0x0			0x72	=>	'r'			0x00	=>	'\0'
    > ```
    >
    > we can set `i` as `0x726c6400` in order to yield the same output under big-endian

5. In the following code, what is going to be printed after `'y='`?  (note: the answer is not a specific value.)  Why does this happen?

   ```c
       cprintf("x=%d y=%d", 3);
   ```

   > Answer:
   >
   > ```shell
   > x=3, y=-2672885966828
   > ```
   >
   > ```shell
   > // we use gdb to check what happened:
   > 
   > // first we break at 1st calling to vcprintf everything should be ok
   > Breakpoint 1, vcprintf (fmt=0xf0101853 "x=%d, y=%d", ap=0xf0117fd4 "\003") at kern/printf.c:18
   > 1: fmt = 0xf0101854 "=%d, y=%d"
   > 2: ap = (va_list) 0xf0117fd4 "\003"
   > Breakpoint 4, cons_putc (c=61) at kern/console.c:434
   > fmt = 0xf0101855 "%d, y=%d"
   > ap = (va_list) 0xf0117fd4 "\003"
   > // ap change:
   > => 0xf0100fce <vprintfmt+648>:  mov    0x14(%ebp),%eax
   > 75                      return va_arg(*ap, int);
   > 1: fmt = 0xf0101856 "d, y=%d"
   > 2: ap = (va_list) 0xf0117fd4 "\003"
   > // to
   > 1: fmt = 0xf0101857 "d, y=%d"
   > 2: ap = (va_list) 0xf0117fd8 "\354\177\021\360\004"
   > Breakpoint 4, cons_putc (c=51) at kern/console.c:434
   > 1: fmt = 0xf0101857 ", y=%d"
   > 2: ap = (va_list) 0xf0117fd8 "\354\177\021\360\004"
   > Breakpoint 4, cons_putc (c=44) at kern/console.c:434
   > 1: fmt = 0xf0101859 "y=%d"
   > 2: ap = (va_list) 0xf0117fd8 "\354\177\021\360\004"
   > Breakpoint 4, cons_putc (c=32) at kern/console.c:434
   > 1: fmt = 0xf0101859 "y=%d"
   > 2: ap = (va_list) 0xf0117fd8 "\354\177\021\360\004"
   > Breakpoint 4, cons_putc (c=121) at kern/console.c:434
   > 1: fmt = 0xf010185a "=%d"
   > 2: ap = (va_list) 0xf0117fd8 "\354\177\021\360\004"
   > Breakpoint 4, cons_putc (c=61) at kern/console.c:434
   > 1: fmt = 0xf010185b "%d"
   > 2: ap = (va_list) 0xf0117fd8 "\354\177\021\360\004"
   > Breakpoint 4, cons_putc (c=45) at kern/console.c:434
   > Breakpoint 4, cons_putc (c=50) at kern/console.c:434
   > Breakpoint 4, cons_putc (c=54) at kern/console.c:434
   > Breakpoint 4, cons_putc (c=55) at kern/console.c:434
   > Breakpoint 4, cons_putc (c=50) at kern/console.c:434
   > Breakpoint 4, cons_putc (c=56) at kern/console.c:434
   > Breakpoint 4, cons_putc (c=56) at kern/console.c:434
   > Breakpoint 4, cons_putc (c=53) at kern/console.c:434
   > Breakpoint 4, cons_putc (c=57) at kern/console.c:434
   > Breakpoint 4, cons_putc (c=54) at kern/console.c:434
   > 
   > // we can examine what is the value at address 0xf0117fd8
   > (gdb) x/x 0xf0117fd8
   > 0xf0117fd8:     0xf0117fec
   > // Is actually a local variable which pointing to 0x00646c72 in Q4
   > // 0xf0117fec is actually -267288596 in signed integer
   > (gdb) x/x 0xf0117fec
   > 0xf0117fec:     0x00646c72
   > (gdb) print/d  0xf0117fec
   > $2 = -267288596
   > ```
   >
   > Now we know that why it print out a negative number.
   >
   > I guess if we change the location of print(), we will get different value.

6. Let's say that GCC changed its calling convention so that it pushed arguments on the stack in declaration order, so that the last argument is pushed last. How would you have to change `cprintf` or its interface so that it would still be possible to pass it a variable number of arguments?

   > ANSWER:
   >
   > we can just revers the order of `cprintf` arguments, so that the order of arguments on stack should be the same.

---

#### Challenge

> Challenge Enhance the console to allow text to be printed in different colors. The traditional way to do this is to make it interpret [ANSI escape sequences](http://rrbrandt.dee.ufcg.edu.br/en/docs/ansi/) embedded in the text strings printed to the console, but you may use any mechanism you like. There is plenty of information on [the 6.828 reference page](https://pdos.csail.mit.edu/6.828/2018/reference.html) and elsewhere on the web on programming the VGA display hardware. If you're feeling really adventurous, you could try switching the VGA hardware into a graphics mode and making the console draw text onto the graphical frame buffer.

### The Stack

#### Exercise 9:
> Exercise 9. Determine where the kernel initializes its stack, and exactly where in memory its stack is located. How does the kernel reserve space for its stack? And at which "end" of this reserved area is the stack pointer initialized to point to? 

#### Answer:

We can use  `objdump` command determine where the kernel stack is located:

```shell
⋊> ~/D/c/m/lab on lab1 ⨯ i386-jos-elf-objdump -x obj/kern/kernel | grep "stack"                   15:37:57
f0118000 g       .data  00000000 bootstacktop
f0110000 g       .data  00000000 bootstack
⋊> ~/D/c/m/lab on lab1 ⨯   
```

We can examine our observation in `kern/entry.S` and its objdump file `obj/kern/kernel.asm`.

In `entry.S` file, we have:

```assembly
# Clear the frame pointer register (EBP)
# so that once we get into debugging C code,
# stack backtraces will be terminated properly.
movl	$0x0,%ebp			# nuke frame pointer

# Set the stack pointer
movl	$(bootstacktop),%esp
```

, and

```assembly
.data
###################################################################
# boot stack
###################################################################
	.p2align	PGSHIFT		# force page alignment
	.globl		bootstack
bootstack:
	.space		KSTKSIZE
	.globl		bootstacktop   
bootstacktop:
```

In `kernel.asm` we have:

```assembly
f010002f <relocated>:
relocated:

	# Clear the frame pointer register (EBP)
	# so that once we get into debugging C code,
	# stack backtraces will be terminated properly.
	movl	$0x0,%ebp			# nuke frame pointer
f010002f:	bd 00 00 00 00       	mov    $0x0,%ebp

	# Set the stack pointer
	movl	$(bootstacktop),%esp
f0100034:	bc 00 80 11 f0       	mov    $0xf0118000,%esp
```

From above, we can find that variable `bootstacktop` is set as `0xf0118000`, which is the same as what we find from `objdump` command. Also, we now know that kernel reserve space for its stack via allocate `KSTKSIZE` bytes data in data section of kernel runnable program. The starting position is `0xf0110000`, which is exported as `bootstack` variable as we found from `objdump` command. Finally, we can determine that the stack pointer initialized to point to higher end from the respect of memory location.

---

#### Exercise 10:

> Exercise 10. To become familiar with the C calling conventions on the x86, find the address of the `test_backtrace` function in `obj/kern/kernel.asm`, set a breakpoint there, and examine what happens each time it gets called after the kernel starts. How many 32-bit words does each recursive nesting level of `test_backtrace` push on the stack, and what are those words?
>
> Note that, for this exercise to work properly, you should be using the patched version of QEMU available on the [tools](https://pdos.csail.mit.edu/6.828/2018/tools.html) page or on Athena. Otherwise, you'll have to manually translate all breakpoint and memory addresses to linear addresses.

#### Answer

First, find the entry point of `test_backtrace`, it is `0xf0100040`

```shell
⋊> ~/D/c/m/lab on lab1 ⨯ cat obj/kern/kernel.asm | grep "test_backtrace"                          15:38:07
f0100040 <test_backtrace>:
test_backtrace(int x)
        cprintf("entering test_backtrace %d\n", x);
f010005c:       7e 0d                   jle    f010006b <test_backtrace+0x2b>
                test_backtrace(x-1);
f0100064:       e8 d7 ff ff ff          call   f0100040 <test_backtrace>
f0100069:       eb 1c                   jmp    f0100087 <test_backtrace+0x47>
        cprintf("leaving test_backtrace %d\n", x);
        test_backtrace(5);
f010013f:       e8 fc fe ff ff          call   f0100040 <test_backtrace>

```

Break at `0xf0100040`:

```shell
(gdb) br *0xf0100040
Breakpoint 1 at 0xf0100040: file kern/init.c, line 13.
(gdb) c
Continuing.
The target architecture is set to "i386".
=> 0xf0100040 <test_backtrace>: push   %ebp

Breakpoint 1, test_backtrace (x=5) at kern/init.c:13
13      {
(gdb) x/16i 0xf0100040
=> 0xf0100040 <test_backtrace>: push   %ebp
   0xf0100041 <test_backtrace+1>:       mov    %esp,%ebp
   0xf0100043 <test_backtrace+3>:       push   %ebx
   0xf0100044 <test_backtrace+4>:       sub    $0x14,%esp
   0xf0100047 <test_backtrace+7>:       mov    0x8(%ebp),%ebx
   0xf010004a <test_backtrace+10>:      mov    %ebx,0x4(%esp)
   0xf010004e <test_backtrace+14>:      movl   $0xf0101800,(%esp)
   0xf0100055 <test_backtrace+21>:      call   0xf010093a <cprintf>
   0xf010005a <test_backtrace+26>:      test   %ebx,%ebx
   0xf010005c <test_backtrace+28>:      jle    0xf010006b <test_backtrace+43>
   0xf010005e <test_backtrace+30>:      lea    -0x1(%ebx),%eax
   0xf0100061 <test_backtrace+33>:      mov    %eax,(%esp)
   0xf0100064 <test_backtrace+36>:      call   0xf0100040 <test_backtrace>
   0xf0100069 <test_backtrace+41>:      jmp    0xf0100087 <test_backtrace+71>
   0xf010006b <test_backtrace+43>:      movl   $0x0,0x8(%esp)
   0xf0100073 <test_backtrace+51>:      movl   $0x0,0x4(%esp)
(gdb) x/16xw $esp
0xf0117fcc:     0xf0100144      0x00000005      0x00001aac      0xf0117fec
0xf0117fdc:     0x00000004      0x00000000      0x00000000      0x00000000
0xf0117fec:     0x00646c72      0x00000000      0x00000000      0x00000000
0xf0117ffc:     0xf010003e      0x00119021      0x00000000      0x00000000
(gdb) x/16xw $esp - 0x20
0xf0117fac:     0xf0100952      0xf010185e      0xf0117fd4      0x00000000
0xf0117fbc:     0x00010074      0x00010074      0x00000000      0xf0117ff8
0xf0117fcc:     0xf0100144      0x00000005      0x00001aac      0xf0117fec
0xf0117fdc:     0x00000004      0x00000000      0x00000000      0x00000000
(gdb)

// we can find that test_backtrace first save $ebx on stack, and then save reserve space for local variables. We can also notice that the program use the local variable spaces as the input arguments spaces for calling function.
// Thus the function pushes (0x14 + 0x4) / 4 = 6 32-bit words on stack.
// Lets's break at 0xf0100055 to examine what they are

(gdb) x/20xw $esp - 0x10
0xf0117fa0:     0x00000000      0x00000000      0xf0117fc8      0xf0100952
0xf0117fb0:     0xf0101800      0x00000005      0x00000000      0x00010074
0xf0117fc0:     0x00010074      0x00010074      0xf0117ff8      0xf0100144
// $esp is pointing to 0xf0117fb0 also the 1st arg of cprintf
// 0xf0117bf4 is the 2nd arg of cprintf
// 0xf0117fb0 ~ 0xf0117fc3 are the spaces reserved by local variables
// 0xf0117fc4 is the saved $ebx and is saved x variable at the same time
// Note that although 0xf0117fb8 ~ 0xf0117fc3 have value in it, it doesn't mean anything,
// Because it has not been initialized, and may save some value of previous functions.
0xf0117fd0:     0x00000005      0x00001aac      0xf0117fec      0x00000004
0xf0117fe0:     0x00000000      0x00000000      0x00000000      0x00646c72
(gdb) info frame
Stack level 0, frame at 0xf0117fd0:
 eip = 0xf0100055 in test_backtrace (kern/init.c:14); saved eip = 0xf0100144
 called by frame at 0xf0118000
 source language c.
 Arglist at 0xf0117fc8, args: x=5
 Locals at 0xf0117fc8, Previous frame's sp is 0xf0117fd0
 Saved registers:
  ebx at 0xf0117fc4, ebp at 0xf0117fc8, eip at 0xf0117fcc
(gdb) info r
eax            0x0                 0
ecx            0x3d4               980
edx            0x3d5               981
ebx            0x5                 5
esp            0xf0117fb0          0xf0117fb0
ebp            0xf0117fc8          0xf0117fc8
esi            0x10074             65652
edi            0x0                 0
eip            0xf0100055          0xf0100055 <test_backtrace+21>
eflags         0x82                [ SF ]
cs             0x8                 8
ss             0x10                16
ds             0x10                16
es             0x10                16
fs             0x10                16
gs             0x10                16
(gdb)

```

We will not report the status of stack for each calling of `test_backtrace` cause they are the same.

---

#### Exercise 11

> Exercise 11. Implement the backtrace function as specified above. Use the same format as in the example, since otherwise the grading script will be confused. When you think you have it working right, run make grade to see if its output conforms to what our grading script expects, and fix it if it doesn't. *After* you have handed in your Lab 1 code, you are welcome to change the output format of the backtrace function any way you like.
>
> If you use `read_ebp()`, note that GCC may generate "optimized" code that calls `read_ebp()` *before* `mon_backtrace()`'s function prologue, which results in an incomplete stack trace (the stack frame of the most recent function call is missing). While we have tried to disable optimizations that cause this reordering, you may want to examine the assembly of `mon_backtrace()` and make sure the call to `read_ebp()` is happening after the function prologue.

#### Answer:

Let's see the code:

```c
int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	cprintf("Stack backtrace:\n");
	uint32_t ebp = read_ebp();
	while (ebp)
	{
		cprintf("  ebp %08x  eip %08x  args %08x %08x %08x %08x %08x\n",\
			ebp,\
			*((uint32_t*) (ebp + 4)),\
			*((uint32_t*) (ebp + 8)),\
			*((uint32_t*) (ebp + 0xc)),\
			*((uint32_t*) (ebp + 0x10)),\
			*((uint32_t*) (ebp + 0x14)),\
			*((uint32_t*) (ebp + 0x18))\
		);
		ebp = *((uint32_t*) (ebp));
	}
	return 0;
}
```

Note that in `entry.S` the kernel has set the initial `$ebp` as `0x0`, so we can read the content of `$ebp` till we reach the point its content is `NULL`

```assembly
	# Clear the frame pointer register (EBP)
	# so that once we get into debugging C code,
	# stack backtraces will be terminated properly.
	movl	$0x0,%ebp			# nuke frame pointer
```

---

#### Eexercise 12:

> Exercise 12.        
>
> Modify your stack backtrace function to display, for each `eip`, the function name, source file name, and line number corresponding to that `eip`.
>
> In `debuginfo_eip`, where do `__STAB_*` come from? This question has a long answer; to help you to        discover the answer, here are some things you might want to do:
>
> -  look in the file `kern/kernel.ld` for `__STAB_*`
> -  run objdump -h obj/kern/kernel
> -  run objdump -G obj/kern/kernel
> -  run gcc -pipe -nostdinc -O2 -fno-builtin -I. -MD -Wall -Wno-format -DJOS_KERNEL -gstabs -c -S kern/init.c, and look at init.s.
> -  see if the bootloader loads the symbol table in memory as part of loading the kernel binary
>
> Complete the implementation of `debuginfo_eip` by inserting the call to `stab_binsearch` to find the line        number for an address.
>
> Add a `backtrace` command to the kernel monitor, and extend your implementation of `mon_backtrace` to call `debuginfo_eip` and print a line for each stack frame of the form:
>
> ```shell
> K> backtrace
> Stack backtrace:
>   ebp f010ff78  eip f01008ae  args 00000001 f010ff8c 00000000 f0110580 00000000
>          kern/monitor.c:143: monitor+106
>   ebp f010ffd8  eip f0100193  args 00000000 00001aac 00000660 00000000 00000000
>          kern/init.c:49: i386_init+59
>   ebp f010fff8  eip f010003d  args 00000000 00000000 0000ffff 10cf9a00 0000ffff
>          kern/entry.S:70: <unknown>+0
> K> 
> ```
>
> Each line gives the file name and line within that file of the stack frame's `eip`, followed by the name of the function and the offset of the `eip` from the first instruction of the function (e.g., `monitor+106` means the return `eip` is 106 bytes past the beginning of `monitor`).
>
> Be sure to print the file and function names on a separate line, to avoid confusing the grading script.
>
> Tip: printf format strings provide an easy, albeit obscure, way to print non-null-terminated strings like those in STABS tables.	`printf("%.*s", length, string)` prints at most `length` characters of `string`. Take a look at the printf man page to find out why this works.
>
> You may find that some functions are missing from the backtrace. For example, you will probably see a call to `monitor()` but not to `runcmd()`. This is because the compiler in-lines some function calls. Other optimizations may cause you to see unexpected line numbers. If you get rid of the `-O2` from `GNUMakefile`, the backtraces may make more sense (but your kernel will run more slowly).

#### Answer:

From `kern/kernel.ld` we can find where do `__STAB_*` come from:

```assembly
	/* Include debugging information in kernel memory */
	.stab : {
		PROVIDE(__STAB_BEGIN__ = .);
		*(.stab);
		PROVIDE(__STAB_END__ = .);
		BYTE(0)		/* Force the linker to allocate space
				   for this section */
	}
```

Run `objdump -h obj/kern/kernel` to get sections information

```shell
⋊> ~/D/c/m/lab on lab1 ⨯ i386-jos-elf-objdump -h obj/kern/kernel                                  15:48:25

obj/kern/kernel:     file format elf32-i386

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         000017c6  f0100000  00100000  00001000  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .rodata       0000074c  f01017e0  001017e0  000027e0  2**5
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  2 .stab         0000453d  f0101f2c  00101f2c  00002f2c  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .stabstr      00008c33  f0106469  00106469  00007469  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .data         0000a300  f0110000  00110000  00011000  2**12
                  CONTENTS, ALLOC, LOAD, DATA
  5 .bss          00000648  f011a300  0011a300  0001b300  2**5
                  CONTENTS, ALLOC, LOAD, DATA
  6 .comment      00000011  00000000  00000000  0001b948  2**0
                  CONTENTS, READONLY
⋊> ~/D/c/m/lab on lab1 ⨯    
```

Run `objdump -G obj/kern/kernel` to get stab section information

```shell
⋊> ~/D/c/m/lab on lab1 ⨯ i386-jos-elf-objdump -G obj/kern/kernel                                  16:42:17

obj/kern/kernel:     file format elf32-i386

Contents of .stab section:

Symnum n_type n_othr n_desc n_value  n_strx String

-1     HdrSym 0      1476   00008c32 1
0      SO     0      0      f0100000 1      {standard input}
1      SOL    0      0      f010000c 18     kern/entry.S
2      SLINE  0      44     f010000c 0
3      SLINE  0      57     f0100015 0
4      SLINE  0      58     f010001a 0
5      SLINE  0      60     f010001d 0
6      SLINE  0      61     f0100020 0
7      SLINE  0      62     f0100025 0
8      SLINE  0      67     f0100028 0
9      SLINE  0      68     f010002d 0
10     SLINE  0      74     f010002f 0
11     SLINE  0      77     f0100034 0
12     SLINE  0      80     f0100039 0
13     SLINE  0      83     f010003e 0
14     SO     0      2      f0100040 31     kern/entrypgdir.c
15     OPT    0      0      00000000 49     gcc2_compiled.
16     LSYM   0      0      00000000 64     int:t(0,1)=r(0,1);-2147483648;2147483647;
17     LSYM   0      0      00000000 106    char:t(0,2)=r(0,2);0;127;
18     LSYM   0      0      00000000 132    long int:t(0,3)=r(0,3);-2147483648;2147483647;
19     LSYM   0      0      00000000 179    unsigned int:t(0,4)=r(0,4);0;4294967295;
20     LSYM   0      0      00000000 220    long unsigned int:t(0,5)=r(0,5);0;4294967295;
21     LSYM   0      0      00000000 266    long long int:t(0,6)=r(0,6);-0;4294967295;
22     LSYM   0      0      00000000 309    long long unsigned int:t(0,7)=r(0,7);0;-1;
23     LSYM   0      0      00000000 352    short int:t(0,8)=r(0,8);-32768;32767;
24     LSYM   0      0      00000000 390    short unsigned int:t(0,9)=r(0,9);0;65535;
25     LSYM   0      0      00000000 432    signed char:t(0,10)=r(0,10);-128;127;
26     LSYM   0      0      00000000 470    unsigned char:t(0,11)=r(0,11);0;255;
27     LSYM   0      0      00000000 507    float:t(0,12)=r(0,1);4;0;
28     LSYM   0      0      00000000 533    double:t(0,13)=r(0,1);8;0;
29     LSYM   0      0      00000000 560    long double:t(0,14)=r(0,1);12;0;
30     LSYM   0      0      00000000 593    void:t(0,15)=(0,15)
31     BINCL  0      0      000159c2 613    ./inc/mmu.h
32     BINCL  0      0      0000607a 625    ./inc/types.h
...
1465   SLINE  0      1051   00000114 0
1466   SLINE  0      1048   00000124 0
1467   RSYM   0      1034   00000005 35880  m0:r(8,6)
1468   LBRAC  0      0      00000054 0
1469   RBRAC  0      0      000000cd 0
1470   RSYM   0      1034   00000005 35880  m0:r(8,6)
1471   LBRAC  0      0      00000114 0
1472   RBRAC  0      0      0000012e 0
1473   FUN    0      0      0000012e 0
1474   ENSYM  0      0      f01017c6 0
1475   SO     0      0      f01017c6 0
1476   HdrSym 0      0      00000000 0
```

Run `gcc -pipe -nostdinc -O2 -fno-builtin -I. -MD -Wall -Wno-format -DJOS_KERNEL -gstabs -c -S kern/init.c`

```assembly
	.file	"init.c"
	.stabs	"kern/init.c",100,0,2,.Ltext0
	.text
.Ltext0:
	.stabs	"gcc2_compiled.",60,0,0,0
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"entering test_backtrace %d\n"
.LC1:
	.string	"leaving test_backtrace %d\n"
	.text
	.p2align 4
	.stabs	"test_backtrace:F(0,1)=(0,1)",36,0,0,test_backtrace
	.stabs	"void:t(0,1)",128,0,0,0
	.stabs	"x:P(0,2)=r(0,2);-2147483648;2147483647;",64,0,0,3
	.stabs	"int:t(0,2)",128,0,0,0
	.globl	test_backtrace
	.type	test_backtrace, @function
test_backtrace:
	.stabn	68,0,13,.LM0-.LFBB1
.LM0:
.LFBB1:
.LFB0:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	.stabn	68,0,14,.LM1-.LFBB1
.LM1:
	movl	%edi, %esi
	xorl	%eax, %eax
	.stabn	68,0,13,.LM2-.LFBB1
.LM2:
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
```

