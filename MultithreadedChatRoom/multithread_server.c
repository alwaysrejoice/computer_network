/* computer network assignment1, author:Hui-Jou Chou
	started date:2/16/2017 */

/*todo:
 done 1. @who send to private
 2. change color for each user
 3. instruction
 4. output chat history
 done 5.client waiting for server
 done 6.check if name is same at the beginning
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define MAX_CLIENTS 20
#define MAX_LINE 2000
#define FALSE 0
#define TRUE 1

/* See http://pueblo.sourceforge.net/doc/manual/ansi_color_codes.html */

const char *const color_normal  = "\033[0m";
const char *const color_green   = "\033[0;1;32m"; // green + bold
const char *const color_yellow  = "\033[0;1;33m"; // yellow + bold
const char *const color_red     = "\033[0;1;31m"; // red + bold
const char *const color_white   = "\033[0;1;30m"; // white + bold
const char *const color_magenta = "\033[0;1;35m"; // magenta + bold
const char *const color_cyan    = "\033[0;1;36m"; // cyan + bold
const char *const color_blue    = "\033[0;1;34m"; // blue + bold


char user_color[][20]={"\033[0;30m","\033[0;31m","\033[0;32m","\033[0;33m","\033[0;34m","\033[0;35m","\033[0;36m","\033[0;30m","\033[0;31m","\033[0;32m","\033[0;33m","\033[0;34m","\033[0;35m","\033[0;36m"};

typedef struct
{
    char *ip;
    int port;

} cmd_parameters;

typedef struct
{
    int sockid;
    int private;
    char username[20];
    char color[20];
    struct sockaddr_in address;
    
} client_info;

typedef struct list_client
{
    client_info *info;
    pthread_mutex_t *mutex;
    struct list_client *next;
} list_client;

int server_sockfd;
struct sockaddr_in server_address;
cmd_parameters *params;
int curr_thread_count=0;
socklen_t server_len, client_len;
pthread_mutex_t curr_thread_count_mutex=PTHREAD_MUTEX_INITIALIZER;
struct list_client ls_client;
char history_msg[2000][1000];
int history_line_num=0;

void print_help(){
    printf("default ip address is 127.0.0.1, port is 6000\n");
    printf("you can change the ip address and port number when you start the server\n");
    printf("Format is as follows:\n");
    printf("./filename -i <ip address> -p <port number>\n");
}
void list_init(list_client *ls)
{
    //ls=(list_client*)malloc(sizeof(list_client));
    ls->info = NULL;
    ls->next = NULL;
    ls->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(ls->mutex, NULL);
}
void list_add(list_client *ls, client_info *info){
    list_client *cur, *prev;
    cur=prev=ls;
    int inserted = FALSE;
    while (cur != NULL)
    {
        /* Lock entry */
        pthread_mutex_lock(cur->mutex);
        
        
        if (cur->info == NULL)
        {
            cur->info = info;
            pthread_mutex_unlock(cur->mutex);
            inserted = TRUE;
            break;
        }
        
        /* Unlock entry */
        pthread_mutex_unlock(cur->mutex);
        
        /* Load next entry */
        prev = cur;
        cur = cur->next;
    }
    
    /* During iteration through list, no existing element could be reused.
     * We therefore need to append a new list_entry to the list.
     */
    if (inserted == FALSE)
    {
        /* Lock last entry again */
        pthread_mutex_lock(prev->mutex);
        
        /* Create new list entry */
        list_client *new_entry = (list_client *)malloc(sizeof(list_client));
        new_entry->info = info;
        new_entry->next = NULL;
        new_entry->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(new_entry->mutex, NULL);
        
        /* Append entry */
        prev->next = new_entry;
        
        /* Unlock list entry */
        pthread_mutex_unlock(prev->mutex);
        
        inserted = TRUE;
    }
    
    
};
int list_remove(list_client *ls, int sockfd)
{
    list_client *cur;
    
    cur = ls;
    
    while (cur != NULL)
    {
        /* Lock entry */
        pthread_mutex_lock(cur->mutex);
        if(cur->info!=NULL){
        /* Delete client_info data if sockfd matches */
        if (cur->info->sockid == sockfd)
        {
            cur->info = NULL;
            curr_thread_count--;
            pthread_mutex_unlock(cur->mutex);
            break;
        }
        }
        
        /* Unlock entry */
        pthread_mutex_unlock(cur->mutex);
        
        /* Load next entry */
        cur = cur->next;
    }
    
    return 0;
}

