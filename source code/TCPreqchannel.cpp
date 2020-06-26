#include "TCPreqchannel.h"

int TCPRequestChannel::server (string port)
{
    struct addrinfo hints, *serv;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &serv)) != 0) {
        cerr  << "getaddrinfo: " << gai_strerror(rv) << endl;
        return -1;
    }
	if ((sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1) {
        perror("server: socket");
		return -1;
    }
    if (bind(sockfd, serv->ai_addr, serv->ai_addrlen) == -1) {
		close(sockfd);
		perror("server: bind");
		return -1;
	}
    freeaddrinfo(serv); // all done with this structure

    if (listen(sockfd, 20) == -1) {
        perror("listen");
        exit(1);
    }
	
	cout << "server: waiting for connections..." << endl;
    return 0;
}

int TCPRequestChannel::client( string host, string port)
{
    struct addrinfo hints, *res;

    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int status;
    //getaddrinfo("www.example.com", "3490", &hints, &res);
    if ((status = getaddrinfo(host.c_str(), port.c_str(), &hints, &res)) != 0)
    {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        return -1;
    }

    // make a socket:
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
    {
        perror("Cannot create scoket");
        return -1;
    }

    // connect!
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("Cannot Connect");
        return -1;
    }
    return 0;
}

