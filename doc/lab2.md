# Lab 2: Memory Management

## Introducation

### Getting started

### Lab Requirements

## Part 1: Physical Page Management

#### Exercise 1:

> Exercise 1. In the file kern/pmap.c, you must implement code for the following functions (probably in the order given).
>
> boot_alloc()
> mem_init() (only up to the call to check_page_free_list(1))
> page_init()
> page_alloc()
> page_free()
>
> check_page_free_list() and check_page_alloc() test your physical page allocator. You should boot JOS and see whether check_page_alloc() reports success. Fix your code so that it passes. You may find it helpful to add your own assert()s to verify that your assumptions are correct.

## Part 2: Virtual Memory

#### Exercise 2:

> Exercise 2. Look at chapters 5 and 6 of the [Intel 80386 Reference Manual](https://pdos.csail.mit.edu/6.828/2018/readings/i386/toc.htm), if you haven't done so already.  Read the sections about page translation and page-based protection closely (5.2 and 6.4). We recommend that you also skim the sections about segmentation; while JOS uses the paging hardware for virtual memory and protection, segment translation and segment-based protection cannot be disabled on the x86, so you will need a basic understanding of it.

### Virtual, Linear, and Physical Addresses

#### Exercise 3:

> Exercise 3. While GDB can only access QEMU's memory by virtual address, it's often useful to be able to inspect physical memory while setting up virtual memory.  Review the QEMU [monitor commands](https://pdos.csail.mit.edu/6.828/2018/labguide.html#qemu) from the lab tools guide, especially the `xp` command, which lets you inspect physical memory.  To access the QEMU monitor, press Ctrl-a c in the terminal (the same binding returns to the serial console). 
>
> Use the xp command in the QEMU monitor and the x command in GDB to inspect memory at corresponding physical and virtual addresses and make sure you see the same data.
>
> Our patched version of QEMU provides an info pg command that may also prove useful: it shows a compact but detailed representation of the current page tables, including all mapped memory ranges, permissions, and flags.  Stock QEMU also provides an info mem command that shows an overview of which ranges of virtual addresses are mapped and with what permissions.

#### Exercise 3 Answer:



| C type       | Address type |
| ------------ | ------------ |
| `T*`         | Virtual      |
| `uintptr_t`  | Virtual      |
| `physaddr_t` | Physical     |

#### Question:

> Question
>
> 1. Assuming that the following JOS kernel code is correct, what type should variable `x` have, `uintptr_t` or `physaddr_t` ?
>
>    ```
>    	mystery_t x;
>    	char* value = return_a_pointer();
>    	*value = 10;
>    	x = (mystery_t) value;
>    ```

#### Question Answer:

> virtual address that is `uintptr_t`.

### Reference Counting

### Page Table Management

#### Exercise 4:

> Exercise 4. In the file `kern/pmap.c`, you must implement code for the following functions.
>
> ```
>         pgdir_walk()
>         boot_map_region()
>         page_lookup()
>         page_remove()
>         page_insert()
> ```
>
> `check_page()`, called from `mem_init()`, tests your page table management routines. You should make sure it reports success before proceeding.

#### Exercise 4 Answer:

## Part 3: Kernel Address Space

### Permissions and Fault Isolation

### Initializing the Kernel Address Space

#### Exercise 5:

> Exercise 5. Fill in the missing code in `mem_init()` after the call to `check_page()`.
>
> Your code should now pass the `check_kern_pgdir()` and `check_page_installed_pgdir()` checks.

#### Exercise 5 Answer:

#### Question 2:

>2. What entries (rows) in the page directory have been filled in at this point? What addresses do they map and where do they point? In other words, fill out this table as much as possible: 
>
>| Entry | Base Virtual Address | Points to (logically):                |
>| ----- | -------------------- | ------------------------------------- |
>| 1023  | ?                    | Page table for top 4MB of phys memory |
>| 1022  | ?                    | ?                                     |
>| .     | ?                    | ?                                     |
>| .     | ?                    | ?                                     |
>| .     | ?                    | ?                                     |
>| 2     | 0x00800000           | ?                                     |
>| 1     | 0x00400000           | ?                                     |
>| 0     | 0x00000000           | [see next question]                   |

#### Answer:

| Entry     | Base Virtual Address | Points to (logically):                                       |
| --------- | -------------------- | ------------------------------------------------------------ |
| 1023      | 0xffc00000           | Page table for top 4MB of phys memory (in 256M)              |
| 1022      | 0xff800000           | Page table for top 4MB ~ 8MB of phys memory                  |
| ~         | ~                    | ~                                                            |
| 960/0x3c0 | 0xf0000000           | KERNBASE/ 1st 4MB of phys memory (kernel)                    |
| 959/0x3bf | 0xefc00000           | KSTACKTOP-PTSIZE; top 32KB(8*PGSIZE) is bootstack; CPUs' kernel stacks |
| 958/0x3be | 0xef800000           | NULL; allocated to Memory-maped I/O                          |
| 957/0x3bd | 0xef400000           | UVPT; current page table; PADDR(kernpgdir)                   |
| 956/0x3bc | 0xef000000           | UPAGES; pages list; PADDR(pages)                             |
| 955/0x3bb | 0xeec00000           | NULL; will be allocated to UTOP; UENVS; UXSTACKTOP           |
| 954/0x3ba | 0xee800000           | NULL; top 32KB will be allocated to user exception stack     |
| ~         | ~                    | ~                                                            |
| 2         | 0x00800000           | NULL; UTEXT; will be allocated as user mm space              |
| 1         | 0x00400000           | NULL;; UTEMP                                                 |
| 0         | 0x00000000           | [see next question]                                          |

#### Question 3:

>3. We have placed the kernel and user environment in the same address space.  Why will user programs not be able to read or write the kernel's memory? What specific mechanisms protect the kernel memory?

#### Answer:

We have page protection mechanisms, in each page table entry and page directory entry we have `user/supervisor` bit, which implicating whether user could access this block of memory. We map the kernel's memory with 'user/supervisor' bit not set, so that user can not access kernel's memory.

#### Question 4:

>4. What is the maximum amount of physical memory that this operating system can support? Why?

#### Answer:

In `UPAGES` we can store `PTSIZE(=4MB) / sizeof(struct PageInfo) (=16 + 32 = 48bits = 6 Bytes) ~= 699051` pages info, which means we can store `699051 * 4KB = 2863312896 ~= 28GB` memory info. Thus, `UPAGES` is not the limitation.

Then the limitation should be how many pages should a kernel can allocate. In `page_alloc` function we use the kernel virtual address to clear the page, so that the kernel virtual memory space will limit us for the amount of memory we can allocate.

```c
// In pmap.c:page_alloc()
if (alloc_flags & ALLOC_ZERO) memset(page2kva(page), '\0', PGSIZE);
```

Then the maximum amount of physical memory this OS can support is 256M. Because `KERNBASE` starts at `0xf0000000`, `32-bit` OS can only address `2^32 == 0xffffffff == 4GB` memory, and ABOVE `KERNBASE` the virtual memory is `one to one` mapped to physical memory, the OS can only address `0xffffffff - 0xf0000000 = 256M` memory space

#### Question 5:

>5. How much space overhead is there for managing memory, if we actually had the maximum amount of physical memory? How is this overhead broken down?

#### Answer:

sizeof(struct PageInfo) * (Max physical mm / 4KB)

#### Question 6:

>6. Revisit the page table setup in `kern/entry.S` and `kern/entrypgdir.c`.  Immediately after we turn on paging, EIP is still a low number (a little over 1MB).  At what point do we transition to running at an EIP above KERNBASE?  What makes it possible for us to continue executing at a low EIP between when we enable paging and when we begin running at an EIP above KERNBASE?  Why is this transition necessary?

#### Answer:

After paging is enabled and a long jump instruction, we run above `KERNBASE`. 

```assembly
	# Now paging is enabled, but we're still running at a low EIP
	# (why is this okay?).  Jump up above KERNBASE before entering
	# C code.
	mov	$relocated, %eax
	jmp	*%eax
```

The `emtry_pagetable` make that possible, because it maps both 1st 4MB virtual memory above KERNBASE and 1st 4MB virtual memory to 1st 4MB physical memory. So the kernel can running at high address.

If we don't do that, the we can not know the physical address of low virtual memory and then we can not shot the instruction of long jump since at that time we still uses low address.

