#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "FIFOreqchannel.h"
#include "TCPreqchannel.h"
#include <thread>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;


/*FIFORequestChannel* create_new_channel (FIFORequestChannel* mainchan){
    char name [1024];
    MESSAGE_TYPE m = NEWCHANNEL_MSG;
    mainchan->cwrite (&m, sizeof(m));
    mainchan->cread (name, 1024);
    FIFORequestChannel* newchan = new FIFORequestChannel (name, FIFORequestChannel::CLIENT_SIDE);
    return newchan;
}*/

void patient_thread_function(int n, int pno, BoundedBuffer* request_buffer, int mb ){
    datamsg d(pno, 0.0, 1);
    double resp = 0;
    for (int i = 0; i < n; i++){
        request_buffer-> push ((char *) &d, sizeof (datamsg));
        d.seconds += 0.004;
    }
}

void file_thread_function(string fname, BoundedBuffer* request_buffer, TCPRequestChannel* chan, int mb){
    //1. create the file 
    string recvfname = "received/" + fname;
    //make it as long as the original length
    char buf [1024];
    filemsg f(0,0);
    memcpy (buf, &f, sizeof(f));
    strcpy (buf + sizeof(f), fname.c_str());
    chan->cwrite (buf, sizeof(f) + fname.size() + 1);
    __int64_t filelength;
    chan->cread (&filelength, sizeof(filelength));
    FILE* fp = fopen(recvfname.c_str(), "w");
    fseek (fp, filelength, SEEK_SET);
    fclose(fp);


    //2. generate all the file messages
    filemsg* fm = (filemsg *) buf;
    __int64_t remlen = filelength;
    while (remlen > 0){
        fm->length = min (remlen, (__int64_t) mb);
        request_buffer-> push(buf, sizeof(filemsg) + fname.size() + +1);
        fm->offset += fm->length;
        remlen -= fm->length;

    }
}

void worker_thread_function(TCPRequestChannel* chan, BoundedBuffer* request_buffer, HistogramCollection* hc, int mb){
    char buf[1024];
    double resp = 0;

    char recvbuf [mb];
    while (true){
        request_buffer->pop(buf, 1024);
        MESSAGE_TYPE* m = (MESSAGE_TYPE *) buf;


        if(*m == DATA_MSG){
            chan->cwrite(buf, sizeof(datamsg));
            chan->cread(&resp, sizeof(double));
            hc->update(((datamsg *)buf)->person, resp);
        }

        else if(*m == QUIT_MSG){
            chan->cwrite(m,sizeof(MESSAGE_TYPE));
            delete chan;
            break;

        }
        else if(*m == FILE_MSG){
            filemsg* fm = (filemsg*) buf;
            string fname = (char *) (fm + 1);
            int sz = sizeof(filemsg) + fname.size() + 1;
            chan->cwrite(buf,sz);
            chan->cread (recvbuf, mb);

            string recvfname = "recv/" + fname;

            FILE* fp = fopen (recvfname.c_str(), "r+");
            fseek (fp, fm->offset ,SEEK_SET);
            fwrite(recvbuf, 1, fm->length, fp);
            fclose(fp);
        }
    }
}






int main(int argc, char *argv[])
{
    string fname;
    int n = 100;    //default number of requests per "patient"
    int p = 10;     // number of patients [1,15]
    int w = 100;    //default number of worker threads
    int b = 100; 	// default capacity of the request buffer, you should change this default
	int m = MAX_MESSAGE; 	// default capacity of the message buffer
    srand(time_t(NULL));
    string host;
    string port;
    int opt = -1;
    while ((opt = getopt(argc, argv, "m:n:b:w:p:f:h:r:")) != -1){
        switch (opt){
            case 'm':
                m = atoi (optarg);
                break;
            case 'n':
                n = atoi (optarg);
                if(n < 1 || n > 15000){
                    EXITONERROR("number of data itmes must be in range of [1,15000] ");
                }
                break;
            case 'p':
                p = atoi (optarg);
                if(p < 1 || p > 15){
                    EXITONERROR("number of patients must be in range of [1,15] ");
                }
                break;
            case 'b':
                b = atoi (optarg);
                if(b < 1){
                EXITONERROR("bounded buffer size must be in range of [1 to a few hundred] ");
                }
                break;
            case 'w':
                w = atoi (optarg);
                break;
            case 'f':
                fname = string(optarg);
                break;
            case 'h':
                host = string(optarg);
                break;
            case 'r':
                port = string(optarg);
                if(atoi(optarg) < 1024 || atoi(optarg) > 65535){
                    EXITONERROR("port number must be in the range of [1024,65535] ");
                }
                break;
        }
    }


    
	TCPRequestChannel* chan = new TCPRequestChannel(host, port, FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
	HistogramCollection hc;
	
    //making histograms and adding to the histogram collection hc
	for (int i = 0; i < p; i++){
        Histogram *h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }
    
    //make w worker channels (make sure to do it sequentially in the main)
    TCPRequestChannel* wchans [w];
    for (int i = 0; i < w; i++){
        wchans[i] = new TCPRequestChannel(host, port, FIFORequestChannel::CLIENT_SIDE);
    }


    struct timeval start, end;
    if (fname.size() > 0){ // handles file request
        gettimeofday(&start, 0);

        /* file thread  */
        thread filethread(file_thread_function, fname, &request_buffer, chan, m);
        
        /*worker threads*/
        thread workers[w];
        for (int i = 0; i < w; i++){
            workers[i] = thread(worker_thread_function, wchans[i], &request_buffer, &hc, m);
        }

        /*join file thread*/
        filethread.join();
        cout << "File thread finished" << endl;

        /* close channels */
        for (int i = 0; i < w; i++){
            MESSAGE_TYPE m = QUIT_MSG;
            request_buffer.push((char *)&m, sizeof(MESSAGE_TYPE));
        }

        /*join threads*/
        for (int i = 0; i < w; i++){
            workers[i].join();
        }
        cout << "worker threads finished" << endl;
        gettimeofday(&end, 0);
    }
    else{ // handles data message requests
        gettimeofday(&start, 0);
        
        //patient threads 
        thread patient[p];
        for (int i = 0; i < p; i++){
        patient [i] = thread (patient_thread_function, n, i+1, &request_buffer, m);
        }

        //worker threads
        thread workers[w];
        for (int i = 0; i < w; i++){
            //cout << "workers[" << i << "] is doing: ";
            workers[i] = thread(worker_thread_function, wchans[i], &request_buffer, &hc, m);
            //cout << endl;
        }

        //Join patient threads
        for (int i = 0; i < p; i++){
            patient [i].join();
        }
        cout << "Patient threads finished" << endl;

        //close channels
        for (int i = 0; i < w; i++){
            MESSAGE_TYPE m = QUIT_MSG;
            request_buffer.push((char *)&m, sizeof(MESSAGE_TYPE));
        }

        //Join worker threads
        for (int i = 0; i < w; i++){
            workers[i].join();
        }
        cout << "worker threads finished" << endl;
        gettimeofday(&end, 0);
    }

    // print the results
	hc.print ();
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
    

    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    wait(0);
    cout << "All Done!!!" << endl;
    delete chan;
    
}
