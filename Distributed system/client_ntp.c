#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

struct timespec start_time, end_time;
void gettime64(uint32_t ts[])
{
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);

	ts[0] = tv.tv_sec;
	ts[1] = tv.tv_nsec;
}

void my_send(int fd)
{
	unsigned char buf[48] = {0};
	uint32_t tts[2];

	buf[0] = 0x23;

	gettime64(tts);
	clock_gettime(CLOCK_REALTIME, &start_time);
	printf("client send time is: %ds %dns\n",tts[0], tts[1]);
	(*(uint32_t *)&buf[40]) = htonl(tts[0]);
	(*(uint32_t *)&buf[44]) = htonl(tts[1]);
	if (send(fd, buf, 48, 0) != 48)
	{
		printf("Send fail\n");
	}
}

void my_receive(int fd)
{
	unsigned char buf[48];
	uint32_t *pt;

	uint32_t t1[2];
	uint32_t t2[2];
	uint32_t t3[2];
	uint32_t t4[2];
	double T1, T2, T3, T4;
	double tfrac = 1000000000.0;
	time_t curr_time;
	time_t diff_sec;

	if (recv(fd, buf, 48, 0) < 48)
	{
		printf("Receive error\n");
	}
	gettime64(t4);
	printf("client receive time is: %ds %dns\n",t4[0], t4[1]);
	
	pt = (uint32_t *)&buf[24];
	t1[0] = htonl(*pt++);
	t1[1] = htonl(*pt++);

	t2[0] = htonl(*pt++);
	t2[1] = htonl(*pt++);

	t3[0] = htonl(*pt++);
	t3[1] = htonl(*pt++);

	T1 = t1[0] + t1[1] / tfrac;
	T2 = t2[0] + t2[1] / tfrac;
	T3 = t3[0] + t3[1] / tfrac;
	T4 = t4[0] + t4[1] / tfrac;

	printf("Tround = %lf\n",(T4 - T1) - (T3 - T2));

	diff_sec = ((int32_t)(t2[0] - t1[0]) + (int32_t)(t3[0] - t4[0])) / 2;
	curr_time = time(NULL) - diff_sec;
	// printf("Current Time at Server:   %s\n", ctime(&curr_time));
	printf("after updated time is %ds %dns\n", (t3[0]+t4[0]-t1[0]+t2[0])/2, (t3[1]+t4[1]-t1[1]+t2[1])/2);
	struct timespec my_time;
	clock_gettime(CLOCK_REALTIME, &my_time);
	printf("current time is %lds %ldns\n", my_time.tv_sec, my_time.tv_nsec);
}

int main(int argc, char *argv[], char **env)
{
	if (argc < 2)
	{
		printf("need address\n");
		return 0;
	}

	int s;
	struct addrinfo *saddr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1)
	{
		printf("Can not create socket.\n");
	}

	if (0 != getaddrinfo(argv[1], "1234", NULL, &saddr))
	{
		printf("Server address not correct.\n");
	}

	if (connect(s, saddr->ai_addr, saddr->ai_addrlen) != 0)
	{
		printf("Connect error\n");
	}
	my_send(s);
	my_receive(s);
	close(s);
	return 0;
}