#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255
#define MAXGET 1000
#define PAGESIZE 700
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
    char page[PAGESIZE];

    if(argc > 6 || argc < 4)
    {
        printf("\nUsage: download [-c count | -d] host-name host-port path \n");
        return 0;
    }

    extern char* optarg;
    int c, times_to_download = 1, err = 0;
    bool debug = false,cflag = false;//cflag means to display body
    while((c = getopt(argc, argv, "c:d")) !=-1)
    {
        switch(c)
        {
            case 'c':
                times_to_download = atoi(optarg);
                cflag = true;
                break;
            case 'd':
                debug = true;
                break;
            case '?':
                err = 1;
                break;
        }
    }

    
   

    for(int i = 0; i < strlen(argv[optind+1]) ; i++)
    {
        if(!isdigit(argv[optind+1][i]))
        {
            printf("\ninvalidport\n");
            return 0;
        }

    }
    nHostPort=atoi(argv[optind+1]);
    strcpy(page,argv[optind+2]);
    strcpy(strHostName,argv[optind]);
    

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
    if(pHostInfo == NULL)
    {
        printf("\ninvalid host name\n");
        return 0;
    }
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
    sprintf(message, "GET %s HTTP/1.1\r\nHost:%s:%s\r\n\r\n",page,strHostName,nHostPort);
    
    //here we are going to allocate space with memory management
    //Send HTTP on the SOcket
    for(int i = 0 ; i < times_to_download ; i++)
    {
        write(hSocket,message,strlen(message));//this is an example of writing a simple line request to the server, we will need to parse
    }
     
    //Read Response back from Socket
    char* wholeMessage = (char*)malloc(1);
    wholeMessage[0] = '\0';
    do
    {
        nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);
        if(nReadAmount < BUFFER_SIZE) 
        {
            pBuffer[nReadAmount] = '\0';
        }
        totalRead+=nReadAmount;
        char* new_str;
        if((new_str = (char*)malloc(strlen(wholeMessage)+strlen(pBuffer)+1)) != NULL)
        {
            new_str[0] = '\0';   // ensures the memory is an empty string
            strcat(new_str,wholeMessage);
            strcat(new_str,pBuffer);

            free(wholeMessage);
            wholeMessage = (char*)malloc(strlen(new_str) + 1);
            strcpy(wholeMessage, new_str);
        }
        else
        {
            printf("\nMalloc for reading in the response absolutely failed!\n");
            // exit?
        }
        free(new_str);
    }
    while(nReadAmount >= BUFFER_SIZE);

    const char* delimeter = "\r\n\r\n";
    char* body = strstr(wholeMessage,delimeter);
    body[0] = '\0';
    body += 3;
    if(debug == true)
    {
        printf("\nRequest: %s\n~~\n", message);
        printf("\nResponse: %s\n~~Bytes read: %i~~\n",wholeMessage,totalRead);
        printf("\nHeader: %s\n",wholeMessage);
    }
    if(cflag == false)
    {
         printf("\nbody: %s\n",body);
    }
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