list_client* list_find_by_sockid(list_client *list_start, int sockid)
{
    list_client *cur;
    
    cur = list_start;
    
    while (cur != NULL)
    {
        /* Lock entry */
        pthread_mutex_lock(cur->mutex);
        if(cur->info!=NULL){
        if (cur->info->sockid == sockid)
        {
            pthread_mutex_unlock(cur->mutex);
            return cur;
        }
        }
        
        /* Unlock entry */
        pthread_mutex_unlock(cur->mutex);
        
        /* Load next entry */
        cur = cur->next;
    }
    
    return NULL;
}
list_client* list_find_by_uname(list_client *list_start, char *username)
{
    list_client *cur;
    
    cur = list_start;
    
    while (cur != NULL)
    {
        /* Lock entry */
        pthread_mutex_lock(cur->mutex);
        if(cur->info!=NULL){
        /* search client_info data if sockfd matches */
        if (strcmp(cur->info->username, username) == 0)
        {
            pthread_mutex_unlock(cur->mutex);
            return cur;
        }
        }
        /* Unlock entry */
        pthread_mutex_unlock(cur->mutex);
        
        /* Load next entry */
        cur = cur->next;
    }
    
    return NULL;
}


int parse_cmd_args(int argc, char *argv[]){
    params->ip="127.0.0.1";
    params->port=6000;
    int c=0;
    
    
    while((c=getopt(argc,argv,"i:p:h"))!=-1){
        switch (c) {
            case 'i':
                params->ip=optarg;
                break;
            case 'p':
                params->port=atoi(optarg);
                if((params->port<1) || (params->port>65535)){
                    printf("Error: invalid port range, you should enter port between 1 and 65535\n");
                }
                    return -1;
                break;
            case 'h':
                print_help();
                return -1;
            default:
                break;
        }
        
        
    }
    
    return 0;
    
    
}

int start_server(){
    /* Initialize client_info list */
    list_init(&ls_client);

    //create socket
    server_sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(server_sockfd<0){
        printf("ERROR opening socket\n");
        return -1;
    }
    
    //Name the socket
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(params->ip);
    server_address.sin_port = htons(params->port);
    server_len = sizeof(server_address);
    if (bind(server_sockfd, (struct sockaddr *)&server_address, server_len) != 0)
    {
        printf("Error calling bind(): %s\n", strerror(errno));
        return -2;
    }
    
    /* Create a connection queue and wait for incoming connections */
    if (listen(server_sockfd, 5) != 0)
    {
        printf("Error calling listen(): %s\n", strerror(errno));
        return -3;
    }
    
    return 0;
    


}


void stop_server(int sig){
   list_client *cur = NULL;
    time_t curtime=time(NULL);
    struct tm *loctime;
    loctime=localtime(&curtime);
    if ((sig == SIGINT) || (sig == SIGTERM))
    {
        if (sig == SIGINT)
            printf( "Server shutdown requested per SIGINT. Performing cleanup ops now.\n");
        if (sig == SIGTERM)
            printf("Server shutdown requested per SIGTERM. Performing cleanup ops now.\n");
        
        //Close all socket connections immediately
       printf("Closing socket connections...\n");
        
        //output log file
        FILE *fp;
        char fname[100];
        memset(fname,0,100);
        strftime(fname,100,"%y%m%d%I%M_log.txt",loctime);
        fp=fopen(fname,"w");
        if(fp==NULL){
            printf("cannot open output file\n");
        }
        fprintf(fp,"This file only recorded public messages.\n");
        for(int i=0; i<history_line_num; i++){
            fprintf(fp,"%s",history_msg[i]);
        }
        
        // Iterate through client list and shutdown sockets
       cur = &ls_client;
      while (cur != NULL)
        {
            // Lock entry
            pthread_mutex_lock(cur->mutex);
            
            //Send message to client
            if (cur->info != NULL)
           {
                close(cur->info->sockid);
            }
            
            //Unlock entry
           pthread_mutex_unlock(cur->mutex);
            
            // Load next index
           cur = cur->next;
        }
        
        // Close listener connection
        printf("Shutting down listener...\n");
        close(server_sockfd);
        
        //Exit process
        printf("Exiting. Byebye.\n");
        exit(0);
    }


}

