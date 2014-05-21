// An interface inspired by RFC 1149
// Written by Shuo Chen.
// This file is in public domain.

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/mbuf.h>
#include <sys/buf.h>
#include <sys/dkstat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/tty.h>
#include <sys/kernel.h>
#include <sys/conf.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/netisr.h>
#include <net/route.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>

struct	ifnet pigeonif;
struct	ifqueue pigeon_out_queue;

void enqueue(struct ifqueue *inq, struct mbuf *m);
struct mbuf *dequeue(struct ifqueue *inq);

int pigeon_dequeue(char *buf, int len)
{
	int copied = 0;
	struct mbuf *m = dequeue(&pigeon_out_queue);
	if (m) {
		copied = m_copydata(m, 0, len, buf);
		m_freem(m);
	}
	return copied;
}

int
pigeonoutput(ifp, m, dst, rt)
	struct ifnet *ifp;
	register struct mbuf *m;
	struct sockaddr *dst;
	register struct rtentry *rt;
{
	enqueue(&pigeon_out_queue, m);
	return 0;
}

/*
 * Process an ioctl request.
 */
int
pigeonioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	u_long cmd;
	caddr_t data;
{
	int error = 0;
	switch (cmd) {
	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		/*
		 * Everything else is done at a higher level.
		 */
		break;
	default:
		error = EINVAL;
	}
	return error;
}

void pigeonattach(int n)
{
	pigeon_out_queue.ifq_maxlen = IFQ_MAXLEN;
	register struct ifnet *ifp = &pigeonif;
	ifp->if_name = "pg";
	ifp->if_mtu = 1500;
	ifp->if_flags = 0;  // IFF_POINTOPOINT;
	ifp->if_ioctl = pigeonioctl;
	ifp->if_output = pigeonoutput;
	ifp->if_type = IFT_PIGEON;
	ifp->if_hdrlen = 0;
	ifp->if_addrlen = 0;
	if_attach(ifp);
}

