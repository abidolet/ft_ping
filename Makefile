NAME = ft_ping
MODE ?= release

BIN_DIR = .bin-$(MODE)
INCLUDES = -Iincludes

CC = cc
CFLAGS = -Wall -Werror -Wextra -MD $(INCLUDES)
LDFLAGS = -lm

ifeq ($(MODE), debug)
	CFLAGS = -Wall -Wextra -MD $(INCLUDES) -g3
endif

VPATH = srcs

SRCS =	main.c	\
		parse.c	\
		run.c	\

OBJS = $(addprefix $(BIN_DIR)/, $(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

RESET	= \033[0m
GRAY	= \033[90m
RED 	= \033[31m
GREEN 	= \033[32m
YELLOW 	= \033[33m
BLUE 	= \033[34m

all:
	$(MAKE) $(NAME)
	printf "$(RESET)"

debug:
	$(MAKE) clean
	$(MAKE) MODE=debug all

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: %.c Makefile | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	printf "$(GRAY)compiling: $(BLUE)%-40s $(GRAY)[%d/%d]\n" "$<" "$$(ls $(BIN_DIR) | grep -c '\.o')" "$(words $(SRCS))"

clean:
	rm -rf .bin-release .bin-debug

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re debug

-include $(DEPS)

.SILENT:
MAKEFLAGS=--no-print-directory