void chomp(char *s)
{
    while(*s && *s != '\n' && *s != '\r') s++;
    
    *s = 0;
}
void send_history_msg(int sockid){
    char buffer[1024];
    
    memset(buffer,0, 1024);
    sprintf(buffer,"%s----------------------------------------------%s\n",color_blue,color_normal);
    write(sockid,buffer,strlen(buffer));
    memset(buffer,0, 1024);
    sprintf(buffer,"%s            welcome to chat room              %s\n",color_blue,color_normal);
    write(sockid,buffer,strlen(buffer));
    memset(buffer,0, 1024);
    sprintf(buffer,"%s-----------------Usage:-----------------------%s\n",color_blue,color_normal);
    write(sockid,buffer,strlen(buffer));
    memset(buffer,0, 1024);
    sprintf(buffer,"%s@who: check current users status              %s\n",color_blue,color_normal);
    write(sockid,buffer,strlen(buffer));
    memset(buffer,0, 1024);
    sprintf(buffer,"%s@name <name>: change username                 %s\n",color_blue,color_normal);
    write(sockid,buffer,strlen(buffer));
    memset(buffer,0, 1024);
    sprintf(buffer,"%s@private <name>: send private msg to the user %s\n",color_blue,color_normal);
    write(sockid,buffer,strlen(buffer));
    memset(buffer,0, 1024);
    sprintf(buffer,"%s@end <name>: end private session with the user%s\n",color_blue,color_normal);
    write(sockid,buffer,strlen(buffer));
    memset(buffer,0, 1024);
    sprintf(buffer,"%s@exit: leave the chat room                    %s\n",color_blue,color_normal);
    write(sockid,buffer,strlen(buffer));
    memset(buffer,0, 1024);
    sprintf(buffer,"%s----------------------------------------------%s\n",color_blue,color_normal);
    write(sockid,buffer,strlen(buffer));
    memset(buffer,0, 1024);
    sprintf(buffer,"%s----------------history messages--------------\n",color_white);
    write(sockid,buffer,strlen(buffer));
    
    for(int i=0; i<history_line_num; i++)
    {
        write(sockid,history_msg[i],strlen(history_msg[i]));
    }
    
    memset(buffer,0, 1024);
    sprintf(buffer,"-----------------messages end-----------------%s\n",color_normal);
    write(sockid,buffer,strlen(buffer));
}

void send_broadcast_msg(char* msg, int sockid){
    struct list_client *cur = NULL;
    //struct list_client *list_entry;
    //list_entry = list_find_by_sockid(&ls_client, sockid);
    //char buffer[1024];
    
    chomp(msg);
    cur=&ls_client;
    while(cur!=NULL){
        /* Lock entry */
        pthread_mutex_lock(cur->mutex);
        
        /* Send message to client */
        if (cur->info != NULL)
        {
            
            write(cur->info->sockid,msg,strlen(msg));
        }
        /* Unlock entry */
        pthread_mutex_unlock(cur->mutex);
        
        /* Load next index */
        cur = cur->next;
    }
    
    
    
}

void send_private_msg(char* user, char* msg){
    chomp(msg);
    //char buffer[1024];
    //memset(buffer,0, 1024);
    //sprintf(buffer,"%s\n",msg);
    list_client *cur=list_find_by_uname(&ls_client,user);
    pthread_mutex_lock(cur->mutex);
    write(cur->info->sockid,msg,strlen(msg));
    pthread_mutex_unlock(cur->mutex);
}

