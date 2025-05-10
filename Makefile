.DEFAULT_GOAL := all

# --- Flags para libtirpc (RPC) ---
TIRPC_CFLAGS := $(shell pkg-config --cflags libtirpc)
TIRPC_LIBS   := $(shell pkg-config --libs   libtirpc)

# --- Herramientas y flags de compilación ---
compiler := gcc
INCLUDES := -I src/headers
CFLAGS   := -Wall -Wextra -fPIC $(INCLUDES) $(TIRPC_CFLAGS)
LDFLAGS  := -lsqlite3 $(TIRPC_LIBS)

# --- Servidor "clásico" ---
SERVER_SRCS := \
	src/server/database_control.c \
	src/server/sql_recall.c      \
	src/server/message_control.c  \
	src/server/server.c
SERVER_OBJS := $(SERVER_SRCS:.c=.o)
SERVER_BIN  := servidor

# --- Generación de stubs RPC con rpcgen ---
RPC_X    := src/rpc-handler/server-rpc-builder.x
RPCGEN   := rpcgen -C -M

RPC_HDR  := src/rpc-handler/server-rpc-builder.h
RPC_CLNT := src/rpc-handler/rpc-service_clnt.c
RPC_SVC  := src/rpc-handler/rpc-service_svc.c
RPC_XDR  := src/rpc-handler/rpc-service_xdr.c

.PHONY: stubs
stubs: $(RPC_HDR) $(RPC_CLNT) $(RPC_SVC) $(RPC_XDR)

$(RPC_HDR): $(RPC_X)
	cd src/rpc-handler && $(RPCGEN) -h -o server-rpc-builder.h server-rpc-builder.x

$(RPC_CLNT): $(RPC_X)
	cd src/rpc-handler && $(RPCGEN) -l -o rpc-service_clnt.c server-rpc-builder.x

$(RPC_SVC): $(RPC_X)
	cd src/rpc-handler && $(RPCGEN) -m -o rpc-service_svc.c server-rpc-builder.x

$(RPC_XDR): $(RPC_X)
	cd src/rpc-handler && $(RPCGEN) -c -o rpc-service_xdr.c server-rpc-builder.x

# --- Servidor RPC separado ---
RPC_SERVER_SRCS := \
	 src/rpc-handler/server-RPC.c \
	 $(RPC_SVC)           \
	 $(RPC_XDR)
RPC_SERVER_OBJS := $(RPC_SERVER_SRCS:.c=.o)
RPC_SERVER_BIN  := servidor_rpc

# --- Meta-targets ---
.PHONY: all clean
all: stubs $(SERVER_BIN) $(RPC_SERVER_BIN)

# Linkeo servidor tradicional
$(SERVER_BIN): $(SERVER_OBJS)
	$(compiler) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Linkeo servidor RPC
$(RPC_SERVER_BIN): $(RPC_SERVER_OBJS)
	$(compiler) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Regla genérica .c → .o
%.o: %.c
	$(compiler) $(CFLAGS) -c $< -o $@

# --- Limpieza completa ---
USER_NAME := $(shell whoami)
clean:
	rm -f \
	  $(SERVER_OBJS) $(SERVER_BIN) \
	  $(RPC_SERVER_OBJS) $(RPC_SERVER_BIN) \
	  $(RPC_HDR) $(RPC_CLNT) $(RPC_SVC) $(RPC_XDR)
	rm -rf database-$(USER_NAME).db
