/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#define BUFSIZE 32

#define RATE 44100
#define MAXDATASIZE 32 // max number of bytes we can get at once


#define BACKLOG 10     // how many pending connections queue will hold

int16_t buffer[BUFSIZE];
int error;
static pa_simple *s1 = NULL;
static char name_buf[] = "PulseAudio default device";

int pulseaudio_begin()
{
  int error;
    /* The Sample format to use */
  static const pa_sample_spec ss = {
  	.format = PA_SAMPLE_S16LE,
  	.rate = RATE,
  	.channels = 2
  };

    /* Create a new playback stream */
  if (!(s1 = pa_simple_new(NULL, "abc", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
	fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
	return 1;
  }
  return 0;
}
int pulseaudio_end()
{
  if (s1 != NULL) {
    pa_simple_free(s1);
    s1 = NULL;
  }
  return 0;
}
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc,char *argv[])
{
    int sockfd, new_fd,numbytes;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
	char buf[100];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
	if (argc != 2) {
        fprintf(stderr,"usage: Server portname\n");
        exit(1);
    }
    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    pulseaudio_begin();
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
			while(1)
			{
				if ((numbytes = recv(new_fd, buffer, MAXDATASIZE-1, 0)) == -1) {
		    		perror("recv");
		    		exit(1);
	   			}
				if(numbytes==0)
				{
					printf("connection Closed from client %s\n",s);
					break;				
				}
        /* Play the received data */
				if (pa_simple_write(s1, buffer, numbytes, &error) < 0) 
				{
					fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
					goto finish;
				}

			}
        close(new_fd);
    }
finish:
	pulseaudio_end();
    return 0;
}

