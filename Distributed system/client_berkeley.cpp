#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <string.h>
#include <iomanip>

using namespace std;
#define PORT 1234

double local_clock;
double server_clock;

int calculate(); // function to calculate differences

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
    cout << res[0] << " " << res[1] << endl;
    return res;
}

int main(int argc, char *argv[])
{

    // time_t timer;
    // srand((unsigned)time(&timer));
    // sleep(1);
    // local_clock = rand() % 20 + 1; // randomize time for each thread between 1 to 20

    struct timespec curr_time;
    int serverfd, serverlen;
    struct sockaddr_in servaddr;

    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
    serverlen = sizeof(servaddr);

    // Connect to server
    int ret = connect(serverfd, (struct sockaddr *)&servaddr, serverlen);
    if (ret > 0)
        cout << "connect success " << serverfd << endl;

    // initializing buffers and variables
    char sendbuffer[50] = {0};
    char recvbuffer[50] = {0};
    char servbuffer[50] = {0};
    double diff = 0.0;
    int avg = 0;

    // read time daemon time from server
    while (1)
    {
        ret = read(serverfd, servbuffer, 50);
        clock_gettime(CLOCK_REALTIME, &curr_time);

        cout << "Local clock time :" << curr_time.tv_sec << "s " << curr_time.tv_nsec << endl;
        if (ret < 0)
        {
            cout << "error in server value";
        }
        else
        {
            cout << servbuffer << endl;

            double *res = char2double(servbuffer);

            string local_time = to_string(curr_time.tv_sec) + "." + to_string(curr_time.tv_nsec);
            memset(sendbuffer, '\0', sizeof(sendbuffer));
            strcpy(sendbuffer, local_time.c_str());
            // send differences to time daemon
            ret = write(serverfd, sendbuffer, 50);
            if (ret < 0)
            {
                cout << "send error" << endl;
            }

            // Receive offset from time daemon
            ret = read(serverfd, recvbuffer, 50);
            cout << recvbuffer << endl;
            if (ret < 0)
            {
                cout << "Error receiving offset";
            }
            else
            {
                res = char2double(recvbuffer);
                cout << "Offset received from daemon process : " << res[0] << "s " << res[1] << endl;
                // calculate synchronized time
                // int updated_time = offset_new + local_clock;
                cout << fixed;
                cout << setprecision(0);
                cout << "Updated local clock time is => " << curr_time.tv_sec + res[0] << "s " << curr_time.tv_nsec + res[1] << endl;
            }

            cout << "++++++++++++++++++++++++++++++++++++++++++++++++=" << endl;
        }
    }
    close(serverfd);
    return 0;
}
