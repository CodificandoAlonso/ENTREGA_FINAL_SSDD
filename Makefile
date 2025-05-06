compiler = gcc
CFLAGS  = -Wall -Wextra -fPIC -lsqlite3 $(INCLUDES)
INCLUDES = -I src/headers


#SERVIDOR
SERVER_SRCS = src/server/server.c src/server/database_control.c src/server/sql_recall.c src/server/message_control.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
SERVER_BIN  = servidor


#Regla para compilarlo todo
all: $(SERVER_BIN)


#COMPILACION SERVER
$(SERVER_BIN): $(SERVER_OBJS)
	$(compiler) -o $@ $(SERVER_OBJS) $(CFLAGS)

#REGLA PARA CREAR LOS .o
%.o: %.c
	$(compiler) $(CFLAGS) -c $< -o $@

USER_NAME := $(shell whoami)

#LIMPIA; ME LO CARGO TODO
clean:
	rm -f $(SERVER_OBJS) \
	      $(SERVER_BIN)
	rm -rf database-$(USER_NAME).db
