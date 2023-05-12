#include "inc/assert.h"
#include "inc/lib.h"
#include "inc/mmu.h"
#include "inc/ns.h"
#include "inc/stdio.h"
#include "inc/string.h"
#include "ns.h"

#define PKTMAP 0x10000000
// tmp use
#define RX_PACKET_SIZE  2048
#define E_RX_EMPTY_BUF 1
#define E_RX_LEN_TOO_LARGE 2

extern union Nsipc nsipcbuf;


void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
    while (true) {
        union Nsipc* pkt = (union Nsipc*) PKTMAP;
        int r = sys_page_alloc(0, (void *)pkt, PTE_U | PTE_W | PTE_P);
        if (r < 0) panic("net input: run out of memory\n");

        while (true)
        {
            char buf[RX_PACKET_SIZE];
            r = sys_net_receive(buf, RX_PACKET_SIZE);
            if (r < 0)
            {
                if (r == -E_RX_EMPTY_BUF) 
                {
                    //debug
                    //cprintf("Receive Buffer Empty\n");
                    //cprintf(".");
                    sys_yield();
                    continue;
                }
                panic("net input: can not receive packet for %e", r);
            }
            //debug
            //cprintf("INPUT: PACKET received: [%d]\n", r);
            pkt->pkt.jp_len = r;
            memcpy(pkt->pkt.jp_data, buf, r);
            ipc_send(ns_envid, NSREQ_INPUT, (void*)pkt, PTE_P | PTE_U);
            break;
        }
        sys_page_unmap(0, (void *)pkt);
    }
}
