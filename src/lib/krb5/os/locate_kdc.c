/*
 * $Source$
 * $Author$
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <krb5/mit-copyright.h>.
 *
 * get socket addresses for KDC.
 */

#if !defined(lint) && !defined(SABER)
static char rcsid_locate_kdc_c[] =
"$Id$";
#endif	/* !lint & !SABER */

#include <krb5/copyright.h>
#include <krb5/krb5.h>
#include <krb5/osconf.h>

#include <krb5/ext-proto.h>
#include <stdio.h>
#include <krb5/libos-proto.h>

#include <sys/types.h>
#include <sys/socket.h>
#ifdef KRB5_USE_INET
#include <netinet/in.h>
#define KRB5_UDP_PORT	htons(8973)	/* XXX */
#endif
#include <netdb.h>
#include "os-proto.h"

/*
 * returns count of number of addresses found
 */

krb5_error_code
krb5_locate_kdc(realm, addr_pp, naddrs)
    krb5_data *realm;
    struct sockaddr **addr_pp;
    int *naddrs;
{
    char **hostlist;
    int code;
    int i, j, out, count;
    struct sockaddr *addr_p;
    struct sockaddr_in *sin_p;
    struct hostent *hp;
    
    hostlist = 0;
    
    if (code = krb5_get_krbhst (realm, &hostlist))
	return(code);

    for (i=0; hostlist[i]; i++)
	;
    count = i;
    
    if (count == 0) {
	*naddrs = 0;
	return 0;
    }

    addr_p = (struct sockaddr *)malloc (sizeof (struct sockaddr) * count);

    for (i=0, out=0; hostlist[i]; i++) {
	hp = gethostbyname(hostlist[i]);
	if (hp != 0) {
	    switch (hp->h_addrtype) {
#ifdef KRB5_USE_INET
	    case AF_INET:
		for (j=0; hp->h_addr_list[j]; j++) {
		    sin_p = (struct sockaddr_in *) &addr_p[out++];
		    memset ((char *)sin_p, sizeof(struct sockaddr), 0);
		    sin_p->sin_family = hp->h_addrtype;
		    sin_p->sin_port = KRB5_UDP_PORT;
		    memcpy((char *)&sin_p->sin_addr,
			   (char *)hp->h_addr_list[j],
			   sizeof(struct in_addr));
		    if (out >= count) {
			count *= 2;
			addr_p = (struct sockaddr *)
			    realloc ((char *)addr_p,
				     sizeof(struct sockaddr) * count);
		    }
		}
		break;
#endif
	    default:
		break;
	    }
	}
	free(hostlist[i]);
	hostlist[i] = 0;
    }
    free ((char *)hostlist);
				/*
				 * XXX need to distinguish between
				 * "can't resolve KDC name" and
				 * "can't find any KDC names"
				 */
    *addr_pp = addr_p;
    *naddrs = out;
    return 0;
}
