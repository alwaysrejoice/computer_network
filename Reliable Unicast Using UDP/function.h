//
//  function.h
//  hw2
//
//  Created by hui-jou chou on 3/18/17.
//
//

#ifndef function_h
#define function_h

// Include any system framework and library headers here that should be included in all compilation units.
// You will also need to set the Prefix Header build setting of one or more of your targets to reference this file.

#endif /* function_h */

#define DATALEN 1500
#define MAX_CAPACITY 1000

extern int recieve_port;

typedef struct
{
    int size;
    int order;
    char data[1024];
} package;

typedef struct
{
    int size, capacity;
    package* arr;
} pack_list;

void init_pack(pack_list *pack);
void add_pack_list(pack_list* arr, char* data);
void parse_file_into_chunk(FILE *fp,pack_list* pack);
int parse_nack_msg(char* nmsg, int* nackarr);
void usage_se(char* argv);
void usage_re(char* argv);
void add_pack_list_re(pack_list* arr, package* data);
int parse_cmd_args(int argc, char *argv[]);
void parse_chunk_back_to_file(pack_list* pack, FILE *fp);
int simulate_unreliable(int percent);
int check_package(pack_list* pack,char nack[]);
int check_duplicate_data(pack_list* list, package* pack);

unsigned char * serialize_int(unsigned char *buffer, int value);
unsigned char * serialize_char(unsigned char *buffer, char value[]);
unsigned char * serialize_pack(unsigned char *buffer, package *value);
int deserialize_int(unsigned char *buffer);
package* deserialize_pack(unsigned char *buffer);









