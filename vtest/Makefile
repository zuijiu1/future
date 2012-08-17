CROSS=/opt/armlite/bin/arm-none-linux-gnueabi-
CC=$(CROSS)gcc
LD=$(CROSS)ld

LDFLAG = -L./lib/ -lpthread -lkeda_ipnc -lenc_link_api -lipnc_rdk_osa -lnetsend -lsyslink


DNFS=/home/sgbn/share/nfs/opt/test/
all: client server 
	cp client $(DNFS)

server: server.o
	cc -o $@ $^ 
client: client.o net.o 
	$(CC) -o $@ $^  $(LDFLAG) 
server.o: server.c
	cc -c $^

%.o: %.c
	$(CC) -c $^ $(INC)

clean:
	-rm server *.o *~ client 
.PHONY: all clean
