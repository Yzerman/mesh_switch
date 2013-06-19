#on Sun you need these
#EXTRA_LIBS=-lnsl -lsocket
EXTRA_LIBS=

BUILD_OPTS=-g

clean:
	rm -f *~ *.bak

all: conn_client.c conn_client.h conn_server.c conn_server.h conn_io.c Mesh_Test.c Mesh.c llist.c llist.h Makefile
	gcc ${BUILD_OPTS} -c conn_client.c -o conn_client.o
	gcc ${BUILD_OPTS} -c conn_server.c -o conn_server.o
	gcc ${BUILD_OPTS} -c conn_io.c     -o conn_io.o
	gcc ${BUILD_OPTS} -c getMesh_paket.c     -o getMesh_paket.o
	gcc ${BUILD_OPTS} -c llist.c     -o llist.o
	gcc ${BUILD_OPTS} Mesh_Test.c conn_server.o conn_client.o conn_io.o llist.o getMesh_paket.o ${EXTRA_LIBS} -o Mesh_Test
	gcc ${BUILD_OPTS} Mesh.c conn_server.o conn_client.o conn_io.o llist.o getMesh_paket.o${EXTRA_LIBS} -o Mesh -pthread

