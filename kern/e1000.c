#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/stdio.h"
#include "inc/string.h"
#include "inc/types.h"
#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here
volatile void *e1000_va;
// for transmit descriptor buffers
__attribute__((__aligned__(TX_DESC_SIZE)))
struct tx_desc tx_desc_buf[TX_DESC_BUF_NUM];
// for transmit packet buffers one on one with TXD
__attribute__((__aligned__(PGSIZE)))
struct tx_packet tx_packet_buf[TX_PACKET_BUF_NUM];
// for receive descriptor buffers
__attribute__((__aligned__(RX_DESC_SIZE)))
struct rx_desc rx_desc_buf[RX_DESC_BUF_NUM];
// for receive packet buffers one on one with RXD
__attribute__((__aligned__(PGSIZE)))
struct rx_packet rx_packet_buf[RX_PACKET_BUF_NUM];


int
pci_e1000_attach(struct pci_func *pcif)
{
    pci_func_enable(pcif);
    e1000_va = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);

    cprintf("pci_e1000_attach: device status register: [%x]\n", *(uint32_t*)((char*)e1000_va + E1000_STATUS));
    
    assert(TX_DESC_BUF_NUM == TX_PACKET_BUF_NUM);
    for (uint32_t i = 0; i < TX_PACKET_BUF_NUM; ++i)
    {
        tx_desc_buf[i].addr = PADDR(&tx_packet_buf[i]);
        tx_desc_buf[i].cmd |= E1000_TXD_CMD_RS >> E1000_TXD_CMD_SHIFT;
        tx_desc_buf[i].status |= E1000_TXD_STAT_DD >> E1000_TXD_STAT_SHIFT;
    }
    

    // Transmit Initialization
    // struct PageInfo *pp = page_alloc(1);
    // tx_desc_buf = page2kva(pp);

    // Transmit Descriptor Base Address
    *(uint32_t*)((char*)e1000_va + E1000_TDBAH) = 0x0;
    *(uint32_t*)((char*)e1000_va + E1000_TDBAL) = PADDR(tx_desc_buf);

    // Transmit Descriptor Length
    *(uint32_t*)((char*)e1000_va + E1000_TDLEN) = (TX_DESC_BUF_NUM * TX_DESC_SIZE) & ~0x7F;

    // Transmit Descriptor Head/Tail
    *(uint32_t*)((char*)e1000_va + E1000_TDH) = 0x0;
    *(uint32_t*)((char*)e1000_va + E1000_TDT) = 0x0;

    // Initialize the Transmit Control Register(TCTL)
    *(uint32_t*)((char*)e1000_va + E1000_TCTL) = 0x0;
    // Enable | PSP | CT | COLD
    *(uint32_t*)((char*)e1000_va + E1000_TCTL) |= E1000_TCTL_EN | E1000_TCTL_PSP | (0x10 << 4) | (0x40 << 12);

    // Initialize the IPG (Inter Packet Gap)
    *(uint32_t*)((char*)e1000_va + E1000_TIPG) = 0x0;
    *(uint32_t*)((char*)e1000_va + E1000_TIPG) |= 10 | 8 << 10 | 6 << 20;


    // debug
    // cprintf("TDLEN = [%d]\n", *(uint32_t*)((char*)e1000_va + E1000_TDLEN));


    // Receive Initialization
    // Program the Receive Address Registers (RAL/RAH) with the desired Ethernet addresses
    struct rx_addr_reg* rar = (struct rx_addr_reg*) E1000_GET_REG(e1000_va, E1000_RA);
    //*rar = E1000_SET_RECEIVE_ADDR_REG(0x120054525634, 0x0, 0x0, 0x1); //0x525400123456 0x120054525634
    // struct rx_addr_reg r;
    // r.ral = 0x12005452;
    // r.rah = 0x3456;
    // r.as = 0;
    // r.rs = 0;
    // r.av = 1;
    // *rar = r;

    *(uint32_t *)(e1000_va + E1000_RA) = 0x12005452;       // QEMU's default MAC address:
    *(uint32_t *)(e1000_va + E1000_RA + 8) = 0x5634 | E1000_RAH_AV;  // 52:54:00:12:34:56
    //debug
    //cprintf("[RAH:RAL] [av]: [%x:%x] [%x]\n", rar->rah, rar->ral, rar->av);
    // cprintf("[RAH:RAL] : [%x:%x]\n", *(uint32_t *)(((void*)e1000_va) + E1000_RA + 8), *(uint32_t *)(e1000_va + E1000_RA));
    // cprintf("ra:[]\n");
    // for (int i = 0; i < 16; ++i)
    //     cprintf("[%x]\n", *(uint8_t*)(e1000_va + E1000_RA + i));

    // cprintf("rar: [%x], ra:[%x]\n", rar, e1000_va + E1000_RA);
    // cprintf("??? %x\n", *(uint32_t*)rar);
    // cprintf("??? %x\n", *(uint32_t*)(e1000_va + E1000_RA + 8));

    
    struct multicast_table_entry* mta_entry = (struct multicast_table_entry*) E1000_GET_REG(e1000_va, E1000_MTA);
    for(int i = 0; i < E1000_MTA_SIZE; ++i)
    {
        mta_entry[i].entry = 0;
    }

    // set interrupt
    int_mask_clr_reg* imc = E1000_GET_REG(e1000_va, E1000_IMC);
    *imc |= (uint32_t) 0x1ffff; // disable all interrupts

    // set receive descriptor minimum threshold interrupt.

    // Set receive descriptor list.
    // Receive Descriptor Base Address:
    rx_desc_base_addr_high_reg* rdbah = E1000_GET_REG(e1000_va, E1000_RDBAH);
    *rdbah = 0x0;
    rx_desc_base_addr_low_reg* rdbal = E1000_GET_REG(e1000_va, E1000_RDBAL);
    *rdbal = PADDR(rx_desc_buf);
    // Receive Descriptor Length
    rx_desc_len_reg* rdl = E1000_GET_REG(e1000_va, E1000_RDLEN);
    *rdl = (RX_DESC_BUF_NUM * RX_DESC_SIZE) & ~0x7ff;
    // Receive Descriptor Head/Tail
    rx_desc_head_reg* rdh = E1000_GET_REG(e1000_va, E1000_RDH);
    *rdh = 0;
    rx_desc_tail_reg* rdt = E1000_GET_REG(e1000_va, E1000_RDT);
    *rdt = RX_DESC_BUF_NUM - 1;

    // Set Receive Control (RCTL) register
    // leave RCTL.EN as 0b until all set
    rx_control_reg* rctl = E1000_GET_REG(e1000_va, E1000_RCTL);
    // E1000_RCTL_LPE
    rx_control_reg rcr = 0;
    rcr |= E1000_RCTL_SZ_2048 | E1000_RCTL_SECRC | E1000_RCTL_BAM;
    rcr &= ~E1000_RCTL_LPE;

    // Set Receive Packets Buf
    for (int i = 0; i < RX_PACKET_BUF_NUM; ++i)
    {
        rx_desc_buf[i].addr = PADDR(&rx_packet_buf[i]);
        rx_desc_buf[i].status = 0x00;
    }


    // Finally Enable receive
    rcr |= E1000_RCTL_EN;
    *rctl = rcr;

    // just try to send a packet
    // int err;
    // err = e1000_send("hello my 1st packect!\0", 23);
    // cprintf("Packet sent done with %d\n", err);
    // err = e1000_send("hello my 2nd packect!\0", 23);
    // cprintf("Packet sent done with %d\n", err);
    // err = e1000_send("hello my 3rd packect!\0", 23);
    // cprintf("Packet sent done with %d\n", err);

    return 0;
}

