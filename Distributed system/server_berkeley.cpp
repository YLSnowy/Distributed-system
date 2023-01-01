#include <iostream>
#include <sstream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <iomanip>

using namespace std;

#define PORT 1234
// defining the number of clients to be 3
#define no_of_clients 3
// #define _XOPEN_SOURCE

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lock = PTHREAD_COND_INITIALIZER;

int thread_counter = 0;
char sendbuffer[50];
char recvbuffer[50] = {'0'};
int globalfd[100];

int difference_s[10]; // global array to store differences between each individual clocks and time daemon
int difference_ns[10];
int avg_s;
int avg_ns;

int serv_clock; // global variable which stores server time
int count = 0;

bool running = true;
struct timespec curr_time;
struct timespec start_time, end_time;


void calculation();
void send_offset();

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

    globalfd[thread_counter - 1] = newsockfd; // store the client file descriptors in a global array

    calculation();
    while (running)
    {
    }
    close(newsockfd);
    pthread_exit(NULL);
}
void calculation() // funtion to send time daemon clock time and calculating differences between local clock and time daemon
{
    int temp_fd = 0;
    char clock_val[50] = {0};
    int temp_clock[50];

    int new_offset[50]; // array to calculate the new offset
    double sum_of_diff_s = 0.0, sum_of_diff_ns = 0.0;

    char servbuffer[50];

    // Sending time daemon's clock value to each of the clients/nodes;
    cout << thread_counter << endl;
    cout << globalfd[0] << endl;
    clock_gettime(CLOCK_REALTIME, &curr_time);
    clock_gettime(CLOCK_REALTIME, &start_time);

    for (int i = 0; i < thread_counter; i++)
    {
        int temp_fd = globalfd[i]; // temporary socket fd
        cout << "current time is " << curr_time.tv_sec << "s " << curr_time.tv_nsec << "ns" << endl;
        string server = to_string(curr_time.tv_sec);
        server += ".";
        server += to_string(curr_time.tv_nsec);
        strcpy(servbuffer, server.c_str());

        int ret = write(temp_fd, servbuffer, 50); // send time to clients
        // cout << "write " << temp_fd << endl;
        if (ret < 0)
        {
            cout << "Error in sending server value" << endl;
        }
        else
        {
            // cout << "write success " << endl;
            memset(sendbuffer, '\0', sizeof(sendbuffer));
            int diffread = read(temp_fd, recvbuffer, 50);

            double *res = char2double(recvbuffer);

            // 存储Master节点和普通节点时间的差值
            difference_s[i] = res[0] - curr_time.tv_sec; // storing them in an array
            difference_ns[i] = res[1] - curr_time.tv_nsec;
            cout << "Differences received from clock[" << i << "]=>" << difference_s[i] << " " << difference_ns[i] << endl;
            sum_of_diff_s = sum_of_diff_s + difference_s[i];
            sum_of_diff_ns = sum_of_diff_ns + difference_ns[i];
        }
    }
    // 对差值计算算术平均值
    avg_s = sum_of_diff_s / (thread_counter + 1);
    avg_ns = sum_of_diff_ns / (thread_counter + 1);

    cout << "Average of differences=>" << avg_s << " " << avg_ns << endl;
    send_offset();
}

// czlculating and sending the new offset value to the local clocks at client side
void send_offset()
{
    for (int i = 0; i < thread_counter; i++)
    {
        int temp_fd = globalfd[i];

        // 计算普通节点需要更新的时间偏移
        int new_offset_s = avg_s - difference_s[i];
        int new_offset_ns = avg_ns - difference_ns[i];
        cout << "offset " << temp_fd << " " << new_offset_s << "s " << new_offset_ns << endl;
        string newoff = to_string(new_offset_s) + "." + to_string(new_offset_ns);
        
        memset(sendbuffer, '\0', sizeof(sendbuffer));
        strcpy(sendbuffer, newoff.c_str());

        int ret = write(temp_fd, sendbuffer, 50);
        if (ret < 0)
            cout << "error in sending offset" << endl;
    }
    cout << fixed;
    cout << setprecision(0);
    cout << "Local time for time daemon=>" << curr_time.tv_sec << "s " << curr_time.tv_nsec << endl;
    cout << "Time updated for time daemon=>" << curr_time.tv_sec + avg_s << "s " << curr_time.tv_nsec + avg_ns << endl;
    clock_gettime(CLOCK_REALTIME, &end_time);
    cout << "after algorithm time is " << end_time.tv_sec << "s " << end_time.tv_nsec << "ns" << endl;
     
    cout << "the accuracy of the algorithm is " << end_time.tv_sec - (curr_time.tv_sec + avg_s) << "s " << end_time.tv_nsec - (curr_time.tv_nsec + avg_ns) << "ns" << endl;
    cout << "The Time to complete the algorithm is " << end_time.tv_sec - start_time.tv_sec << "s " << end_time.tv_nsec - start_time.tv_nsec << "ns" << endl;
    
    // exit(0);
    return;
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

    // Randomizing local clock values between 0 and 21
    // time_t timer;
    // srand((unsigned)time(&timer));
    // serv_clock = rand() % 20 + 1;

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
