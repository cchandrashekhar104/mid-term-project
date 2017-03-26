/*
** server.c -- a stream socket server demo
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define MAXDATASIZE 100
#define BACKLOG 10     // how many pending connections queue will hold
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#define BUFSIZE 1024
 struct sigaction na;
 struct itimerval timer;
 int sockfd, new_fd;
 int numbytes;
  
  

    pa_simple *saa = NULL;
    int ret = 1;
    int error;

    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char buf[MAXDATASIZE];
   

static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };

//Main loop 

void periodic_task  (int signum)
{
     uint8_t bufi[BUFSIZE];
     ssize_t r;
       //reciving bits//
       if ((r = recv(new_fd, bufi, sizeof(bufi), 0)) == -1) {
               perror("recv");
                 exit(1);
                 }

        //playback audio//
       if (pa_simple_write(saa, bufi, (size_t) r, &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
             if (saa)
               pa_simple_free(saa);
            exit(1);
       }
        
               
 }
         

  
void my_handler_for_sigint(int );
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

int main(int arg,char *argv[])
{
 memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

  

  if (arg != 2) {
       fprintf(stderr,"usage: Port number to bind?\n");
        exit(1);
    }    

if ((rv = getaddrinfo(NULL,argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can//
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

    freeaddrinfo(servinfo); // all done with this structure//

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

//registering the signal handler//
if (signal(SIGINT, my_handler_for_sigint) == SIG_ERR)
      printf("\ncan't catch SIGINT\n");    

 if (!(saa = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        if (saa)
          pa_simple_free(saa);
        exit(1);

    }


while(1) {// main accept() loop
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

    

printf("\nspeak");


 /* Install periodic_task  as the signal handler for SIGVTALRM. */
 memset (&na, 0, sizeof (na));
 na.sa_handler = &periodic_task ;
 sigaction (SIGVTALRM, &na, NULL);

 /* Configure the timer to expire after 5 msec... */
 timer.it_value.tv_sec = 0;
 timer.it_value.tv_usec = 5000;

 /* ... and every 5 msec after that. */
 timer.it_interval.tv_sec = 0;
 timer.it_interval.tv_usec = 5000;

 /* Start a virtual timer. It counts down whenever this process is    executing. */
 setitimer (ITIMER_VIRTUAL, &timer, NULL);

 /* Do busy work. */
 while (1);
}

  // Make sure that every single sample was played //
    if (pa_simple_drain(saa, &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
        if (saa)
               pa_simple_free(saa);
            exit(1);
}


}

// signal handler//
void my_handler_for_sigint(int signumber)
{
  char ans[2];
  if (signumber == SIGINT)
  {
    printf("received SIGINT\n");
    printf("Program received a CTRL-C\n");
    printf("Terminate Y/N : "); 
    scanf("%s", ans);
    if (strcmp(ans,"Y") == 0)
    {
       printf("Exiting ....\n");
       close(sockfd);       
       close(new_fd);       
	exit(0); 
    }
    else
    {
       printf("Continung ..\n");
    }
  }
}


