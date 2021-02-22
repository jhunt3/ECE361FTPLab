#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <regex.h>

#define BUF_SIZE 1100

typedef struct tagPacket {
	unsigned int total_frag;
	unsigned int frag_no;
	unsigned int size;
	char *filename;
	char filedata[1000]; 
} Packet;
