#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>


void readfromserver(int sockfd)
{
    char buffer[1024];
    memset(buffer, 0, 1024);

    int readerr = read(sockfd, buffer, 1024);
    if(readerr == -1)
    {
        perror("read error");
        exit(-1);
    }
    else if(readerr == 0)
    {
        printf("exiting\n");
        exit(0);
    }

    printf("%s\n", buffer);
}


int writetoserver(int sockfd)
{
    char input[1024];

    memset(input, 0, 1024);
    
    fgets(input, 128, stdin);

    int writeerr = write(sockfd, input, sizeof(input));

    if(writeerr == -1)
    {
        perror("write error");
        exit(-1);
    }
    else if(writeerr == 0)
    {
        printf("exiting");
        exit(0);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    
    if(argc != 3)
    {
        printf("useage: ./dbclient serverDNS port\n");
        exit(-1);
    }

    char* hostname = argv[1];
    char* service = "http";
    struct addrinfo hints;
    struct addrinfo* res;

    int port = atoi(argv[2]);

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(hostname, service, &hints, &res);


    //GETTING IP -------------------------

    void* addr;
   // char* ipver;
    char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_in* sockaddr;

    for(struct addrinfo* p = res; p != NULL; p = p->ai_next)
    {
        if(res->ai_family == AF_INET) //ipv4
        {
            printf("ipv4\n");
            sockaddr = (struct sockaddr_in*)res->ai_addr;

            sockaddr->sin_port = port;
            addr = &(sockaddr->sin_addr);
        }
        else //IPV6
        {
            printf("ipv6\n");
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)res->ai_addr;
            addr = &(ipv6->sin6_addr);
        }
    }
    

    inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
    printf("ip: %s\n", ipstr);
    printf("port: %i\n", sockaddr->sin_port);
    printf("port: %i\n", port);

    //CREATING SOCKET --------------------------

    int sockfd;
    sockfd = socket(res->ai_family, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        printf("failed to create socket\n");
        exit(-1);
    }

    printf("successfully created socket\n");

    //CONNECT TO SERVER ---------------------------

    int connect_error = connect(sockfd, (struct sockaddr*)sockaddr, sizeof(*sockaddr));
    if(connect_error == -1)
    {
        perror("connect error");
        exit(-1);
    }  
    
    printf("successfully connected\n");


    //READ/WRITE ------------------------------------

    for(;;)
    {
        readfromserver(sockfd);
        writetoserver(sockfd);

    }
   

    close(sockfd);
    return 0;   
}
