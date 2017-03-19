/* computer network assignment1, author:Hui-Jou Chou
	started date:2/16/2017 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>


void write_msg(int *sockfd);
void read_msg(int *sockfd);
void chomp(char *s);
char *username;
int threads_over=0;

int main(int argc, char *argv[]){
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    pthread_t threads[2];
    
    if(argc<4){
        printf("usage %s <host ip> <port> <name>\n",argv[0]);
        printf("default ip is 127.0.0.1, port is 6000, please type them\n");
        exit(0);
    }
    portno = atoi(argv[2]);
    username=argv[3];
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(portno);
    
    while(1){
        sockfd=socket(AF_INET,SOCK_STREAM, 0);
        if (sockfd < 0){
            printf("ERROR opening socket\n");
            exit(0);
        }

    if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr))<0)
    {
        printf("error connecting\n");
        
        printf("trying to connect to server again...\n");
        sleep(3);
    }else
        break;
    }
    
   //send username to server
    n = write(sockfd,username,strlen(username));
    if (n < 0) printf("ERROR writing to socket\n");

    
    
    
    int r1,r2;
    r1=pthread_create(&threads[0], NULL, (void*)&write_msg,&sockfd);
    r2=pthread_create(&threads[1],NULL, (void*)&read_msg,&sockfd);
    if(r1 || r2){
        printf("error, return code from pthread_create() is %d,%d\n",r1,r2);
        exit(-1);
    }
    while(1){
        if(threads_over==1) break;
    }
    close(sockfd);
    return 0;
    
}


void write_msg(int *sockfd){
    char buffer[1024];
    int n;
    int sockfd_s=*sockfd;
    char quit[6]="@exit";
    while(1){
    	bzero(buffer,1024);
        //printf("%s:",username);
    	fgets(buffer,1023,stdin);
    	n = write(sockfd_s,buffer,strlen(buffer));
        chomp(buffer);

       // printf("%s,%lu\n",buffer,strlen(buffer));
    	if (n < 0)
        	printf("ERROR writing to socket\n");
        if(strcmp(buffer,quit)==0){
            threads_over=1;
            break;
        	}
    	}

};

void read_msg(int *sockfd){
	char msg_get[1024];
    int n=0;
    int sockfd_s=*sockfd;
    while(1){
    	bzero(msg_get,1024);
    	n = read(sockfd_s,msg_get,1023);
    	if (n < 0)
        	printf("ERROR reading from socket\n");
        	//threads_over=1;
        else if(n>0)
        	printf("%s\n",msg_get);
        else{
            printf("Server disconnect\n");
            threads_over=1;
            break;
        }
    }
};





void chomp(char *s)
{
    while(*s && *s != '\n' && *s != '\r') s++;
    
    *s = 0;
}





