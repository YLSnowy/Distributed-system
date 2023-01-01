#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

using namespace std;
#define PORT 1234

int local_clock;
int server_clock;

int calculate(); // function to calculate differences

int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    int serverfd, serverlen;
    struct sockaddr_in servaddr;
    // Socket creation for threads
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
    serverlen = sizeof(servaddr);

    // Connect to server
    connect(serverfd, (struct sockaddr *)&servaddr, serverlen);
    // initializing buffers and variables
    char sendbuffer[50] = {0};
    char recvbuffer[50] = {0};
    char servbuffer[50] = {0};
    int diff = 0;
    int avg = 0;
    cout << "connect success " << serverfd << endl;
    // read time daemon time from server

    struct timespec curr_time;
    for (int i = 0; i < 10; i++)
    {
        int ret = read(serverfd, recvbuffer, 50);
        if (ret < 0)
        {
            cout << "error in server value";
        }
        else
        {
            clock_gettime(CLOCK_REALTIME, &curr_time);
            cout << "client current start_time is " << curr_time.tv_sec << endl;
            memset(sendbuffer, '\0', sizeof(sendbuffer));
            string client = to_string(curr_time.tv_sec) + "." + to_string(curr_time.tv_nsec);
            strcpy(sendbuffer, client.c_str());
            write(serverfd, sendbuffer, 50);
        }
    }

    close(serverfd);
    return 0;
}
