# Lab4: Preemptive Multitasking

## Introduction

### Getting Started

### Lab Requirements

## Part A: Multiprocessor Support and Cooperative Multitasking

### Multiprocessor Support

#### Exercise 1.
> Implement mmio_map_region in kern/pmap.c. To see how this is used, look at the beginning of lapic_init in kern/lapic.c. You'll have to do the next exercise, too, before the tests for mmio_map_region will run. 


### Application Processor Bootstrap

#### Exercise 2.
> Read boot_aps() and mp_main() in kern/init.c, and the assembly code in kern/mpentry.S. Make sure you understand the control flow transfer during the bootstrap of APs. Then modify your implementation of page_init() in kern/pmap.c to avoid adding the page at MPENTRY_PADDR to the free list, so that we can safely copy and run AP bootstrap code at that physical address. Your code should pass the updated check_page_free_list() test (but might fail the updated check_kern_pgdir() test, which we will fix soon).

#### Question 1.
> Compare kern/mpentry.S side by side with boot/boot.S. Bearing in mind that kern/mpentry.S is compiled and linked to run above KERNBASE just like everything else in the kernel, what is the purpose of macro MPBOOTPHYS? Why is it necessary in kern/mpentry.S but not in boot/boot.S? In other words, what could go wrong if it were omitted in kern/mpentry.S?
> Hint: recall the differences between the link address and the load address that we have discussed in Lab 1. 

Answer:
In boot.S we use low memory which is equal to physical address. However in mpentry.S we are running the kernel code, which has already running in high memory, and lgdt instruction needs physical address, so we use MPBOOTPHYS macro to translate it into physical memory.

## Part B: Copy-on-Write Fork

### User-level page fault handling

#### Setting the Page Fault Handler

##### Exercise 8.

> Implement the sys_env_set_pgfault_upcall system call. Be sure to enable permission checking when looking up the environment ID of the target environment, since this is a "dangerous" system call. 

#### Normal and Exception Stacks in User Environments

#### Exercise 14.

NOTE:
```c
    SETGATE(idt[T_SYSCALL ], 0, GD_KT, &_SYSCALL , 3);
```
the second arg should be 0!!!


## Part C: Preemptive Multitasking and Inter-Process communication (IPC)

### Clock Interrupts and Preemption

#### Interrupt discipline

##### Exercise 13.

#### Handling Clock Interrupts

##### Exercise 14.

### Inter-Process communication (IPC)

#### IPC in JOS

#### Sending and Receiving Messages

#### Transferring Pages

#### Implementing IPC

##### Exercise 15.

