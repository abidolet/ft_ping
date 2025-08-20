#ifndef FT_PING_H
#define FT_PING_H

# include <stdbool.h>
# include <netinet/ip_icmp.h>

# define PACKET_SIZE 56
# define PACKET_SIZE_TOTAL (sizeof(struct iphdr) + sizeof(struct icmphdr) + PACKET_SIZE)
# define MIN(a, b) (a < b ? a : b)
# define MAX(a, b) (a > b ? a : b)
# define CLOSE(fd) if (fd > 1) {close(fd); fd = -1;}

typedef struct	s_data
{
	bool			verbose;
	char			*host;
	struct addrinfo	*res;
	int				sockfd;
	char			*addr;
	int				epoll_fd;
	struct timeval	start;
	size_t			packets_sent;
	size_t			packets_received;
	float			packet_loss;
	float			min_rtt;
	float			max_rtt;
	float			total_rtt;
	float			total_rtt_sq;
	float			avg_rtt;
	float			stddev_rtt;
}   t_data;

typedef enum	e_options
{
	VERBOSE,
	HELP,
	INVALID
}	e_options;

typedef struct options
{
	const char	*name;
	e_options	opt;
	bool		selected;
}	t_options;

struct icmp_packet
{
    struct icmphdr	hdr;
    char            msg[PACKET_SIZE];
};

// parse.c
void	parse_data(int argc, char **argv, t_data *data);

// run.c
void	run(t_data *data);

#endif