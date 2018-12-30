#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef int bool;
#define true 1
#define false 0

static struct termios old, new;

void initTermios(int echo) {
  tcgetattr(0, &old); /* grab old terminal i/o settings */
  new = old; /* make new settings same as old settings */
  new.c_lflag &= ~ICANON; /* disable buffered i/o */
  if (echo) {
      new.c_lflag |= ECHO; /* set echo mode */
  } else {
      new.c_lflag &= ~ECHO; /* set no echo mode */
  }
  tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) {
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) {
  char c;
  initTermios(echo);
  c = getchar();
  resetTermios();
  return c;
}

/* Read 1 character without echo */
char getch(void) {
  return getch_(0);
}

void error(const char* msg){
    perror(msg);
    exit(0);
}

int main(int argc, char* argv[])
{
    FILE *f;
    f = fopen("clientError.log", "a+");
    bool stop = false;
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;
    char message[1];
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        fprintf(f, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        fprintf(f, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
        fprintf(f, "ERROR connecting\n");
    }
    while (stop == false) {
        printf("Please enter the message: ");
        message[0]=0;
        message[0]=getch();
        send(sockfd, message, strlen(message),0);
        printf("%s\n", message);
        if (message[0] == 'q') {
            stop = true;
        }
    }
    close(sockfd);
    return 0;
}