void list_show(list_client *ls){
    list_client *cur;
    cur=ls;
    printf("people in public chat\n");
    while(cur!=NULL){
        if(cur->info!=NULL && cur->info->private==0){
            
            printf("sockfd=%d, username=%s\n",cur->info->sockid,cur->info->username);
        }
        cur=cur->next;
    }
    cur=ls;
    
    while(cur!=NULL){
        if(cur->info!=NULL && cur->info->private==1){
            printf("people with private target: ");
            printf("sockfd=%d, username=%s\n",cur->info->sockid,cur->info->username);
        }
        cur=cur->next;
    }
}

void list_show_touser(list_client *ls,int sockid){
    char buffer[1024];
    memset(buffer,0, 1024);
    sprintf(buffer,"%speople in public chat%s\n",color_red,color_normal);
    write(sockid,buffer,strlen(buffer));
    list_client *cur;
    
    cur=ls;
    while(cur!=NULL){
        if(cur->info!=NULL && cur->info->private==0){
            memset(buffer,0, 1024);
            sprintf(buffer,"%ssockfd=%d, username=%s%s\n",color_red,cur->info->sockid,cur->info->username,color_normal);
            write(sockid,buffer,strlen(buffer));
            
        }
        cur=cur->next;
    }
    cur=ls;
    
    while(cur!=NULL){
        if(cur->info!=NULL && cur->info->private==1){
            memset(buffer,0, 1024);
            sprintf(buffer,"%speople with private target: %s",color_yellow,color_normal);
            write(sockid,buffer,strlen(buffer));
            memset(buffer,0, 1024);
            sprintf(buffer,"%ssockfd=%d, username=%s%s\n",color_red,cur->info->sockid,cur->info->username,color_normal);
            write(sockid,buffer,strlen(buffer));
            
        }
        cur=cur->next;
    }
}

void change_name(int sockid, char *uname){
    struct list_client *list_entry;
    list_entry = list_find_by_sockid(&ls_client, sockid);
    strcpy(list_entry->info->username,uname);
};

