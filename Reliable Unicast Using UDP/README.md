Assignment2: Reliable Unicast Using UDP
Author: Hui-Jou Chou
Last modified date: 3/20/2017

This is a UDP Reliable Unicast program implemented in C.
Please use Makefile to compile the two programs.

there is a sender program and a receiver program
to enter sender program, just type as follows:

./sender

there is a default port and a default input file, but you can specifiy different port and input file. Type:

./sender -h

to see how to specify parameter.

To enter receiver program, just type as follows:

./receiver

You can type -h to see how to specify the parameter as well. The default parameter for port is 6000,
the default loss percent is 0

if you would like to change the port number, sender and receiver must be specified with same port number.


Data synchronization design
step 1. In the sender side, break file into small chunks and store them in a structure, the structure looks as follows:

typedef struct
{
	int size;
	int order;
	char data[1024];
} package;

Variable SIZE stores the number of total packages, in case of package loss, so every package have to have this information in order to make sure data integrity.
variable ORDER stores the order of this chunk package, in case of arriving receiver in different order. we can sort the data based on sequencing number. 
Variable DATA stores chunked data.

step 2. serialize packages into bytes

step 3. send packages to receiver

step 4. receiver get the packages 

step 5. deserialize packages into package structure.

step 6.receiver sorts the data packages based on the order in case of having different order of packages when arriving

step 7. receiver check data integrity based on checking if total packages they get equal to SIZE which is stored in the every package.

step 8. if reciever does not get total packages, it would request sender to send the packages that it doesn't get yet.

step 9. Once the reciever make sure the data is intrgral, write data into file and output to output.txt, the ourput file would be in the same directory as program.










