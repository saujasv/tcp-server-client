#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#define MAX 80 
#define PORT 8080

char in_buffer[MAX]; 
char out_buffer[MAX];
int client_no;

/*
 * Function opens file with filename and sends it to client.
 */
void send_file(char *filename, int socket_fd)
{
	FILE *f = fopen(filename, "r");
	int bytes_read;
	while ((bytes_read = fread(out_buffer, sizeof(char), MAX, f)) > 0)
	{
		if (send(socket_fd, out_buffer, bytes_read, 0) < 0)
		{
			printf("ERROR\n");
			break;
		}
		bzero(out_buffer, MAX);
	}
}

/*
 * Function lists all files in the current directory.
 */
void listall(char *path, int socket_fd)
{
	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		perror("Directory doesn't exist");
		bzero(out_buffer, MAX);
		sprintf(out_buffer, "Directory doesn't exist");
	}

	struct dirent *dir_details = readdir(dir);
	while (dir_details != NULL)
	{
		bzero(out_buffer, MAX);
		sprintf(out_buffer, "%s", dir_details->d_name);
		write(socket_fd, out_buffer, sizeof(out_buffer));
		dir_details = readdir(dir);
	}

	bzero(out_buffer, MAX);
	sprintf(out_buffer, "\n");
	write(socket_fd, out_buffer, sizeof(out_buffer));

	closedir(dir);
}

/*
 * Function that runs the server
 */
void serve(int socket_fd) 
{
	int n;
	while (1) 
	{ 
		bzero(in_buffer, MAX); 
		read(socket_fd, in_buffer, sizeof(in_buffer));
		printf("From client %d: %s", client_no, in_buffer);
		bzero(out_buffer, MAX); 
		if (strncmp(in_buffer, "listall", 7) == 0)
		{
			listall(".", socket_fd);
		}
		else if (strncmp(in_buffer, "send", 4) == 0)
		{
			char filename[MAX];
			char file_size[MAX];
			bzero(out_buffer, MAX);
			strncpy(filename, in_buffer + 5, strlen(in_buffer) - 6);
			int fd = open(filename, O_RDONLY);
			if (fd < 0)
			{
				perror("open");
				if (errno == ENOENT)
					sprintf(file_size, "%d", -1);	
				else
					sprintf(file_size, "%d", -2);
			}
			else
			{
				struct stat file_stat;
				fstat(fd, &file_stat);
				sprintf(file_size, "%lld", file_stat.st_size);
			}
			close(fd);

			if (send(socket_fd, file_size, sizeof(file_size), 0) < 0)
			{
				printf("ERROR\n");
				break;
			}

			send_file(filename, socket_fd);
			printf("File %s transferred to client %d.\n", filename, client_no);
			close(fd);
		}
		else if (strncmp("exit", in_buffer, 4) == 0) { 
			printf("Server process for client %d exit.\n", client_no); 
			break; 
		}
		else
		{
			sprintf(out_buffer, "Invalid command");
			write(socket_fd, out_buffer, sizeof(out_buffer));
		}
	} 
} 

int main() 
{ 
	int socket_fd, conn_fd, len; 
	struct sockaddr_in server_address, cli; 

	socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
	if (socket_fd < 0) 
	{ 
		printf("Socket creation failed.\n"); 
		exit(1); 
	} 
	else
		printf("Socket created.\n"); 
	bzero(&server_address, sizeof(server_address)); 

	server_address.sin_family = AF_INET; 
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); 
	server_address.sin_port = htons(PORT); 

	if ((bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address))) != 0) { 
		printf("Socket bind failed.\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully bound.\n"); 

	// Listen for clients on the socket
	if ((listen(socket_fd, 5)) != 0) { 
		printf("Listen failed.\n"); 
		exit(0); 
	} 
	else
		printf("Server listening.\n"); 
	
	while (1) {
		len = sizeof(cli); 

		// Accept connection from client on a different file descriptor
		conn_fd = accept(socket_fd, (struct sockaddr *) &cli, (unsigned int *) &len); 
		if (conn_fd < 0) 
		{ 
			printf("Server accept failed.\n"); 
			exit(0); 
		}
		else
		{
			// Client handled by child process, server continues to listen
			pid_t childpid = fork();
			client_no++;
			if (childpid == 0)
			{
				printf("Server accepts client %d.\n", client_no);
				close(socket_fd);
				serve(conn_fd);
				close(conn_fd);
				exit(0);
			}
		}
	}

	close(socket_fd); 
} 
