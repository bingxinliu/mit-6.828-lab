// implement fork from user space

#include "inc/env.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/stdio.h"
#include "inc/syscall.h"
#include "inc/types.h"
#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
    if (!(err & FEC_PR) || !(uvpd[PDX(addr)] & PTE_P) || !(uvpt[PGNUM(addr)] & PTE_COW))
        panic("pgfault: unrecoverable page fault, faulting to access page\n");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
    //debug
    //cprintf("fault_va?: [%x]\n", addr);
    if ((r = sys_page_alloc(0, PFTEMP, PTE_P | PTE_U | PTE_W)) != 0)
        panic("pgfault: cannot alloc new page with error code: %e\n", r);

    memcpy(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);

    if ((r = sys_page_map(0, PFTEMP, 0, ROUNDDOWN(addr, PGSIZE), PTE_P | PTE_U | PTE_W)) != 0)
        panic("pgfault: cannot map new page with error: %e\n", r);

    if ((r = sys_page_unmap(0, PFTEMP)) != 0)
        panic("pgfault: unable to unmap temporary page with error: %e\n", r);

	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
    int perm = PTE_U | PTE_P;
    if (uvpt[pn] & PTE_W || uvpt[pn] & PTE_COW)
        perm |= PTE_COW;

    if (uvpt[pn] & PTE_SHARE)
    {
        return sys_page_map(0, (void*)(pn*PGSIZE), envid, (void*)(pn*PGSIZE), uvpt[pn] & PTE_SYSCALL);
    }
    r = sys_page_map(0, (void*) (pn * PGSIZE), envid, (void*) (pn * PGSIZE), perm); 
    if (r != 0) return  r;

    if (perm & PTE_COW)
    {
        r = sys_page_map(envid, (void*) (pn * PGSIZE) , 0, (void*) (pn * PGSIZE), perm);
        if (r != 0) return r;
    }


	// panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
    envid_t child_id;
    int errno;
    set_pgfault_handler(pgfault);

    child_id = sys_exofork();

    if (child_id == 0)
    {
        thisenv = &envs[ENVX(sys_getenvid())];
        return 0;
    }

    for (uint32_t addr = 0; addr < (UTOP - 2*PGSIZE); addr += PGSIZE)
    {
        if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P))
        {
            errno = duppage(child_id, PGNUM(addr));
            if (errno != 0)
                panic("fork: can not duppage with addr: [%x] addr and errno: %e\n", addr, errno);
        }
    }

    errno = sys_page_alloc(child_id, (void *) UXSTACKTOP - PGSIZE, PTE_U | PTE_W | PTE_P);
    if (errno != 0) panic("fork: can not alloc with errno: %e\n", errno);

    extern void _pgfault_upcall(void);
    errno = sys_env_set_pgfault_upcall(child_id, _pgfault_upcall);
    if (errno != 0) panic("fork: can not set pgfault upcall with errno: %e\n", errno);

    errno = sys_env_set_status(child_id, ENV_RUNNABLE);
    if (errno != 0) panic("fork: can not set status with errno: %e\n", errno);
    
    //debug
    cprintf("child[%08x] created\n", child_id);

    return child_id;

	// panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
