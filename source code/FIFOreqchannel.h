
#ifndef _FIFOreqchannel_H_
#define _FIFOreqchannel_H_

#include "common.h"

class FIFORequestChannel
{
public:
	enum Side {SERVER_SIDE, CLIENT_SIDE};
	enum Mode {READ_MODE, WRITE_MODE};
	
private:
	
	
	string my_name;
	Side my_side;
	
	int wfd;
	int rfd;
	
	string pipe1, pipe2;
	int open_pipe(string _pipe_name, int mode);
	
public:
	FIFORequestChannel(const string _name, const Side _side);
	
	~FIFORequestChannel();

	int cread (void* msgbuf, int bufcapacity);
	
	int cwrite (void *msgbuf , int msglen);
	 
	string name(); 
};

#endif
