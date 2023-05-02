#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H
#define E1000_VENDOR_ID 0x8086
#define E1000_DEVICE_ID 0x100E

#include "kern/pci.h"

int pci_e1000_attach(struct pci_func *pcif);
// #include "kern/e1000.c"
// int
// pci_e1000_attach(struct pci_func *pcif)
// {
//     pci_func_enable(pcif);
//     return 0;
// }


#endif  // SOL >= 6
