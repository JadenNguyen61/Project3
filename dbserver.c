#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include "msg.h"



void* handler(void* arg)
    {
        //Output Messages
        char* message = "Enter your choice (1 to put, 2 to get, 0 to quit): ";
        char* namemessage = "Enter the name: ";
        char* idmessage = "Enter the id: ";
        char* getmessage = "Enter the id: ";
        char* errmessage = "unrecognized request, put another input to reset: ";
        char* errorput = "Put failed";
        char* successput = "Put success\n";

        int* clientfdptr = (int*)arg;
        int clientfd = *clientfdptr;
        
        
        struct record user; 
        
        int32_t dbfd;
        // open input file
        dbfd = open("db", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
        if (dbfd == -1) {
        perror("open failed");
        exit(EXIT_FAILURE);
        }
        
        
        for(;;)
        {

            write(clientfd, message, strlen(message));
            char buffer[1024];
            memset(buffer, 0, 1024);

            read(clientfd, &buffer, 1024);

            int input = atoi(buffer);

            
            int closed = 0;
            struct record readuser;
            switch(input)
            {
                case 0: //EXIT
                    closed = 1;
                    break;
                case 1: //PUT
                    //Name message
                    write(clientfd, namemessage, strlen(namemessage));
                    ssize_t name = read(clientfd, &buffer, sizeof(buffer) + 1);
                    strcpy(user.name, buffer);
                    
                    
                    //ID Message
                    write(clientfd, idmessage, strlen(idmessage));
                    uint32_t userID = read(clientfd, &buffer, sizeof(buffer));
                    user.id = atoi(buffer);
                    
                    //Error check
                    if(name == -1|| userID == -1){
                      write(clientfd, errorput, strlen(errorput));
                      break;
                    }else{
                    //Store in file
                      lseek(dbfd, 0, SEEK_END);
                      write(clientfd, successput, strlen(successput));
                      
                      write(dbfd, &user, sizeof(user));
                    }
                    
                    
                    
                    break;
                case 2: //GET
                    
                    //User Input
                    write(clientfd, getmessage, strlen(getmessage));
                    read(clientfd, &buffer, sizeof(buffer));
                    
                    //Find file size
                    int dbsize = lseek(dbfd, 0, SEEK_END);
                    int targetid = atoi(buffer);
                    for(int i = 0; i < dbsize / sizeof(user); i++){
                      lseek(dbfd, i*sizeof(user), SEEK_SET);
                      read(dbfd, &readuser, sizeof(readuser));
                      
                    //Finding if id = target 
                      if(readuser.id == targetid){
                        break;
                      }
                    }
                    //Formatting
                    sprintf(buffer, "Name:%sID:%u\n", readuser.name, readuser.id);
                    write(clientfd, buffer, strlen(buffer));
                    
                    
                    
                    break;
                default: //else
                    write(clientfd, errmessage, strlen(errmessage));
                    read(clientfd, &buffer, 1024);
                    break;

            }

            if(closed != 0)
            {
                break;
            }

        }
        

        close(clientfd);

        return NULL;
    }

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("useage: ./dbserver port\n");
        exit(-1);
    }


    char* hostname = "cs2.utdallas.edu";
    char* service = "http";
    struct addrinfo hints;
    struct addrinfo* res;

    int port = atoi(argv[1]);

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(hostname, service, &hints, &res);


    //GETTING IP -------------------------

    void* addr;

    struct sockaddr_in* sockaddr;
   // char* ipver;
    char ipstr[INET6_ADDRSTRLEN];

    for(struct addrinfo* p = res; p != NULL; p = p->ai_next)
    {
        if(res->ai_family == AF_INET) //ipv4
        {
            printf("ipv4\n");
            sockaddr = (struct sockaddr_in*)res->ai_addr;

            sockaddr->sin_port = port;
            addr = &(sockaddr->sin_addr);
        }
    }
    

    inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
    printf("ip: %s\n", ipstr);


    //CREATING SOCKET --------------------------

    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        printf("failed to create socket\n");
        exit(-1);
    }

    printf("successfully created socket\n");

    //BIND SOCKET --------------------------

    int bind_error = bind(sockfd, (struct sockaddr*)sockaddr, sizeof(*sockaddr));
    if(bind_error == -1)
    {
        perror("bind failed");
        exit(-1);
    }

    printf("successfully bound socket\n");

    for(;;)
    {
        //LISTEN ------------------------------------

        int listen_error = listen(sockfd, 1024);
        if(listen_error == -1)
        {
            perror("Listen error");
            exit(-1);
        }

        printf("successfully listening\n");

        //ACCEPT CLIENT ----------------------------


        struct sockaddr* clientaddr = malloc(sizeof(struct sockaddr));
        if(clientaddr == NULL)
        {
            printf("malloc error for clientaddr\n");
            exit(-1);
        }

        memset(clientaddr, 0, sizeof(*clientaddr));


        socklen_t addrlen = sizeof(*clientaddr);
        int clientfd = accept(sockfd, clientaddr, &addrlen);
        if(clientfd == -1)
        {
            perror("accept error");
            exit(-1);
        }
        printf("successfully accepted\n");

        pthread_t p1;
        pthread_create(&p1, NULL, handler, &clientfd);
    }
    


   

    return 0;
}