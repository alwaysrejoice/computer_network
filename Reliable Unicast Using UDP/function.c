//
//  function.c
//  hw2
//
//  Created by hui-jou chou on 3/18/17.
//
//

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

int recieve_port=6000;




void usage_se(char* argv)
{
    printf("There is a default test file which is rbprotocol-testcases.txt and a default port which is 6000\n");
    printf("but you can specify another port and new file.\n");
    printf("usage: %s [-p 1~65535] [-f string]\n", argv);
    printf("		-p <port>\n");
    printf("		-f <input file to be sent>\n");
    
}
void init_pack(pack_list* pack)
{
    memset(pack, 0, sizeof(pack_list));
}
void add_pack_list(pack_list* arr, char data[])
{
    //printf("debug %d\n",arr->size);
    if(arr->size==0)
    {
        arr->capacity=MAX_CAPACITY;
        arr->arr=(package*)malloc(sizeof(package)*MAX_CAPACITY);
        
    }
    if(arr->size>=MAX_CAPACITY)
    {
        printf("The file is too big, over the default memory capacity\n");
        exit(1);
    }
    strcpy(arr->arr[arr->size].data,data);
    arr->arr[arr->size].order=arr->size+1;
    arr->size++;
}
unsigned char * serialize_int(unsigned char *buffer, int value)
{
        /* Write big-endian int value into buffer; assumes 32-bit int and 8-bit char. */
        buffer[0] = value >> 24;
        buffer[1] = value >> 16;
        buffer[2] = value >> 8;
        buffer[3] = value;
        return buffer + 4;
}
           
unsigned char * serialize_char(unsigned char *buffer, char value[])
{
        strcpy(buffer,value);
        return buffer + strlen(value);
}
           
unsigned char * serialize_pack(unsigned char *buffer, package *value)
{
    unsigned char * ptr;
    ptr = serialize_int(buffer, value->size);
    ptr = serialize_int(ptr, value->order);
    ptr = serialize_char(ptr, value->data);
	return ptr;
}
           
int deserialize_int(unsigned char *buffer)
{
        int value = 0;
        
        value |= buffer[0] << 24;
        value |= buffer[1] << 16;
        value |= buffer[2] << 8;
        value |= buffer[3];
        return value;
}
package* deserialize_pack(unsigned char *buffer)
{
	package* pack=(package*)malloc(sizeof(package));
    pack->size=deserialize_int(buffer);
    pack->order=deserialize_int(buffer+4);
    strcpy(pack->data, buffer+8);
    return pack;
}
           
           
void parse_file_into_chunk(FILE *fp, pack_list* pack)
{
    while(!feof(fp))
    {
        
        char buff[1024];
        memset(buff,0, 1024);
        int nread=fread(buff,1,1024,fp);
        printf("Bytes read %d\n",nread);
        if(nread>0)
        {
            //printf("%s\n",buff);
            add_pack_list(pack, buff);
        }
    }
    for(int i=0; i<pack->size; i++)
    {
        pack->arr[i].size=pack->size;
    }
               
}
int parse_nack_msg(char* nmsg, int* nackarr)
{
    const char nack[]="nack";
    char* token=strtok(nmsg,",");
    int i=0;
    while(token)
    {
        if(strcmp(token,nack)!=0)
        {
            nackarr[i++]=atoi(token);
        }
        token=strtok(NULL,",");
    }
    return i;
}
           
void usage_re(char* argv)
{
    printf("The default loss percent is 0 and port is 6000\n");
    printf("but you can specify another port and loss percent.\n");
    printf("usage: %s [-p 1~65535] [-s 0~100]\n", argv);
    printf("		-p <port>\n");
    printf("		-s <loss percent>\n");
}
          
void add_pack_list_re(pack_list* arr, package* pack)
{
    if(arr->size==0)
    {
        arr->capacity=MAX_CAPACITY;
        arr->arr=(package*)malloc(sizeof(package)*MAX_CAPACITY);
    }
    if(arr->size>=MAX_CAPACITY)
    {
        printf("The file is too big, over the default memory capacity\n");
        exit(1);
    }
    strcpy(arr->arr[arr->size].data,pack->data);
    arr->arr[arr->size].order=pack->order;
    arr->arr[arr->size].size=pack->size;
    arr->size++;

}
int parse_cmd_args(int argc, char *argv[])
{
    int loss_percent=0;
    int c;
    while((c=getopt(argc,argv,"p:s:h"))!=-1)
    {
        switch (c)
        {
            case 'p':
                recieve_port=atoi(optarg);
                if((recieve_port<1) || (recieve_port>65535))
            	{
                    printf("Error: invalid port range, you should enter port between 1 and 65535\n");
                    return -1;
                }
                break;
            case 's':
            	loss_percent=atoi(optarg);
                break;
            case 'h':
                usage_re(argv[0]);
                return -1;
            default:
                break;
        }
    }
    return loss_percent;
}
int cmpfunc (const void * a, const void * b)
{
    package* s1 = (package*) a;
    package* s2 = (package*) b;
    return ( s1->order - s2->order);
}
int check_package(pack_list* pack, char nack[])
{
    if(pack->size!=0){
    qsort(pack->arr, pack->size, sizeof(package),cmpfunc);
    int total=pack->arr[0].size;
    int diff=total-pack->size;
    if(diff!=0){
    	int loss[diff];
    	int j=0,k=0;
    	for(int i=1; i<=total; i++){
        	if(j!=pack->size){
        		if (i!=pack->arr[j].order)
        		{
            		loss[k++]=i;
        		}
        		else
        		{
            		j++;
        		}
        	}
        	else
        	{
            	loss[k++]=i;
        	}
    	}
    	sprintf(nack, "nack");
    	for(int i=0;i<k;i++){
        	sprintf(nack, "%s,%d",nack,loss[i]);
    	}
        return 1;
    }
        return 0;
    }
    return 2;
    
}
int check_duplicate_data(pack_list* list, package* pack)
{
    for(int i=0; i<list->size; i++){
        if(pack->order==list->arr[i].order){
            return 1;
        }
    }
    return 0;
}
void parse_chunk_back_to_file(pack_list* pack, FILE *fp)
{
    fp = fopen( "output.txt" , "w+" );
    for(int i=0; i<pack->size; i++)
    {
        //printf("%s\n",pack->arr[i].data);
    	fwrite(pack->arr[i].data , 1 , 1024 , fp );
    }
    fclose(fp);
               
}
int simulate_unreliable(int percent)
{
    float num=rand() / (RAND_MAX + 1.0);
    if(num<percent/100.0){
        return 0;
    }
    else return 1;
    
}
               
               
               
               
               
               
