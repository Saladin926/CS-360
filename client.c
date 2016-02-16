
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <iostream>
#include <cmath>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define HOST_NAME_SIZE      255
#define MAXGET 1000
#define NSOCKETS 10
#define NSTD 3
#define PAGESIZE 700

using namespace std;

int  main(int argc, char* argv[])
{
    int numSockets;
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;
    char page[PAGESIZE];
    bool printAll = false;
    double average = 0;
    float stdDev = 0;
    if(argc > 6 || argc < 5)
    {
        printf("\nUsage: webtest host port path count [-d]\n");
        return 0;
    }
    else
    {
        strcpy(strHostName,argv[1]);
        nHostPort=atoi(argv[2]);
        strcpy(page,argv[3]);
        numSockets = atoi(argv[4]);
    }
    int c;
    int err = 0;
    while((c = getopt(argc, argv, "d")) !=-1)
    {
        switch(c)
        {
            case 'd':
                printAll = true;
                break;
            case '?':
                err = 1;
                break;
        }
    }

    int hSocket[numSockets];                 /* handle to socket */
    struct timeval oldtime[numSockets+NSTD];

    printf("\nMaking a socket");
    /* make a socket */
    for(int i = 0; i < numSockets; i++) {
        hSocket[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(hSocket[i] == SOCKET_ERROR)
        {
            printf("\nCould not make a socket\n");
            return 0;
        }
    }

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    int epollFD = epoll_create(1);
    // Send the requests and set up the epoll data
    for(int i = 0; i < numSockets; i++) {
        /* connect to host */
        if(connect(hSocket[i],(struct sockaddr*)&Address,sizeof(Address)) 
           == SOCKET_ERROR)
        {
            printf("\nCould not connect to host\n");
            return 0;
        }
        //Create HTTP Message
        char* message = (char*)malloc(MAXGET);
        sprintf(message, "GET %s HTTP/1.1\r\nHost:%s:%d\r\n\r\n",page,strHostName,nHostPort);
        

        write(hSocket[i],message,strlen(message));
        // Keep track of the time when we sent the request
        gettimeofday(&oldtime[hSocket[i]],NULL);
        // Tell epoll that we want to know when this socket has data
        struct epoll_event event;
        event.data.fd = hSocket[i];
        event.events = EPOLLIN;
        int ret = epoll_ctl(epollFD,EPOLL_CTL_ADD,hSocket[i],&event);
        if(ret)
            perror ("epoll_ctl");
    }
    double findAverage;
    for(int i = 0; i < numSockets; i++) {
        struct epoll_event event;
        int rval = epoll_wait(epollFD,&event,1,-1);
        if(rval < 0)
            perror("epoll_wait");
        rval = read(event.data.fd,pBuffer,BUFFER_SIZE);
        struct timeval newtime;
        // Get the current time and subtract the starting time for this request.
        gettimeofday(&newtime,NULL);
        double usec = (newtime.tv_sec - oldtime[event.data.fd].tv_sec)*(double)1000000+(newtime.tv_usec-oldtime[event.data.fd].tv_usec);
        if(printAll)
        {
            std::cout << "Time "<<usec/1000000<<std::endl;
            printf("got %d from %d\n",rval,event.data.fd);
        }
        
        // Take this one out of epoll
        epoll_ctl(epollFD,EPOLL_CTL_DEL,event.data.fd,&event);
        findAverage += (usec/1000000);
        average = findAverage/i;
        stdDev += (pow(((usec/1000000)-average),2));
    }
    stdDev = stdDev/numSockets;
    cout << "Average: " << average << endl;
    cout << "Standard Deviation: " << stdDev << endl;
    // Now close the sockets
    for(int i = 0; i < numSockets; i++) {
        //printf("\nClosing socket\n");
        /* close socket */                       
        if(close(hSocket[i]) == SOCKET_ERROR)
        {
            printf("\nCould not close socket\n");
            return 0;
        }
    }
}
