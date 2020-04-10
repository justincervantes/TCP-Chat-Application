/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		client.cpp - A simple TCP chat client program.
--
--	PROGRAM:			client
--
--	FUNCTIONS:			Berkeley Socket API
--						readThread
--
--	DATE:				February 2, 2008
--
--	REVISIONS:			(Date and Description)
--						January 2005
--						Modified the read loop to use fgets.
--						While loop is based on the buffer length 
--						April 8, 2020
--						Modified the code to work for a chat room application
--						Added a read thread 
--						Added the name request and buffer formatting to include it in each message
--						
--
--	DESIGNERS:			Aman Abdulla
--						Justin Cervantes
--
--	PROGRAMMERS:		Aman Abdulla
--						Justin Cervantes
--
--	NOTES:
--	The program will establish a TCP connection to a user specifed server.
--  The server can be specified using a fully qualified domain name or and
--	IP address. After the connection has been established the user will be
--  prompted for their chat room name, followed by a series of messages. 
--	The user's name and message are sent to the server; the server only 
--	messages the client when another client has submitted something to the
--  chat.
--
--  COMPILE: g++ -Wall -o client client.cpp -lpthread
--  RUN: ./client <host address>
---------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#define SERVER_TCP_PORT		7000	// Default port
#define BUFLEN				255  	// Buffer length

void readThread(int sd);

int main (int argc, char **argv)
{
	int sd, port;
	struct hostent	*hp;
	struct sockaddr_in server;
	char  *host, sbuf[BUFLEN], **pptr;
	char str[16];

	switch(argc)
	{
		case 2:
			host =	argv[1];	// Host name
			port =	SERVER_TCP_PORT;
		break;
		case 3:
			host =	argv[1];
			port =	atoi(argv[2]);	// User specified port
		break;
		default:
			fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
			exit(1);
	}

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
		exit(1);
	}
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	// Connecting to the server
	if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}
	printf("Connected:    Server Name: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));
	

	// Get the user's desired chatroom alias
	printf("Enter your chatroom name: \n");
	fgets (sbuf, BUFLEN, stdin);
	int partition;
	for(int i = 0; i < BUFLEN; i++) {
		if(sbuf[i]=='\n') {
			sbuf[i] = ':';
			sbuf[i+1] = ' ';
			partition = i+2;
			break;
		}
	}

	
	// Starts polling for messages from the server
	std::thread reader(readThread, sd);


	// Poll for user messages to the server, and sends the name plus the message
	char message[BUFLEN-partition];
	while(true) {
		fgets (message, BUFLEN-partition, stdin);
		for(int i = partition; i<BUFLEN-1; i++) {
			sbuf[i]  = message[i-partition];
		}
		sbuf[BUFLEN-1] = 0;
		send (sd, sbuf, BUFLEN, 0);
	}
	
	// Socket closes on SIGINT, and the server is notified via TCP

	return (0);
}


/*------------------------------------------------------------------------------
-- FUNCTION: 		readThread
--
-- DATE: 			April 8, 2020
--
-- REVISIONS:
-- Version, Date and Description
--
-- DESIGNER: 		Justin Cervantes
--
-- PROGRAMMER: 		Justin Cervantes
--
-- INTERFACE: 		readThread(int sd)
--
-- RETURNS:			void
--
-- NOTES:
-- Polls for new messages from the server.
--------------------------------------------------------------------------*/
void readThread(int sd) {
	char rbuf[BUFLEN];
	char* bp;
	int n;

	while(true) {
		bp = rbuf;
		int bytes_to_read = BUFLEN;

		// client makes repeated calls to recv until no more data is expected to arrive.
		n = 0;
		while ((n = recv (sd, bp, bytes_to_read, 0)) < BUFLEN)
		{
			bp += n;
			bytes_to_read -= n;
		}
		printf ("%s", rbuf);
		fflush(stdout);
	}
	
}