int
e1000_send(char* buf, uint32_t size)
{
    assert(size < TX_PACKET_SIZE);
    uint32_t tdt = *(uint32_t*)((char*)e1000_va + E1000_TDT);
    // debug
    // cprintf("index of tdt: [%d]\n", tdt);
    assert(tx_desc_buf[tdt].cmd & (E1000_TXD_CMD_RS >> E1000_TXD_CMD_SHIFT));
    if ((tx_desc_buf[tdt].status & (E1000_TXD_STAT_DD >> E1000_TXD_STAT_SHIFT)) == 0) return -E_TX_RUNOUT_BUF;

    memcpy(&tx_packet_buf[tdt], buf, size);
    tx_desc_buf[tdt].length = size;
    tx_desc_buf[tdt].cmd |= (E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP) >> E1000_TXD_CMD_SHIFT;
    *(uint32_t*)((char*)e1000_va + E1000_TDT) = (tdt + 1) % TX_DESC_BUF_NUM;
    // debug
    // cprintf("E1000: PACKET SENT [%d]\n", tdt);
    return 0;
}

int
e1000_receive(char* buf, uint32_t size)
{
    rx_desc_tail_reg rdt = *(rx_desc_tail_reg*) E1000_GET_REG(e1000_va, E1000_RDT);
    // wrap the index if it points to the one beyond the end of ring
    rdt = (rdt + 1) % RX_DESC_BUF_NUM;

    if ((rx_desc_buf[rdt].status & E1000_RXD_STAT_DD) == 0) 
        return -E_RX_EMPTY_BUF;
    assert(rx_desc_buf[rdt].status & E1000_RXD_STAT_EOP);

    if (rx_desc_buf[rdt].length > size) panic("e1000: receive try to overflow [%e]\n", -E_RX_LEN_TOO_LARGE);
    uint32_t len = rx_desc_buf[rdt].length;
    memcpy(buf, (void*)&rx_packet_buf[rdt], len);

    rx_desc_buf[rdt].status &= ~E1000_RXD_STAT_DD;
    *(rx_desc_tail_reg*) E1000_GET_REG(e1000_va, E1000_RDT) = rdt;
    //debug
    // cprintf("rdh[%d]rdt[%d]pktlen[%d]\n", *(rx_desc_head_reg*) E1000_GET_REG(e1000_va, E1000_RDH),*(rx_desc_tail_reg*) E1000_GET_REG(e1000_va, E1000_RDT), len);
    // cprintf("E1000: PACKET RECEIVED [%d]\n", rdt);

    return len;
}
