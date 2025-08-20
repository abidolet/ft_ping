#include "ft_ping.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void	print_options()
{
	const char	*msg = "Usage: ft_ping [OPTION...] HOST ...\n"
		"Send ICMP ECHO_REQUEST packets to network hosts.\n\n"
		"   -v, --verbose\tverbose output\n"
		"   -?, --help\t\tgive this help list\n"
		"\nReport bugs to <abidolet@student.42lyon.fr>.\n";

	dprintf(2, msg);
	exit(0);
}

static e_options	get_option_enum(const char *opt_str)
{
	t_options	option_map[sizeof(e_options)] = {
		{"verbose", VERBOSE, false},
		{"help", HELP, false},
	};

	const size_t	opt_size = strlen(opt_str);
	e_options		option = INVALID;
	size_t			nb_options = 0;

	for (size_t i = 0; option_map[i].name; i++)
	{
		if (strncmp(opt_str, option_map[i].name, opt_size) == 0)
		{
			option_map[i].selected = true;
			option = option_map[i].opt;
			nb_options++;
		}
	}

	if (nb_options > 1)
	{
		dprintf(2, "ft_ping: option '--%s' is ambiguous; possibilities:", opt_str);
		for (size_t i = 0; option_map[i].name; i++)
		{
			if (option_map[i].selected)
			{
				dprintf(2, " '--%s'", option_map[i].name);
			}
		}
		dprintf(2, "\n");
		exit(64);
	}

	return (option);
}

static void	parse_options(char *arg, t_data *data)
{
	if (*arg != '-')
	{
		while (*arg)
		{
			switch (*arg)
			{
				case 'v':	data->verbose = true; break;
				case '?':	print_options(); break;
				default:	dprintf(2, "ft_ping: invalid option -- '%c'\n"
								"Try 'ft_ping --help' for more information.\n", *arg);
							exit(64);
			}
			arg++;
		}
	}
	else
	{
		switch (get_option_enum(arg + 1))
		{
			case VERBOSE:	data->verbose = true; break ;
			case HELP:		print_options(); break ;
			default:		dprintf(2, "ft_ping: unrecognized option '--%s'\n"
								"Try 'ft_ping --help' for more information.\n", arg + 1);
							exit(64);
		}
	}
}

void	parse_data(int argc, char **argv, t_data *data)
{
	for (int i = 1; i < argc; i++)
	{
		char	*arg = argv[i];
		if (*arg == '-')
		{
			parse_options(arg + 1, data);
		}
		else
		{
			data->host = arg;
		}
	}

	if (data->host == NULL)
	{
		dprintf(2, "ft_ping: ping: missing host operand\n"
			"Try 'ft_ping --help' for more information.\n");
		exit(64);
	}

}
