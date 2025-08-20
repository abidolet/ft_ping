#include "ft_ping.h"

int	main(int argc, char **argv)
{
	t_data	data = {0};

	parse_data(argc, argv, &data);
	run(&data);
	return (0);
}