    /* Our socket information */
    struct addrinfo info;
/* Points to the results */
    struct addrinfo* servinfo;

    /* Prepare the struct info */
    memset (&info, 0, sizeof info); 
    info.ai_family = AF_UNSPEC;
    info.ai_socktype = SOCK_STREAM;
    info.ai_flags = AI_PASSIVE;

    /* Stores the result (success/failure) of the request */
    int status;

    /* Stores the ip address */
    char ipstr[INET6_ADDRSTRLEN];

    /* We pass NULL to getaddrinfo because we want to get info for the local
     * machine */
    if ((status = getaddrinfo (NULL, port, &info, &servinfo)) != 0)
    {
        fprintf (stderr, "Something went wrong! %s \n", gai_strerror (status));
        return (2);
    }

    /* Loop through our result ip addresses looking for a valid entry */
    struct addrinfo* p;
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        void* addr;
        char* ip_ver;

        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*) p->ai_addr;
            addr = &(ipv4->sin_addr);
            ip_ver = "IPv4";
        }
        else
        {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*) p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ip_ver = "IPv6";
        }

        inet_ntop (p->ai_family, addr, ipstr, sizeof (ipstr));
        printf ("   %s: %s \n", ip_ver, ipstr);

    }

    /* Get a socket descriptor */
    int sockfd = 
        socket (servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd == -1)
    {
        fprintf (stderr, "Could not create a socket to %s\n", servinfo->ai_family);
        freeaddrinfo (servinfo);
        return 2;
    }


