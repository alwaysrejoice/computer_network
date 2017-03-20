/* Hw2 reliable unicast using UDP
student: Hui-Jou Chou
starting date: 3/17/2017
 todo: 1.timeout
       2. check duplicate
       3. makefile
 	   4. readme
       5. test case
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "function.h"

//int recieve_port=6000;
int loss_percent=0;
int cnt;
char* reci_ip="127.0.0.1";



int main(int argc, char* argv[]){
    struct timeval tv;
    tv.tv_sec=5;
    tv.tv_usec=0;
    int ret=parse_cmd_args(argc, argv);
    if(ret==-1)
    {
        exit(0);
    }else{
        loss_percent=ret;
    }
    pack_list sent_pack;
    init_pack(&sent_pack);

    struct sockaddr_in myaddr;
    struct sockaddr_in remaddr;
    socklen_t addrlen=sizeof(remaddr);
    int recvlen;
    int fd;
    int msgcnt=0;
    char buf[DATALEN];
    package* data_get;
    /* create a UDP socket */
    if((fd= socket(AF_INET, SOCK_DGRAM, 0)) <0){
        perror("cannot create socket\n");
        return 0;
    }
    /* bind the socket to any valid IP address and a specific port */
    memset((char*)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    myaddr.sin_port = htons(recieve_port);
    
    if(bind(fd, (struct sockaddr*)&myaddr, sizeof(myaddr))<0){
        perror("bind failed");
        return 0;
    }
    printf("waiting on port %d\n", recieve_port);
    while(1){
        
        if(setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0)
        {
            perror("Error");
        }
        memset(buf,0, DATALEN);
        recvlen = recvfrom(fd, buf,DATALEN,0,(struct sockaddr*)&remaddr,&addrlen);
        //printf("%s\n",buf);
        if(recvlen>0){
            buf[recvlen]=0;
            //simulate data loss environment
            int is_loss=simulate_unreliable(loss_percent);
            if(is_loss==1){
                //check whether or not getting total packages
            	if(strcmp(buf,"check")==0){
                    printf("server msg: check\n");
                	char nack_msg[DATALEN];
                	int is_all=check_package(&sent_pack, nack_msg);
                    //send nack message
                	if(is_all==1){
                        printf("sending: %s\n",nack_msg);
                    	cnt=sendto(fd, nack_msg,strlen(nack_msg),0,(struct sockaddr *) &remaddr, sizeof(remaddr));
                    	if(cnt<0)
                    	{
                        	perror("sendto");
                        	exit(1);
                    	}

                	}
                    //send ack message
                	else if(is_all==0)
                	{
                        printf("sending : ack\n");
                    	char ack[]="ack";
                    	cnt=sendto(fd, ack,strlen(ack),0,(struct sockaddr *) &remaddr, sizeof(remaddr));
                    	if(cnt<0)
                    	{
                        	perror("sendto");
                        	exit(1);
                    	}
                	}
                    else
                    {
                        printf("sending : nack, no data get\n");
                        char noack[]="nack, no data";
                        cnt=sendto(fd, noack,strlen(noack),0,(struct sockaddr *) &remaddr, sizeof(remaddr));
                        if(cnt<0)
                        {
                            perror("sendto");
                            exit(1);
                        }

                    }
            	}
                //confirm all data get and output file.
            	else if(strcmp(buf,"all finished")==0){
                    printf("%s\n",buf);
                	FILE *fp;
                	parse_chunk_back_to_file(&sent_pack, fp);
                	break;
            	}
            	else
            	{
               		data_get=deserialize_pack(buf);
                    //printf("%s\n",data_get->data);
                    printf("package %d got, total is %d\n",data_get->order, data_get->size);
                    int is_dup=check_duplicate_data(&sent_pack, data_get);
                    if(is_dup==0) add_pack_list_re(&sent_pack, data_get);
                    else printf("this is duplicate data package, drop it!\n");
            	}
        	}
            else
            {
                printf("pretend data loss\n");
            }
        }
        else{
            printf("5 secs timeout\n");
            printf("no sender send packages yet.\n");
            printf("continue to wait...\n");
        }
    }
    printf("shut down!!\n");
    close(fd);
    return 0;
    
    
}
