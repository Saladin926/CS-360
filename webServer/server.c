#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "cs360utils.h"
#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define MESSAGE             "This is the message I'm sending back and forth"
#define QUEUE_SIZE          5

void serve(int hSocket)
{

    vector<char*> headers;
        //parse headers
        string filePath = "";
        GetHeaderLines(headers, hSocket, false);
        for(int i = 0; i < headers.size(); i++)
        {
            if(strstr(headers[i], "HTTP_Referer") != NULL)
            {
                int slashPos = 0;
                string filePointer = headers[i];
                for(int j = 0; j < 3 ; j++)
                {
                    slashPos = filePointer.find("/",slashPos);
                    slashPos += 1;

                }

                if(filePointer[slashPos] != '\0')
                {
                    filePath = filePointer.substr(slashPos);
                }

            }
        }
        cout << "***this should be the file path: " << filePath << endl;
        //set content type and content length dynamically
        //parse the get request for file extension
        //for the content type
        string fileExt = "";
        size_t found = filePath.find_last_of(".");
        fileExt = filePath.substr(found+1);

        string contentType = "";

        if(fileExt == "html")
        {
             contentType = "Content-Type: text/html";
        }
        else if(fileExt == "txt")
        {
            contentType = "Content-Type: text/plain";
        }
        else if(fileExt == "jpg")
        {
            contentType = "Content-Type: image/jpg";
        }
        else if(fileExt == "gif")
        {
            contentType = "Content-Type: image/gif";
        }

        cout << "***this should be the content type: " << contentType << endl;
        //use stat to determine type of request
        struct stat filestat;
        const char* charFilePath = filePath.c_str();
        if(stat(charFilePath, &filestat)) {
            cout <<"ERROR in stat\n";

        }
        if(S_ISREG(filestat.st_mode)) {
            cout << filePath << " is a regular file \n";
            cout << "file size = "<<filestat.st_size <<"\n";
            FILE *fp = fopen(charFilePath, "r");
            char *buffer = (char*)  malloc(filestat.st_size);
            fread(buffer, filestat.st_size, 1, fp);
            printf("Got\n%s", buffer); //for web server just print to socket instead of the screen
            free(buffer);
            fclose(fp);
        }
        if(S_ISDIR(filestat.st_mode)) {
        cout << filePath << " is a directory \n";
        DIR* dirp;
        struct dirent *dp;

        dirp = opendir(charFilePath);
        while((dp = readdir(dirp)) != NULL)
        printf("name %s\n", dp->d_name);
        (void)closedir(dirp);
        }
        //- folder
        //- regular file
        //- invalid (return 404)
        //if not valid, return canned 404 response
        //if valid file, use stat to find file size
        //if folder, return index.html if it exists, if not return a directory listing (html links)
        string msg = "HTTP/1.1 200 OK\r\nContent- Type: text/plain\r\nContent-Length: 3 \r\n\r\nhey guys!";
        write(hSocket,msg.c_str(), msg.size());
}

int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;

    if(argc < 2)
      {
        printf("\nUsage: server host-port\n");
        return 0;
      }
    else
      {
        nHostPort=atoi(argv[1]);
      }

    printf("\nStarting server");

    printf("\nMaking socket");
    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nBinding to port %d",nHostPort);
    int optval = 1;
    setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) 
                        == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }
 /*  get port number */
    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port) );

        printf("Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n"
              , Address.sin_family
              , Address.sin_addr.s_addr
              , ntohs(Address.sin_port)
            );


    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }

    for(;;)
    {
        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n",
        Address.sin_addr.s_addr,
        ntohs(Address.sin_port));
        memset(pBuffer, 0, sizeof(pBuffer));
        read(hSocket,pBuffer,BUFFER_SIZE);
        printf("got from browser \n %s\n",pBuffer);

        //call server function
        serve(hSocket);
       
        /* close socket */
        if(close(hServerSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
    }
}
