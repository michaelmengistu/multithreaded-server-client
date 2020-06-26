# multithreaded-server-client
Created a server that can store and send any data file extremely fast by using multi-threading. 
Also includes data files of 15 patients ECG values in the data base and makes a histogram of the data values of the patients to send to the client when requested. 

## How to run and use the server:
- place the server directory on a computer you want to host the server on.
- open a terminal in the server directory
- run the server executable file by typing `./server -r <assign port number>` on the command line
- to add files you want to store on the server, place them in the "BIMDC" directory

## How to run and use the client:
- place the client directory on a computer you want to access server on.
- open a terminal in the client directory.
- to request a histogram of the patients ECG values from the server, run the client executable file by typing `./client -n <number of ECG values>  -p <number of patients> -h <IP address of server> -r <assigned port number>` on the command line.
- to request a file from the server, run the client executable file by typing `./client -n <number of ECG values>  -p <number of patients> -h <IP address of server> -r <assigned port number>`
- The data files you received from server will be placed in "recv" directory.
