/*
 * for video testing client  sagebane
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#define book "deadbeef"

#include "drv_kedacapture.h"

#define QCIF (176*144)
#define SIZE QCIF

typedef struct {
	pthread_t thread;
	int thread_stat;
	int nfd;

	int port;
	char *host;
} CD;				/* capture describer */

static int getkey(void)
{      
        int in;
       
        struct termios new_settings;
        struct termios stored_settings;
        tcgetattr(0,&stored_settings);
        new_settings = stored_settings;
        new_settings.c_lflag &= (~ICANON);
        new_settings.c_cc[VTIME] = 0;
        tcgetattr(0,&stored_settings);
        new_settings.c_cc[VMIN] = 1;
        tcsetattr(0,TCSANOW,&new_settings);

        in = getchar();

        tcsetattr(0,TCSANOW,&stored_settings);
        return in;
}
static void loadfirmware()
{
	
	RemotePrtOpen();

	NetraProcOpen( 2, "fw_m3vpss.xem3");
	sleep(2);
	printf("cut line..............................\n");	
//	NetraProcOpen( 1, "ipcsvr.xem3");

	printf("load firmware finished\n");
}

static void unloadfirmware()
{
//	NetraProcClose(1);
	NetraProcClose(2);
}

static void *process(void *arg)
{
	char buf[SIZE*3/2];
	int ret;
	unsigned short w, h;
	Keda_BufInfo bufinfo;
	int fd;

	CD *cd = (CD *)arg;
	printf("new thread!\n");
	cd->nfd = netinit(cd->host, cd->port);
	
	while(cd->thread_stat){
		ret = Keda_CaptureGetBuf(0, &bufinfo);
		if (ret){
			printf ("%s: get buffer failed!\n", __FUNCTION__);
			continue;
		}
		w = bufinfo.imgW;
		h = bufinfo.imgH ; 

		if (bufinfo.bufvirt){
			ret = netsend(cd->nfd, bufinfo.bufvirt, w*h*3/2);
		}
		ret = Keda_CapturePutBuf(0, &bufinfo);
		usleep(1*1000); 
	}
	netexit(cd->nfd);
	pthread_exit(0);
}
//#define ENABLE_ENCODE 1

int main(int argc, char **argv)
{
	int fd,ret, nfd;
	CD cd;
	Keda_CapPrm param;
	
	if (argc != 3){
		printf("usage: %s <host> <port>\n", argv[0]);
		exit(0);
	}
	printf("%s\n", book);
	
	loadfirmware();
	
	memset(&cd, 0, sizeof(CD));

	cd.host = argv[1];
	cd.port = atoi(argv[2]);

	param.width1 = 720;//  1280;
	param.height1 = 576;// 576;

	param.width2 = 320;
	param.height2 = 240;
//	param.aewbenable = 1;
	printf("........main.\n");
	
	ret = Keda_Ipnc_CaptureCreate(&param);
	printf("........main...\n");
	if (ret){
		printf ("Capture Create failed\n");
		return -1;
	}

	cd.thread_stat = 1;
	pthread_create(&cd.thread, 0, process, &cd);
	unsigned short w,h;
	int kcode;
	while(1)
	{
		kcode = getkey();
		if ('q' == kcode){
			cd.thread_stat = 0;
			break;
		}
		else if ('t' == kcode){
			printf("......w:%d, h:%d.....\n", w, h);
			Keda_CaptureGetParam(0, &w, &h);
			printf("......w:%d, h:%d.....\n", w, h);
			sleep(1);
			Keda_CaptureSetParam(0, 320, 240);
			sleep(1);
			Keda_CaptureGetParam(0, &w, &h);
			printf("o......w:%d, h:%d.....\n", w, h);

		}
		
		usleep(200*000);
	}

	pthread_join(cd.thread, 0);
	Keda_Ipnc_CaptureDelete();
	unloadfirmware();
	
	return 0;
}