void client_msg(int *arg){
    int sockid=*arg;
    int private_mode=0;
    const char who[]="@who";
    const char name[]="@name";
    const char private[]="@private";
    const char end_priv[]="@end";
    const char exit_chat[]="@exit";
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockid,&readfds);
    int private_name_num=0;
    char private_name[10][50];

    struct list_client *list_entry;
    list_entry = list_find_by_sockid(&ls_client, sockid);
    
    /* Send welcome and history message */
    send_history_msg(sockid);
    
    char buffer[1024];
    char message[1024];
    char history[1024];
    memset(buffer,0, 1024);
    sprintf(buffer,"%sUser %s joined this chat.%s",color_cyan,list_entry->info->username, color_normal);
    send_broadcast_msg(buffer, sockid);
    char m[1024];
    memset(m,0, 1024);
    sprintf(m,"User %s joined this chat.\n",list_entry->info->username);
    strcpy(history_msg[history_line_num++], m);
    
    while(1){
        int ret=select(FD_SETSIZE, &readfds, (fd_set *)0,(fd_set *)0, (struct timeval *) 0);
        if (ret == -1)
        {
            printf( "Error calling select() on thread:%s\n",strerror(errno));
            
        }
        else{
			//pthread_mutex_lock(list_entry ->mutex);
        	memset(buffer,0, 1024);
            
            //printf("waiting...\n");
            ret=read(sockid, buffer,1024);
            chomp(buffer);
            if(ret>0){
                //user request to see chat room member list
                if(strncmp(buffer,who,4)==0)
                {
                    list_show_touser(&ls_client,sockid);
                }
                //change username
                else if(strncmp(buffer,name,5)==0)
                {
                    char dump[10],uname[100];
                    sscanf(buffer,"%s %s",dump,uname);
                    struct list_client *list_name=list_find_by_uname(&ls_client,uname);
                    if(list_name==NULL){
                        memset(message,0, 1024);
                        sprintf(message,"%s%s change name to %s%s\n",color_magenta, list_entry->info->username, uname, color_normal);
                        printf("%s",message);
                        strcpy(history_msg[history_line_num++],message);
                        change_name(sockid,uname);
                        send_broadcast_msg(message,sockid);
                    }else{
                        memset(message,0, 1024);
                        sprintf(message,"%sserver: cannot change the username. Username already in use.%s\n",color_magenta, color_normal);
                        send_private_msg(list_entry->info->username,message);
                        printf("Cannot change %s username. username already in use\n",list_entry->info->username);
                    }
                }
                //request to send private message
                else if(strncmp(buffer,private,8)==0)
                {
                    char dump[10],uname[100];
                    sscanf(buffer,"%s %s",dump,uname);
                    struct list_client *list_name=list_find_by_uname(&ls_client,uname);
                    if(list_name!=NULL){
                        if(list_name->info->private==1){
                            memset(message,0, 1024);
                            sprintf(message,"%sserver:%s already in private mode. request deny.%s\n",color_magenta, uname, color_normal);
                            printf("%s",message);
                            send_private_msg(list_entry->info->username,message);
                        }
                        else
                        {
                        list_name->info->private=1;
                        private_mode=1;
                        strcpy(private_name[private_name_num++],uname);
                        memset(message,0, 1024);
                        sprintf(message,"%sserver:you are in private mode to %s\t%s\n",color_magenta, uname, color_normal);
                        printf("%s",message);
                            send_private_msg(list_entry->info->username,message);
                        }
                    }else{
                        memset(message,0, 1024);
                        sprintf(message,"%sserver:no user called %s%s\n",color_magenta, uname, color_normal);
                        printf("%s",message);
                        send_private_msg(list_entry->info->username,message);
                    }
                    
                    
                }
                //end private mode to someone
                else if(strncmp(buffer,end_priv,4)==0){
                    int tmp_num=0;
                    char tmp_name[10][50];
                    char dump[10],uname[100];
                    sscanf(buffer,"%s %s",dump,uname);
                    for(int i=0; i<private_name_num;i++){
                        if(strcmp(private_name[i],uname)!=0){
                            strcpy(tmp_name[tmp_num++],private_name[i]);
                        }else{
                            struct list_client *list_name=list_find_by_uname(&ls_client,uname);
                            memset(message,0, 1024);
                            sprintf(message,"%sserver:you left private mode with %s%s\n",color_magenta, uname, color_normal);
                            list_name->info->private=0;
                            send_private_msg(list_entry->info->username,message);
                        }
                    }
                    for(int i=0; i<tmp_num;i++){
                        strcpy(private_name[i],tmp_name[i]);
                    }
                    private_name_num=tmp_num;
                    if(private_name_num==0){
                        private_mode=0;
                    	memset(message,0, 1024);
                    	sprintf(message,"%sserver:your private mode is over%s\n",color_magenta, color_normal);
                        send_private_msg(list_entry->info->username,message);
                    }
                    
                    
                }
                //leave the chat room
                else if(strncmp(buffer,exit_chat,5)==0)
                {
                    memset(message,0, 1024);
                    sprintf(message,"%s%s leave the chat room.%s\n",color_green,list_entry->info->username,color_normal);
                    printf("%s",message);
                    strcpy(history_msg[history_line_num++],message);
                    send_broadcast_msg(message, sockid);
                    list_remove(&ls_client, sockid);
                    /* Disconnect client from server */
                    close(sockid);
                    /* Terminate this thread */
                    pthread_exit(0);
                }
                else if(private_mode==1){
                    for(int i=0;i<private_name_num; i++){
                    memset(message,0, 1024);
                    sprintf(message,"%s@%s:%s]\t\n",list_entry->info->username, private_name[i], buffer);
                        printf("(private)%s",message);
                        //pthread_mutex_lock(list_entry->mutex);

                        send_private_msg(list_entry->info->username,message);
                        //sleep(1);
                        //pthread_mutex_unlock(list_entry->mutex);

                    }
                    memset(message,0, 1024);
                    sprintf(message,"(private)%s:%s\n",list_entry->info->username, buffer);
                    printf("%s",message);
                    for(int i=0; i<private_name_num;i++){
                        send_private_msg(private_name[i],message);
                    }
                }
                else
                {	memset(message,0, 1024);
                    memset(history,0,1024);
                	sprintf(message,"%s%s:%s%s\n",list_entry->info->color, list_entry->info->username, buffer,color_normal);
                    sprintf(history,"%s:%s\n",list_entry->info->username, buffer);
                	printf("%s",message);
                	strcpy(history_msg[history_line_num++],history);
                    send_broadcast_msg(message, sockid);
                }
            }else if(ret==0){
                memset(message,0, 1024);
                memset(history,0,1024);
                sprintf(message,"%s%s leave the chat room.%s\n",color_green,list_entry->info->username,color_normal);
                printf("%s",message);
                sprintf(history,"%s leave the chat room.\n",list_entry->info->username);
                strcpy(history_msg[history_line_num++],history);
                send_broadcast_msg(message, sockid);
                list_remove(&ls_client, sockid);
                /* Disconnect client from server */
                close(sockid);
                /* Terminate this thread */
                pthread_exit(0);
            }
           // pthread_mutex_unlock(list_entry->mutex);

        }
    }
    
    
}



