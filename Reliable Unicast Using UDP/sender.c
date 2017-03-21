/* hw2 reliable unicast using UDP
   student: Hui-Jou Chou
   starting date: 3/16/2017 */
/* todo:1.nack parse
 		2. timeout mechanism */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include "function.h"

//int recieve_port=6000;
char *reci_ip = "127.0.0.1";
const char ack[]="ack";
const char nack[]="nack";


int main(int argc, char* argv[]){
    struct timeval tv;
    tv.tv_sec=5;
    tv.tv_usec=0;

    struct sockaddr_in reci_addr;
    int sock,cnt;
    /*set up socket */
    sock = socket(AF_INET, SOCK_DGRAM,0);
    if(sock<0){
        perror("socket");
        exit(1);
    }
    
    char* file_name="rbprotocol-testcases.txt";
    int c;
    while((c=getopt(argc,argv,"p:f:h"))!=-1){
        switch (c) {
            case 'p':
            	recieve_port=atoi(optarg);
            	if((recieve_port<1) || (recieve_port>65535)){
                	printf("Error: invalid port range, you should enter port between 1 and 65535\n");
                	return 0;
            	}
            
            	break;
            case 'f':
            	file_name=optarg;
            	break;
            case 'h':
            	usage_se(argv[0]);
            	return -1;
            default:
            	break;
        }
        
        
    }
    bzero((char *)&reci_addr, sizeof(reci_addr));
    reci_addr.sin_family = AF_INET;
    reci_addr.sin_addr.s_addr = inet_addr(reci_ip);
    reci_addr.sin_port = htons(recieve_port);
    int slen=sizeof(reci_addr);
    FILE *fp=fopen(file_name,"rb");
    if(fp==NULL){
        printf("file open error\n");
        return 0;
    }
    pack_list sent_pack;
    
    init_pack(&sent_pack);
    printf("hi hi\n");
    parse_file_into_chunk(fp, &sent_pack);
    int nack_num=0;
    int nack_arr[DATALEN];
    while(1){
        if(nack_num>0)
        {
            for(int i=0; i<nack_num; i++)
            {
                for(int j=0; j<sent_pack.size; j++)
                {
                    if(sent_pack.arr[j].order==nack_arr[i])
                    {
                        printf("resending package %d/%d to reciever\n",sent_pack.arr[j].order,sent_pack.arr[j].size);
                        unsigned char buf[DATALEN], *ptr;
                        memset(buf,0, DATALEN);
                        ptr=serialize_pack(buf, sent_pack.arr+j);
                    	cnt=sendto(sock, buf,ptr-buf,0,(struct sockaddr *) &reci_addr, sizeof(reci_addr));
                		if(cnt<0)
                		{
                    		perror("sendto");
                    		exit(1);
                		}
                    }
                }

            }
        }
        else
        {
        	for(int i=0; i<sent_pack.size; i++)
        	{
                printf("sending package %d/%d to reciever\n",sent_pack.arr[i].order,sent_pack.arr[i].size);
                unsigned char buf[DATALEN], *ptr;
                memset(buf,0, DATALEN);
                ptr=serialize_pack(buf, sent_pack.arr+i);
        		cnt=sendto(sock,buf ,ptr-buf,0,(struct sockaddr *) &reci_addr, sizeof(reci_addr));
        		if(cnt<0)
            	{
            		perror("sendto");
            		exit(1);
        		}
        	}
        }
        int is_continue=1;
        int no_wait=0;
        int test_times=1;
        while(is_continue)
        {
            if(test_times<4)
            {
        		char buf[DATALEN];
                printf("request to check\n");
        		strcpy(buf,"check");
        		cnt=sendto(sock, buf,strlen(buf),0,(struct sockaddr *) &reci_addr, sizeof(reci_addr));
        		if(cnt<0)
        		{
            		perror("sendto");
            		exit(1);
        		}
				memset(buf,0, DATALEN);
        		if(setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0)
                {
            		perror("Error");
        		}
        		int recvlen=recvfrom(sock,buf, DATALEN,0,(struct sockaddr*)&reci_addr,&slen);
        		if (recvlen>=0)
        		{
            		buf[recvlen]=0;
            		printf("%s\n",buf);
            		if(strcmp(buf,ack)==0)
            		{
                		memset(buf,0, DATALEN);
                		strcpy(buf,"all finished");
                		printf("sending 'all finished' notice to reciever\n");
                		cnt=sendto(sock, buf,strlen(buf),0,(struct sockaddr *) &reci_addr, sizeof(reci_addr));
                		if(cnt<0)
                		{
                    		perror("sendto");
                    		exit(1);
                		}
             			nack_num=0;
                		is_continue=0;
            		}
                    else if(strcmp(buf,"nack, no data")==0)
                    {
                        is_continue=0;
                        no_wait=1;
                    }
            		else
            		{
                		nack_num=parse_nack_msg(buf, nack_arr);
                		is_continue=0;
                        no_wait=1;
            		}
        		}
                else
                {
            		printf("5 secs timeout\n");
            		printf("try send request again (%d)\n",test_times);
            		test_times++;
        		}
            }
            else
            {
                printf("already tried three times and there is no response. The reciever might shut down.\n");
                is_continue=0;
            }
            
        }
        if(is_continue==0&&no_wait==0)
        {
        	printf("waiting for 5 secs and will start to send data to a reciever.\n");
        	sleep(5);
            printf("Starting to send data...\n");
        }
    }


    close(sock);
    return(0);



}
