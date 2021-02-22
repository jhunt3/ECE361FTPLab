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
	FILE *file;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
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
		    char filepath[100];
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
	clock_t start, end;
	start = clock();
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
	end=clock();
	printf("RTT=%fs\n",((double)(end-start)/CLOCKS_PER_SEC));
	printf("Received: ");
	printf(buf);
	printf("\n");
	if(strcmp(buf,"yes")==0){
		fprintf(stdout, "A file transfer can start\n");
	}else{
		fprintf(stdout, "Invalid command\n");
	}

	//fseek(fp, 0L, SEEK_END);
	//long int fs = ftell(fp);
	//printf("Size of transferring file: %ld bytes",fs);
	close(sockfd);

	return 0;
}
