/*
** talker.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include <libgen.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
//#define SERVERPORT "4950"	// the port users will be connecting to
#define MAXBUFLEN 100
int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	char fname[100];
	char splitStrings[2][50];
	int i, j, cnt;
	char buf[MAXBUFLEN];
	char cwd[PATH_MAX];
	char cmpcwd[PATH_MAX];
	char filepath[100];
	FILE *file;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	struct packet {
		unsigned int total_frag;
		unsigned int frag_no;
		unsigned int size;
		char *filename;
		char filedata[1000]; 
	};
	strcat(cwd,"/");
	if (argc != 3) {
		fprintf(stderr,"usage: talker hostname message\n");
		exit(1);
	}
	printf("ftp command:\n");
	fgets(fname, 100, stdin);
	fname[strcspn(fname,"\n")]=0;
	j=0; cnt=0;
	for(i=0;i<=(strlen(fname));i++)
	{
	// if space or NULL found, assign NULL into splitStrings[cnt]
		if(fname[i]==' '||fname[i]=='\0')
	        {
        	    //if (getcwd(cwd, sizeof(cwd)) != NULL) {
		//	printf("Current working dir: %s\n", cwd);
		  //  } else {
		//	perror("getcwd() error");
	        //	return 1;
        	  //  }

	            splitStrings[cnt][j]='\0';
	            cnt++;  //for next word
	            j=0;    //for next word, init index to 0
		    //cmpcwd=cwd;
		    //strcat(cwd,"/");
		    //strcat(cwd,splitStrings[cnt-1]);
		    
		    strcpy(filepath, splitStrings[cnt-1]);
		    printf("Current working file: '%s'\n", filepath);
		    printf("%d, %s\n",cnt, filepath);
		    printf("%d\n", access(filepath, F_OK));
		    		    if (cnt != 1 && access(filepath, F_OK)==-1){
			if(file=fopen(filepath,"r")){
				printf("%d, %s\n",cnt, filepath);
				printf("File Found");
			}else{
				printf("%d, %s\n",cnt, filepath);
		    		fprintf(stderr,"invalid file\n");
				exit(1);
			}
		    }
	        }
	        else
	        {
	            splitStrings[cnt][j]=fname[i];
	            j++;
	        }
	}
	if(strcmp(splitStrings[0], "ftp")!=0){
		fprintf(stderr,"not ftp command\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	if ((numbytes = sendto(sockfd, splitStrings[0], strlen(fname), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);
	memset(buf, 0, MAXBUFLEN);
	printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
			 (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}
	printf("Received: ");
	printf(buf);
	printf("\n");
	if(strcmp(buf,"yes")==0){
		fprintf(stdout, "A file transfer can start\n");
	}else{
		fprintf(stdout, "Invalid command\n");
	}
	char* ts = strdup(filepath);
	char* filename = basename(ts);
	printf("Sending: %s\n",filename);
	file=fopen(filepath,"r");
	fseek(file, 0L, SEEK_END);
	long int fs = ftell(file);
	printf("Size of transferring file: %ld bytes\n",fs);
	int numPacks = fs/1000 + 1;
	int modSize = fs%1000;
	printf("Number of packets being sent: %d\n",numPacks);
	rewind(file);
	for (int i = 0;i<numPacks;i++){
		struct packet pack;
		if((i+1)<numPacks){
			pack.size = 1000;
		}else{
			pack.size = modSize;
		}

		
		pack.total_frag = numPacks;
		pack.frag_no = i+1;
		pack.filename = filename;
		memset(pack.filedata, 0, sizeof(char) * (1000));
		while(fread((void*)pack.filedata, sizeof(char), pack.size, file)==1){

		}
		//printf("%s\n", pack.filedata);
		//printf("%d\n", sizeof(pack.filedata));
		//printf("%d\n", pack.size);
		//pack.size = sizeof(pack.filedata);
		printf("Sending packet %d\n",(i+1));
				printf("Size: %d\n",pack.size);

		//printf("Data: %s\n",pack.filedata);
		//
		//
		char packStr[2000];
		memset(packStr, 0, sizeof(char) * (2000));
		char metadata[1000] = "";
		char numtostr[100];
		//scanf("%f", &pack.total_frag);
		sprintf(numtostr, "%d",pack.total_frag); 
		strcat(metadata, numtostr);
		strcat(metadata, ":");
		//scanf("%f", &pack.frag_no);
		sprintf(numtostr, "%d",pack.frag_no); 
		strcat(metadata, numtostr);
		
		strcat(metadata, ":");
		sprintf(numtostr, "%d",pack.size); 
		strcat(metadata, numtostr);
		strcat(metadata, ":");
		/*int counter;
		while(pack.filename[counter] !='\0'){
			if(pack.filename[counter]==':'){
				pack.filename[counter]='/';
			}
		}*/
		strcat(metadata,pack.filename);
		strcat(metadata, ":");
		
		memcpy(packStr, metadata, sizeof(char)*strlen(metadata));
		memcpy(&packStr[sizeof(char)*strlen(metadata)], pack.filedata, pack.size);
		printf("%s\n",packStr);
		fcntl(sockfd,F_SETFL, O_NONBLOCK);
		bool transmitted=false;
		for(;;){
			clock_t start, end;
			start = clock();
			printf("Sending packet %d\n",i);
			if ((numbytes = sendto(sockfd, packStr, sizeof(packStr), 0,
				p->ai_addr, p->ai_addrlen)) == -1) {
				perror("talker: sendto");
				exit(1);
			}

			//freeaddrinfo(servinfo);
			memset(buf, 0, MAXBUFLEN);
			//printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
			addr_len = sizeof their_addr;

			for(;;){
				end=clock();
				if((double)(end-start)/CLOCKS_PER_SEC>0.5){
					break;
				}

				if(numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
					(struct sockaddr *)&their_addr, &addr_len)!=-1){
					transmitted=true;
					printf("Received: ");
					printf(buf);
					printf(" in %fs\n",(double)(end-start)/CLOCKS_PER_SEC);
					if(strcmp(buf,"ACK")==0){
						fprintf(stdout, "Packet Acknowledged, continuing...\n");
					}else{
						fprintf(stdout, "Packet delivery failed.\n");
					}


					break;
				}
				/*if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
					(struct sockaddr *)&their_addr, &addr_len)) == -1) {
					perror("recvfrom");
					exit(1);
				}*/
			}
			if(transmitted==true){
				break;
			}
		}

	
	}

	close(sockfd);

	return 0;
}
