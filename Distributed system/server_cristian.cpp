#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <iomanip>

using namespace std;

#define PORT 1234
// defining the number of clients to be 3
#define no_of_clients 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lock = PTHREAD_COND_INITIALIZER;

int thread_counter = 0;
char sendbuffer[50];
char recvbuffer[50] = {'0'};

double *char2double(char ch[])
{
    double s = 0.0, ns = 0.0;
    int flag = 0, non1 = 0, non2;
    for (int i = 0; i < strlen(ch); i++)
    {
        if (ch[i] == '.')
        {
            flag = 1;
            s /= 10;
            continue;
        }
        if (ch[i] == '-' && flag == 0)
        {
            non1 = 1;
            continue;
        }
        else if (ch[i] == '-' && flag == 1)
        {
            non2 = 1;
            continue;
        }
        if (flag == 0)
        {
            s += (ch[i] - '0');
            s *= 10;
        }
        else
        {
            ns += (ch[i] - '0');
            ns *= 10;
        }
    }
    ns /= 10;
    double *res = new double();
    if (non1 == 1)
        res[0] = -1 * s;
    else
        res[0] = s;
    if (non2 == 1)
        res[1] = -1 * ns;
    else
        res[1] = ns;
    // cout << res[0] << " " << res[1] << endl;
    return res;
}

// czlculating and sending the new offset value to the local clocks at client side
void Clock_Synchronization(int newsockfd)
{
    char sendbuffer[50] = {0};
    char recvbuffer[50] = {0};
    char servbuffer[50] = {0};
    cout << "connect success " << newsockfd << endl;

    for (int i = 0; i < 10; i++)
    {
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_REALTIME, &start_time);
        cout << "server current time is " << start_time.tv_sec << "s " << start_time.tv_nsec << "ns" << endl;
        string server = to_string(start_time.tv_sec) + "." + to_string(start_time.tv_nsec);
        strcpy(servbuffer, server.c_str());
        int ret = write(newsockfd, servbuffer, 50);
        if (ret < 0)
            cout << "send fail" << endl;

        ret = read(newsockfd, recvbuffer, 50);
        if (ret < 0)
        {
            cout << "rece from client failed" << endl;
        }
        else
        {

            double *res = char2double(recvbuffer);
            cout << fixed;
            cout << setprecision(0);
            cout << "receive client current time is " << res[0] << "s " << res[1] << "ns" << endl;

            clock_gettime(CLOCK_REALTIME, &end_time);
            cout << "Tround is " << end_time.tv_sec - start_time.tv_sec << "s " << end_time.tv_nsec - start_time.tv_nsec << "ns" << endl;
            cout << "After updated time is " << res[0] + (end_time.tv_sec - start_time.tv_sec) / 2 << "s " << res[1] + (end_time.tv_nsec - start_time.tv_nsec) / 2 << "ns" << endl;
            cout << "current time is " << end_time.tv_sec << "s " << end_time.tv_nsec << "ns" << endl;
            cout << "the accuracy of the algorithm is " << end_time.tv_sec - (res[0] + (end_time.tv_sec - start_time.tv_sec) / 2) << "s " << end_time.tv_nsec - (res[1] + (end_time.tv_nsec - start_time.tv_nsec) / 2) << "ns" << endl;
        }
        clock_gettime(CLOCK_REALTIME, &end_time);
        cout << "The Time to complete the algorithm is " << end_time.tv_sec - start_time.tv_sec << "s " << end_time.tv_nsec - start_time.tv_nsec << endl;
        cout << "---------------------------------------------------------------------" << endl;
        sleep(1);
    }
    return;
}

void *worker_thread(void *args)
{
    int newsockfd = *((int *)args);
    cout << "Socket for thread is: " << newsockfd << endl;

    pthread_mutex_lock(&mutex); // lock

    while (!&lock)
    {
        pthread_cond_wait(&lock, &mutex);
    }
    pthread_cond_signal(&lock);
    pthread_mutex_unlock(&mutex);

    Clock_Synchronization(newsockfd);
    close(newsockfd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

    // Creation of Threads
    pthread_t t[no_of_clients];

    int sockfd, clientfd, addrlen;
    struct sockaddr_in servaddr, cliaddr;

    // Creation of sockets
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cout << "Error creating a socket" << endl;
        return -1;
    }
    // fill servaddr with zeros
    memset(&servaddr, 0, sizeof(struct sockaddr_in));

    // Assigning IP address with port
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // bind IP address with port
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        cerr << "Binding Error: " << endl;
        exit(0);
    }
    else
    {
        cout << "Binding........" << endl;
    }

    // Listening for client connection requests
    if (listen(sockfd, 5) < 0)
    {
        cerr << "Listening Error!! " << endl;
        exit(0);
    }
    else
    {
        cout << "Listening for connections...... " << endl;
    }
    addrlen = sizeof(cliaddr);
    thread_counter = 0;

    /// creation of threads
    while (1)
    {
        clientfd = accept(sockfd, (struct sockaddr *)&cliaddr, (socklen_t *)&addrlen);
        if (clientfd == EAGAIN)
        {
            continue;
        }
        pthread_create(&t[thread_counter], NULL, worker_thread, (void *)&clientfd);
        cout << "thread_counter :" << thread_counter << endl;
        thread_counter++;

        while (thread_counter > no_of_clients)
        {
        }
    }
    for (int i = 0; i < thread_counter; i++)
    {
        pthread_join(t[i], NULL); // waiting for all threads to finish
    }

    return 0;
}