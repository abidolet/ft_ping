#include "ft_ping.h"
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <math.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

volatile sig_atomic_t	g_stop = 0;

void	print_statistics(t_data *data)
{
	printf("--- %s ping statistics ---\n", data->host);

	if (data->packets_sent > 0)
	{
		data->packet_loss = (float)(data->packets_sent - data->packets_received) / data->packets_sent * 100;
	}
	printf("%zu packets transmitted, %zu packets received, %.0f%% packet loss\n",
		data->packets_sent, data->packets_received, data->packet_loss);

	if (data->packets_received > 0)
	{
		data->avg_rtt = data->total_rtt / data->packets_received;
		float	variance = (data->total_rtt_sq / data->packets_received) -
			(data->avg_rtt * data->avg_rtt);
		data->stddev_rtt = sqrtf(variance);
	}
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n",
		data->min_rtt, data->avg_rtt, data->max_rtt, data->stddev_rtt);
}

unsigned short compute_checksum(unsigned short *addr, int len)
{
	int				nleft = len;
	int				sum = 0;
	unsigned short	*w = addr;
	unsigned short	answer = 0;

	while (nleft > 1) 
	{
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1)
	{
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;
	return (answer);
}

void	receive_packet(t_data *data)
{
	char				recv_buf[PACKET_SIZE_TOTAL];
	struct sockaddr_in	from = {0};
	socklen_t			fromlen = sizeof(from);

	ssize_t	bytes_received = recvfrom(data->sockfd, recv_buf,
		sizeof(recv_buf), 0, (struct sockaddr *)&from, &fromlen);
	if (bytes_received <= 0)
	{
		perror("recvfrom");
		return ;
	}

	struct timeval	end;
	gettimeofday(&end, NULL);

	struct iphdr	*ip_hdr = (struct iphdr *)recv_buf;
	struct icmphdr	*icmp_hdr = (struct icmphdr *)(recv_buf + (ip_hdr->ihl * 4));

	if (icmp_hdr->un.echo.id != getpid())
	{
		return ;
	}

	float	elapsed = (float)(end.tv_sec - data->start.tv_sec) * 1000.0f +
		(float)(end.tv_usec - data->start.tv_usec) / 1000.0f;
	data->packets_received++;
	data->total_rtt += elapsed;
	data->total_rtt_sq += elapsed * elapsed;
	data->min_rtt = MIN(data->min_rtt, elapsed);
	data->max_rtt = MAX(data->max_rtt, elapsed);

	printf("%d bytes from %s: icmp_seq=%zu ttl=%u time=%.3f ms\n",
		ip_hdr->ttl, data->addr, data->packets_sent, ip_hdr->ttl, elapsed);
}

void	send_packet(t_data *data)
{
	struct icmp_packet	packet = {0};

	packet.hdr.type = ICMP_ECHO;
	packet.hdr.un.echo.id = getpid();
	packet.hdr.un.echo.sequence = data->packets_sent + 1;
	memset(packet.msg, 0x42, PACKET_SIZE);
	packet.hdr.checksum = compute_checksum((unsigned short *)&packet, sizeof(packet));

	gettimeofday(&data->start, NULL);

	if (sendto(data->sockfd, &packet, sizeof(packet), 0,
		data->res->ai_addr, data->res->ai_addrlen) < 0)
	{
		perror("sendto");
		return ;
	}
}

void	sigint_handler(int signal)
{
	(void)signal;
	g_stop = 1;
}

void	clear(t_data *data)
{
	CLOSE(data->epoll_fd);
	CLOSE(data->sockfd);
	freeaddrinfo(data->res);
	exit(64);
}

void	init(t_data *data)
{
	data->epoll_fd = epoll_create1(0);
	if (data->epoll_fd < 0)
	{
		perror("epoll_create1");
		exit(64);
	}

	data->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (data->sockfd < 0)
	{
		perror("socket");
		clear(data);
	}

	struct addrinfo hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	if (getaddrinfo(data->host, NULL, &hints, &data->res) != 0)
	{
		perror("getaddrinfo");
		clear(data);
	}

	struct epoll_event ev = {0};
	ev.events = EPOLLIN;
	ev.data.ptr = data;

	if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, data->sockfd, &ev) < 0)
	{
		perror("epoll_ctl");
		clear(data);
	}

	data->addr = inet_ntoa(((struct sockaddr_in *)data->res->ai_addr)->sin_addr);
	printf("PING %s (%s): %d data bytes", data->host,
		data->addr, PACKET_SIZE);

	data->min_rtt = __FLT_MAX__;

	if (data->verbose)
	{
		printf(", id 0x%04x = %d", getpid(), getpid());
	}
	printf("\n");
}

void	run(t_data *data)
{
	init(data);
	signal(SIGINT, sigint_handler);

	while (!g_stop)
	{
		send_packet(data);

		struct epoll_event	events[1];
		int nfds = epoll_wait(data->epoll_fd, events, 1, 1000);
		if (nfds <= 0)
		{
			perror("epoll_wait");
		}
		else if (nfds > 0 && events[0].events & EPOLLIN)
		{
			receive_packet(data);
		}
		data->packets_sent++;
		sleep(1);
	}

	CLOSE(data->epoll_fd);
	CLOSE(data->sockfd);
	freeaddrinfo(data->res);
	print_statistics(data);
}
