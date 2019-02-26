#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h>
#include <sys/socket.h> 
#include <errno.h>

#define MAX 80 
#define PORT 8080

int max(int a, int b)
{
	if (a > b)	return a;
	else		return b;
}

char in_buffer[MAX]; 
char out_buffer[MAX];

/*
 * Function that gets the list of files on the server.
 */
void get_file_list(int socket_fd)
{
	while (1) 
	{
		bzero(in_buffer, sizeof(in_buffer)); 
		read(socket_fd, in_buffer, sizeof(in_buffer)); 

		if (in_buffer[0] == '\n')
			break;
		else
			printf("%s\n", in_buffer);
	}
}

/*
 * Function that gets the contents of a file from the server and
 * writes it to a files on the client machine.
 */
void get_file(char *filename, int n_bytes, int socket_fd)
{
	FILE *f = fopen(filename, "w");
	if (f == NULL)
		return;

	int bytes_read;
	bzero(in_buffer, MAX);
	while ((bytes_read = recv(socket_fd, in_buffer, MAX, 0)) > 0 && (n_bytes > 0))
	{
		if (bytes_read < 0)
		{
			printf("ERROR\n");
			break;
		}
		n_bytes -= bytes_read;
		fwrite(in_buffer, sizeof(char), bytes_read, f);
		if (n_bytes <= 0)
			break;
		bzero(in_buffer, MAX);
	}

	fclose(f);
}

/*
 * Function that runs the client process.
 */
void client(int socket_fd) 
{
	int n, bytes_read; 
	for (;;) { 
		bzero(in_buffer, sizeof(in_buffer)); 
		printf(">>> "); 
		n = 0;
		while ((out_buffer[n++] = getchar()) != '\n') 
			; 
		write(socket_fd, out_buffer, sizeof(out_buffer));
		if (strncmp(out_buffer, "listall", 7) == 0)
		{
			get_file_list(socket_fd);
		}
		else if (strncmp(out_buffer, "send", 4) == 0)
		{
			char filename[MAX];
			strcpy(filename, out_buffer + 5);
			int i;
			for (i = 0, n = strlen(filename); i < n; i++)
				if (filename[i] == '\n')
				{
					filename[i] = '\0';
					break;
				}
			if ((bytes_read = recv(socket_fd, in_buffer, MAX, 0)) < 0)
			{
				printf("ERROR.\n");
				continue;
			}
			int n_bytes = atoi(in_buffer);
			if (n_bytes == -1)
				printf("File does not exist.\n");
			else if (n_bytes == -2)
				printf("Error during transfer.\n");
			else
				get_file(filename, n_bytes, socket_fd);
		}
		else if ((strncmp(out_buffer, "exit", 4)) == 0)
		{ 
			printf("Client Exit...\n"); 
			break; 
		}
		else
		{
			printf("Invalid command. Please try again.\n");
		} 
	} 
} 

int main() 
{ 
	int socket_fd, connfd; 
	struct sockaddr_in client_address, cli; 

	socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
	if (socket_fd == -1) { 
		printf("Socket creation failed.\n"); 
		exit(0); 
	} 
	else
		printf("Socket created.\n"); 
	bzero(&client_address, sizeof(client_address)); 

	client_address.sin_family = AF_INET; 
	client_address.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	client_address.sin_port = htons(PORT); 

	if (connect(socket_fd, (struct sockaddr *)&client_address, sizeof(client_address)) != 0) { 
		printf("Failed to connect to server\n"); 
		exit(0); 
	} 
	else
		printf("Connected to the server.\n"); 

	client(socket_fd); 

	close(socket_fd); 
} 
