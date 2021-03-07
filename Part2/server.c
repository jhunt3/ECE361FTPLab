/*
** listener.c -- a datagram sockets "server" demo
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

#define MYPORT "4950"	// the port users will be connecting to

#define MAXBUFLEN 2000

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	struct packet {
		unsigned int total_frag;
		unsigned int frag_no;
		unsigned int size;
		char *filename;
		char filedata[1000]; 
	};

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("listener: got packet from %s\n",
		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);

	if(strcmp(buf,"ftp")==0){
		if ((numbytes = sendto(sockfd,"yes", strlen("yes"), 0,
			 (struct sockaddr *)&their_addr, sizeof(their_addr))) == -1) {
			
			perror("sendto");
			exit(1);
		}
		printf("Sending response 'yes'\n");
		FILE *fp;
		for(;;){
			memset(buf, 0, MAXBUFLEN);
			if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
				(struct sockaddr *)&their_addr, &addr_len)) == -1) {
				perror("recvfrom");
				exit(1);
			}
			printf("Packet:\n");
			int stage = 0;
			int numPacks;
			int fragNo;
			int fragSize;
			char filename[100];
			char filedata[1000];
			char value[100];
			char* c;
			int cnt = 0;
			for(int i=0; buf[i]!=0;i++){
				//strcpy(c,buf[i]);
				//c = &buf[i];
				if(buf[i]==':'){
					if(stage==0){
						numPacks = atoi(value);
						printf("\nNumber of packs: %d\n",numPacks);	
					}else if(stage==1){
						fragNo = atoi(value);
						printf("\nFragment Number: %d\n",fragNo);
					}else if(stage==2){
						fragSize = atoi(value);
						printf("\nSize: %d\n",fragSize);
					}else if(stage==3){
						strcpy(filename,value);
						char fn[100]="/nfs/ug/homes-1/h/huntjere/ECE361/ECE361FTPLab/Part2/serverstorage/";
						/*int counter;
						while(filename[counter] !='\0'){
							if(filename[counter]=='/'){
								filename[counter]=':';
							}
						}*/

						strcat(fn,filename);
						printf("\nFile Name: %s\n",fn);
						if(fragNo==1){
							fp=fopen(fn,"w");
						}else{
							fp=fopen(fn,"a");
						}
						for(int j=(i+1); j<(i+1+fragSize);j++){
							//filedata[j-i-1]=buf[j];
							fputc(buf[j],fp);
						}
						//fputc(buf[i],fp);
						fclose(fp);
						break;
					}
					memset(value, 0, 100);
					cnt=0;
					stage++;
				}else{
					value[cnt]=buf[i];
					cnt++;
					//strcat(value,c);
				}
				printf("%c",buf[i]);
				//buf++;
			}
			printf("server: packet is %d bytes long\n", numbytes);
			if ((numbytes = sendto(sockfd,"ACK", strlen("ACK"), 0,
				 (struct sockaddr *)&their_addr, sizeof(their_addr))) == -1) {
			
				perror("sendto");
				exit(1);
			}
			if(numPacks==fragNo){break;}

		}

	}else{
		if ((numbytes = sendto(sockfd,"no", strlen("no"), 0,
			 (struct sockaddr *)&their_addr, sizeof(their_addr))) == -1) {
			
			perror("sendto");
			exit(1);
		}
		printf("Sending response 'no'\n");

	}


	close(sockfd);

	return 0;
}
