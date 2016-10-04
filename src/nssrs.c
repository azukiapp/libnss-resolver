/*
 * NSS plugin for looking up by extra nameservers
 */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <nss.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ares.h>

#include "resolver.h"
#include "debug.h"

#define ALIGN(a) (((a+sizeof(void*)-1)/sizeof(void*))*sizeof(void*))

static int
pack_hostent(struct hostent *result,
        char *buffer,
        size_t buflen,
        const struct hostent *hent)
{
    const char *name = hent->h_name;
    const void *addr = hent->h_addr_list[0];
    char **aliases, *r_addr, *addrlist;
    size_t l, idx, aidx;

    /* we can't allocate any memory, the buffer is where we need to
     * return things we want to use
     *
     * 1st, the hostname */
    l = strlen(name);
    result->h_name = buffer;
    memcpy (result->h_name, name, l);
    buffer[l] = '\0';

    idx = ALIGN (l+1);

    /* 2nd, the aliases array */
    aliases = (char **)buffer + idx;
    int alias_cnt = 0;
    for(char** alias = hent->h_aliases; hent->h_aliases != NULL && *alias != NULL; alias++) alias_cnt++;
    idx += (alias_cnt+1) * sizeof(void*);
    for(alias_cnt = 0; hent->h_aliases != NULL && hent->h_aliases[alias_cnt] != NULL; alias_cnt++)
    {
    	if(idx >= buflen)
    		return NSS_STATUS_TRYAGAIN;
    	aliases[alias_cnt] = buffer + idx;
    	l = strlen(hent->h_aliases[alias_cnt]) + 1;
    	memcpy(aliases[alias_cnt], hent->h_aliases[alias_cnt], l);
    	idx += l;
    }
    aliases[alias_cnt] = NULL;
    result->h_aliases = aliases;


    result->h_addrtype = AF_INET;
    result->h_length = sizeof (struct in_addr);

    /* 3rd, address */
    r_addr = buffer + idx;
    memcpy(r_addr, addr, result->h_length);
    idx += ALIGN (result->h_length);

    /* 4th, the addresses ptr array */
    addrlist = buffer + idx;
    //FIXME
    ((char **) addrlist)[0] = r_addr;
    ((char **) addrlist)[1] = NULL;

    result->h_addr_list = (char **) addrlist;
    return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_resolver_gethostbyname2_r (const char *name,
        int af,
        struct hostent *result,
        char *buffer,
        size_t buflen,
        int *errnop,
        int *h_errnop)
{
    if (af != AF_INET) {
        *errnop = EAFNOSUPPORT;
        *h_errnop = NO_DATA;
        return NSS_STATUS_UNAVAIL;
    }

    debug("Query libnss-resolver: %s - %s", NSSRS_DEFAULT_FOLDER, (char *)name);
    struct hostent *hosts = nssrs_resolve(NSSRS_DEFAULT_FOLDER, (char *)name);

    if (!hosts || hosts->h_name == NULL) {
        *errnop = ENOENT;
        *h_errnop = HOST_NOT_FOUND;
        if (hosts) {
            ares_free_hostent(hosts);
        }
        debug("Host not found");
        return NSS_STATUS_NOTFOUND;
    }

    int ok = pack_hostent(result, buffer, buflen, hosts);
    ares_free_hostent(hosts);

    return ok;
}

enum nss_status
_nss_resolver_gethostbyname_r (const char *name,
        struct hostent *result,
        char *buffer,
        size_t buflen,
        int *errnop,
        int *h_errnop)
{
    return _nss_resolver_gethostbyname2_r(name,
            AF_INET,
            result,
            buffer,
            buflen,
            errnop,
            h_errnop);
}

enum nss_status
_nss_resolver_gethostbyaddr_r (const void *addr,
        socklen_t len,
        int af,
        struct hostent *result,
        char *buffer,
        size_t buflen,
        int *errnop,
        int *h_errnop)
{
    if (af != AF_INET) {
        *errnop = EAFNOSUPPORT;
        *h_errnop = NO_DATA;
        return NSS_STATUS_UNAVAIL;
    }

    if (len != sizeof (struct in_addr)) {
        *errnop = EINVAL;
        *h_errnop = NO_RECOVERY;
        return NSS_STATUS_UNAVAIL;
    }

	const struct in_addr *address = addr;
	char addrstr[INET6_ADDRSTRLEN];
	
	sprintf(addrstr, "%s", inet_ntoa(*address));
    debug("Query libnss-resolver: %s - %s", NSSRS_DEFAULT_FOLDER, addrstr);

    struct hostent *hosts = nssrs_resolve(NSSRS_DEFAULT_FOLDER, addrstr);

    if (!hosts || hosts->h_name == NULL) {
        *errnop = ENOENT;
        *h_errnop = HOST_NOT_FOUND;
        if (hosts) {
            ares_free_hostent(hosts);
        }
        debug("Host not found");
        return NSS_STATUS_NOTFOUND;
    }

    int ok = pack_hostent(result, buffer, buflen, hosts);
    ares_free_hostent(hosts);
    
    if(ok == NSS_STATUS_TRYAGAIN)
    {
    	*errnop = ERANGE;
    	*h_errnop = TRY_AGAIN;
    }
    return ok;
}
