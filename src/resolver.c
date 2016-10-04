#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ares.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "resolver.h"
#include "files.h"
#include "debug.h"

static void wait_ares(ares_channel channel) {
    for(;;){
        struct timeval *tvp, tv;
        fd_set read_fds, write_fds;
        int nfds;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        nfds = ares_fds(channel, &read_fds, &write_fds);
        if(nfds == 0) {
            break;
        }
        tvp = ares_timeout(channel, NULL, &tv);
        select(nfds, &read_fds, &write_fds, NULL, tvp);
        ares_process(channel, &read_fds, &write_fds);
    }
}

static char **copy_list(char **list) {
    char **p;
    char **new_list = NULL;
    int count = 0;
    int index = 0;

    // Alias
    for (p = list; *p; p++) {
        count++;
    }

    debug("list size: %d", count);
    new_list = malloc((count+1) * sizeof(char *));
    for (p = list; *p; p++) {
        new_list[index] = malloc(sizeof(struct in_addr));
        if (new_list[index]) {
          memcpy(new_list[index], *p, sizeof(struct in_addr));
        }
        index++;
    }
    new_list[index] = NULL;

    return new_list;
}

static char ** copy_string_list(char ** list)
{
	char ** new_list = NULL;
	int count = 0;
	for(char** s = list; list != NULL && *s != NULL; s++) count++;
	new_list = malloc((count+1) * sizeof(char*));
	for(count = 0; list != NULL && list[count] != NULL; count++)
		new_list[count] = strdup(list[count]);
	new_list[count] = NULL;
	return new_list;
}

void nssrs_copy_hostent(struct hostent *from, struct hostent *to) {
    if (from->h_name != NULL) {
        to->h_name = strdup(from->h_name);
        to->h_addrtype  = from->h_addrtype;
        to->h_length    = from->h_length;
        to->h_aliases   = copy_string_list(from->h_aliases);
        to->h_addr_list = copy_list(from->h_addr_list);
    }
}

void nssrs_init_hostent(struct hostent *h) {
    h->h_name      = NULL;
    h->h_addrtype  = 0;
    h->h_length    = 0;
    h->h_aliases   = NULL;
    h->h_addr_list = NULL;
}

static void callback(void *arg, int status, int timeouts, struct hostent *from) {
    struct hostent *to = (struct hostent *)arg;

    if(!from || status != ARES_SUCCESS){
        debug("Failed to lookup: %s\n", ares_strerror(status));
        return;
    }

    // Save return
    nssrs_copy_hostent(from, to);
}

struct hostent *nssrs_resolver_by_servers(char *name, char *nameserver) {
    ares_channel channel = NULL;
    int status, optmask = 0;
    struct ares_options options;
    struct hostent *results;
    struct in_addr inet_address;

    status = ares_library_init(ARES_LIB_INIT_ALL);
    if (status != ARES_SUCCESS) {
        debug("ares_library_init: %s\n", ares_strerror(status));
        return NULL;
    }

    optmask = ARES_OPT_SERVERS | ARES_OPT_UDP_PORT | ARES_OPT_TCP_PORT;
    memset(&options, 0, sizeof(options));
    options.servers  = NULL;
    options.nservers = 0;
    options.tcp_port = 53;
    options.udp_port = 53;
    options.flags    = ARES_FLAG_NOCHECKRESP;

    status = ares_init_options(&channel, &options, optmask);
    if(status != ARES_SUCCESS) {
        debug("ares_init_options: %s\n", ares_strerror(status));
        return NULL;
    }

    status = ares_set_servers_csv(channel, nameserver);
    if (status != ARES_SUCCESS) {
      debug("ares_set_servers_csv: %s\n", ares_strerror(status));
      ares_destroy(channel);
      ares_library_cleanup();
      return NULL;
    }

    // Wait resolver
    results = malloc(sizeof(struct hostent));
    nssrs_init_hostent(results);
    if(inet_aton(name, &inet_address) != 0)
    {
    	ares_gethostbyaddr(channel, &inet_address, sizeof(inet_address), AF_INET, &callback, results);
    }
    else
    {
	    ares_gethostbyname(channel, name, AF_INET, &callback, results);
	}
    wait_ares(channel);
    ares_destroy(channel);
    ares_library_cleanup();

	debug("h_name: %s", results->h_name);
	for(char** alias = results->h_aliases; results->h_aliases != NULL && *alias != NULL; alias++)
		debug("h_alias: %s", *alias);

    if (results->h_name != NULL) {
        return results;
    }

    free(results);
    return NULL;
}

struct hostent *nssrs_resolve(char *folder, char *domain) {
    struct hostent *results = NULL;
    char *file;
    struct in_addr inet_address;
    
    if(inet_aton(domain, &inet_address))
    {
    	/* Make PTR address from IPv4 */
    	char ptraddr[29];
    	memset(ptraddr, 0, sizeof(ptraddr));
    	char *dot = domain + strlen(domain);
    	while(dot >= domain)
    	{
    		if(dot[0] == '.' || dot == domain)
    		{
	    		char *octet = dot + 1;
    			if(dot == domain) octet = domain;
    			int len = (char*)strchrnul(octet, '.') - octet + 2;
	    		snprintf(ptraddr + strlen(ptraddr), len, "%s.", octet);
	    	}
	    	dot--;
    	}
    	strcat(ptraddr, "in-addr.arpa");
    	file = nssrs_getfile_by_sufix(folder, ptraddr);
    }
    else
    {
    	file = nssrs_getfile_by_sufix(folder, domain);
    }

    if (file) {
        debug("resolver file: %s", file);
        struct resolver_file *rf= nssrs_parse_routes(file);
        if (rf) {
            debug("resolver servers: %s", rf->servers);
            results = nssrs_resolver_by_servers(domain, rf->servers);
            nssrs_free(rf->servers);
            nssrs_free(rf);

#ifdef DEBUG
            if (results) {
                char ip[INET6_ADDRSTRLEN];
                int i = 0;

                for (i = 0; results->h_addr_list[i]; ++i) {
                    inet_ntop(results->h_addrtype, results->h_addr_list[i], ip, sizeof(ip));
                    debug("ip: %s", ip);
                }
            }
#endif
        }
        nssrs_free(file);
    }

    return results;
}

