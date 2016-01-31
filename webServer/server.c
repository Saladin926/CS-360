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
#include <sstream>
#include "cs360utils.h"
#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define MESSAGE             "This is the message I'm sending back and forth"
#define QUEUE_SIZE          5
#define BUFFER_SIZE2         10000
using namespace std;
void serve(int hSocket,string prepend)
{
    bool servedFile = false;
    vector<char*> headers;
        //parse headers
        string filePath = "";
        GetHeaderLines(headers, hSocket, false);
        for(int i = 0; i < headers.size(); i++)
        { 
            if(strstr(headers[i], "HTTP_GET") != NULL)
            {
                int slashPos = 0;
                string filePointer = headers[i];
                slashPos = filePointer.find("/",slashPos);

                size_t foundSpace = filePointer.find_last_of(" ");
                slashPos += 1;
                filePath = filePointer.substr(slashPos,foundSpace - slashPos);

                cout << "This is the file Path: " << filePath << endl;
            }
        }
        filePath = prepend +"/"+ filePath;
        cout << "File path: " << filePath << endl;
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

        cout << "This should be the content type: " << contentType << endl;
        stringstream ss;
        //use stat to determine type of request
        
        struct stat filestat = {};
        //check if it's possibly a directory
        if(filePath == "")
        {
            filePath = ".";
        }
        const char* charFilePath = filePath.c_str();
        if(stat(charFilePath, &filestat)) {
            cout <<"ERROR in stat\n";
        }
        if(S_ISREG(filestat.st_mode)) {
            cout << filePath << " is a regular file \n";
            cout << "file size = "<<filestat.st_size <<"\n";
            FILE *fp;

            if(fileExt == "jpg" || fileExt == "gif")
            {
                fp = fopen(charFilePath, "rb");
                
            }
            else
            {
                fp = fopen(charFilePath, "r");

            }
            char *buffer = (char*)  malloc(filestat.st_size);
            fread(buffer, 1, filestat.st_size, fp);
            //printf("Got\n%s", buffer); //for web server just print to socket instead of the screen
            ss << "HTTP/1.1 200 OK\r\n" << contentType << "\r\n" << "Content-Length: " << filestat.st_size << "\r\n\r\n";
            const string headerString = ss.str();
            cout << headerString << "\n";
            write(hSocket, headerString.c_str(), headerString.size());
            write(hSocket, buffer, filestat.st_size);
            //write(hSocket, buffer, sizeof(buffer));
            free(buffer);
            fclose(fp);
        }
        if(S_ISDIR(filestat.st_mode)) {
            cout << filePath << " is a directory \n";
            DIR* dirp;
            struct dirent *dp;

            dirp = opendir(charFilePath);
            char pBuffer[BUFFER_SIZE2];
            stringstream ssforIndex;
            bool indexFound = false;
            while((dp = readdir(dirp)) != NULL)
            {
                memset(pBuffer,0,sizeof(pBuffer));
                printf("<a href = \"%s\">%s</a><br>\n",dp->d_name,dp->d_name); 
                sprintf(pBuffer,"<a href = \"%s\">%s</a><br>\n",dp->d_name,dp->d_name);
                ssforIndex << pBuffer;
                if(strcmp(dp->d_name, "index.html") == 0)
                {
                    indexFound = true;
                    break;
                }
            }
            if(indexFound == true)
            {
                struct stat newfilestat = {};
                string newcharFilePath = filePath + "/index.html";
                const char* nCharFilePath = newcharFilePath.c_str();

                if(stat(nCharFilePath, &newfilestat)) {
                cout <<"ERROR in stat\n";
                }
                if(S_ISREG(newfilestat.st_mode)) {
                   
                    FILE *fp = fopen(nCharFilePath, "r");

                    char *buffer = (char*)  malloc(newfilestat.st_size);
                    fread(buffer, 1, newfilestat.st_size, fp);
                    //printf("Got\n%s", buffer); //for web server just print to socket instead of the screen
                    ss << "HTTP/1.1 200 OK\r\n" << "Content-Type: text/html" << "\r\n" << "Content-Length: " << newfilestat.st_size << "\r\n\r\n";
                    const string headerString = ss.str();
                    cout << headerString << "\n";
                    write(hSocket, headerString.c_str(), headerString.size());
                    write(hSocket, buffer, newfilestat.st_size);
                    //write(hSocket, buffer, sizeof(buffer));
                    free(buffer);
                    fclose(fp);
                }

            }
            else
            {
                stringstream headersForIndex;
                string findIndex = ssforIndex.str();
                headersForIndex << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n" << "Content-Length: " << findIndex.size() << "\r\n\r\n";
                write(hSocket, headersForIndex.str().c_str(), headersForIndex.str().size());
                write(hSocket, findIndex.c_str(), findIndex.size());
            }
            
            servedFile = true;
            (void)closedir(dirp);
        } 
        if(servedFile == false)
        {

            string notFound = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 13\r\n\r\n404 Not Found";
            write(hSocket, notFound.c_str(), notFound.size());
        }
        //- folder
        //- regular file
        //- invalid (return 404)
        //if not valid, return canned 404 response
        //if valid file, use stat to find file size
        //if folder, return index.html if it exists, if not return a directory listing (html links)
        
       
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
        //read(hSocket,pBuffer,BUFFER_SIZE);
        printf("got from browser \n %s\n",pBuffer);

        //call server function
        serve(hSocket,argv[2]);
       
        /* close socket */
        if(close(hSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
    }
}
