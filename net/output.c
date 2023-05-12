#include "inc/lib.h"
#include "inc/ns.h"
#include "inc/stdio.h"
#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
    while (true) {
        int r;
        envid_t src_env;

        if ((r = ipc_recv(&src_env, &nsipcbuf, NULL)) < 0)
            cprintf("%s: ipc_recv erro [%e]\n", binaryname, r);
        assert(src_env == ns_envid);

        assert(r == NSREQ_OUTPUT);

        //debug
        //cprintf("OUTPUT: receive a NSREQ_OUTPUT packet, send to e1000\n");

        r = sys_net_send(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len);
        
    }
}