int main(int argc, char *argv[]){
    int ret=0;
    struct sockaddr_in client_addr;
    int client_sockfd=0;
    pthread_t threads[MAX_CLIENTS];
    
    params = malloc(sizeof(cmd_parameters));
    ret=parse_cmd_args(argc, argv);
    if(ret==-1)
    {
        exit(0);
    }
    // setup signal handler
    signal(SIGINT, stop_server);
    signal(SIGTERM, stop_server);
    
    //start the server listener
    if(start_server()<0){
        printf("Error during server start.\n");
        exit(-1);
    }
    char message[1024];
    while(1){
        
        printf("Waiting for incoming connection...\n");
        client_len=sizeof(client_addr);
        client_sockfd=accept(server_sockfd, (struct sockaddr *)&client_addr, &client_len);
        if(client_sockfd>0){
            printf("server accepted new connection on socket id %d\n",client_sockfd);
            pthread_mutex_lock(&curr_thread_count_mutex);
            char buffer[1024];
            if(curr_thread_count<MAX_CLIENTS){
                bzero(buffer,1024);
                read(client_sockfd,buffer,1024);
                chomp(buffer);
                //check if name already in use after the first user login
                if(curr_thread_count>0)
                {
               		struct list_client *list_name=list_find_by_uname(&ls_client,buffer);
                	if(list_name!=NULL)
                    {
                    	memset(message,0, 1024);
                    	sprintf(message,"%sserver: Username already in use. You will be called user%d temporarily.\n You can change your name after login.%s\n",color_magenta,client_sockfd, color_normal);
                        write(client_sockfd,message,strlen(message));
                        bzero(buffer,1024);
                        sprintf(buffer,"user%d",client_sockfd);
                	}
                }

                
                
                client_info *tmp_client=(client_info*)malloc(sizeof(client_info));
                tmp_client->sockid=client_sockfd;
                tmp_client->address=client_addr;
                tmp_client->private=0;
                strcpy(tmp_client->username, buffer);
                int rand_color=((rand()%14)+curr_thread_count)%14;
                printf("assign color number:%d\n",rand_color);
                strcpy(tmp_client->color,user_color[rand_color]);
                //tmp_client->username=buffer;
                printf("user %s login (sockid=%d)\n",tmp_client->username,tmp_client->sockid);
                
                list_add(&ls_client,tmp_client);
                list_show(&ls_client);
                //printf("hi\n");
                
                ret=pthread_create(&threads[curr_thread_count],NULL,(void*)&client_msg,(void*)&client_sockfd);
                if(ret==0){
                    pthread_detach(threads[curr_thread_count]);
                    curr_thread_count++;
                    printf("user %s joined this chat.\n",buffer);
                    
                    
                }else{
                    free(tmp_client);
                    close(client_sockfd);
                }
                
                
            }
            else{
                printf("Max connections reached. Connection limit is %d.",MAX_CLIENTS);
                close(client_sockfd);
            }
            pthread_mutex_unlock(&curr_thread_count_mutex);
        }else{
            printf("connection could not be established.\n");
            perror(strerror(errno));
            exit(-3);
        }
        
        
        
        
    }
    free(params);
    return 0;
    

    
}


