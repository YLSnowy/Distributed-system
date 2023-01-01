#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

void gettime64(uint32_t ts[])
{
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);

	ts[0] = tv.tv_sec;
	ts[1] = tv.tv_nsec;
}

int my_send(int socket_fd, struct sockaddr *saddr_p, socklen_t saddrlen, unsigned char recv_buf[], uint32_t recv_time[])
{
	unsigned char send_buf[48];
	uint32_t *u32p;

	send_buf[0] = (recv_buf[0] & 0x38) + 4;
	send_buf[1] = 0x01;
	*(uint32_t *)&send_buf[12] = htonl(0x4C4F434C);
	send_buf[2] = recv_buf[2];
	send_buf[3] = (signed char)(-6);
	u32p = (uint32_t *)&send_buf[4];
	*u32p++ = 0;
	*u32p++ = 0;
	u32p++;
	gettime64(u32p);
	*u32p = htonl(*u32p - 60);
	u32p++;
	*u32p = htonl(*u32p);
	u32p++;
	*u32p++ = *(uint32_t *)&recv_buf[40];
	*u32p++ = *(uint32_t *)&recv_buf[44];
	*u32p++ = htonl(recv_time[0]);
	*u32p++ = htonl(recv_time[1]);
	gettime64(u32p);
	printf("server send time is: %ds %dns\n",u32p[0], u32p[1]);
	*u32p = htonl(*u32p);
	u32p++;
	*u32p = htonl(*u32p);

	int ret = sendto(socket_fd, send_buf, sizeof(send_buf), 0, saddr_p, saddrlen);
	if (ret < 48)
	{
		printf("sendto fail");
		return 1;
	}
	printf("++++++++++++++++++++++++++++++++++++++++++++\n");
	return 0;
}

void my_receive(int fd)
{
	struct sockaddr src_addr;
	socklen_t src_addrlen = sizeof(src_addr);
	unsigned char buf[48];
	uint32_t recv_time[2];
	pid_t pid;

	while (1)
	{
		while (1)
		{
			int ret = recvfrom(fd, buf, 48, 0, &src_addr, &src_addrlen);
			if (ret >= 48)
				break;
		};

		gettime64(recv_time);
		printf("receive client time is: %ds %dns\n",recv_time[0], recv_time[1]);

		pid = fork();
		if (pid == 0)
		{
			my_send(fd, &src_addr, src_addrlen, buf, recv_time);
			exit(0);
		}
		else if (pid == -1)
		{
			printf("fork() fail\n");
			exit(0);
		}
	}
}

int main(int argc, char *argv[], char **env)
{
	struct sockaddr_in bind_addr;
	memset(&bind_addr, 0, sizeof(bind_addr));

	int s;
	struct sockaddr_in sinaddr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1)
	{
		printf("Can not create socket.");
		exit(0);
	}

	memset(&sinaddr, 0, sizeof(sinaddr));
	sinaddr.sin_family = AF_INET;
	sinaddr.sin_port = htons(1234);
	sinaddr.sin_addr.s_addr = bind_addr.sin_addr.s_addr;

	if (0 != bind(s, (struct sockaddr *)&sinaddr, sizeof(sinaddr)))
	{
		printf("Bind error");
		exit(0);
	}

	printf("listening\n");

	my_receive(s);
	close(s);
	return 0;
}