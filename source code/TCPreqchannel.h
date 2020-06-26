
#ifndef _TCPreqchannel_H_
#define _TCPreqchannel_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <thread>
#include "FIFOreqchannel.h"
using namespace std;

class TCPRequestChannel{

private:
    int sockfd;
    //int slave_socket;
    int clinet;
    int server(string port);
    int client(string host, string port);
    void talk_to_server (int sockfd);
    
    struct addrinfo hints;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;

public:
    TCPRequestChannel(const string _host, const string _port, int _side){
        if(_side == 0){ //server side
            server(_port);
        }
        else{ //client side
            client(_host, _port);
        }
    }

    TCPRequestChannel(int _s){
        sockfd = _s;
    }

    ~TCPRequestChannel(){
        close(sockfd);
    }

    int cread(void* buf, int len){
        return recv(sockfd, buf, len, 0);
    }

    int cwrite(void* buf, int len){
        return send (sockfd, buf, len, 0);
    }

    int getsocket(){
        return sockfd;
    }
    



};

#endif