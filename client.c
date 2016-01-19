#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255
#define MAXGET 1000

int  main(int argc, char* argv[])
{
    int hSocket;                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;
    int totalRead = 0;

    if(argc < 6)
    {
        printf("\nUsage: myfile host-name host-port path -c or -d\n");
        return 0;
    }
    else
    {
        strcpy(strHostName,argv[1]);
        nHostPort=atoi(argv[2]);
    }

    printf("\nMaking a socket\n");
    /* make a socket */
    hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(hSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    /* connect to host */
    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address))
    == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }


    //Create HTTP Message
    char* message = (char*)malloc(MAXGET);
    sprintf(message, "GET %s HTTP/1.1\r\nHost:%s:%s\r\n\r\n",argv[3],argv[1],argv[2]);
    printf("\nRequest: %s\n~~\n", message);
    //here we are going to allocate space with memory management
    //Send HTTP on the SOcket
    write(hSocket,message,strlen(message));//this is an example of writing a simple line request to the server, we will need to parse
    //Read Response back from Socket
    char* wholeMessage = "";
    do
    {
        nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);
        if(nReadAmount < BUFFER_SIZE) {
                pBuffer[nReadAmount] = '\0';
        }
        totalRead+=nReadAmount;
        char* new_str;
        if((new_str = (char*)malloc(strlen(wholeMessage)+strlen(pBuffer)+1)) != NULL)
        {
            new_str[0] = '\0';   // ensures the memory is an empty string
            strcat(new_str,wholeMessage);
            strcat(new_str,pBuffer);

            wholeMessage = new_str;
        }
        else
        {
            printf("\nMalloc for reading in the response absolutely failed!\n");
            // exit?
        }
    }
    while(nReadAmount >= BUFFER_SIZE);


    printf("\nResponse: %s\n~~Bytes read: %i~~\n",wholeMessage,totalRead);
    /* write what we received back to the server */
    //write(hSocket,pBuffer,nReadAmount);
    //printf("\nWriting \"%s\" to server",pBuffer);

    printf("\nClosing socket\n");
    /* close socket */
    if(close(hSocket) == SOCKET_ERROR)
    {
        printf("\nCould not close socket\n");
        return 0;
    }
}