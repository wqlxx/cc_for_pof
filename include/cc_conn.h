#include <cc_common.h>

/* Define the string length of IPv4 address. */
#define CC_IP_ADDRESS_STRING_LEN 20

/* Define the server's port number. */
#define CC_CONTROLLER_PORT_NUM 6633

/* Define the max retry time of cnnection. */
#define CC_CONNECTION_MAX_RETRY_TIME 0XFFFFFFFF

/* Define the retry interval of connection if connection fails. */
#define CC_CONNECTION_RETRY_INTERVAL 2  /* Seconds. */

/* Define max size of sending buffer. */
#define CC_SEND_BUF_MAX_SIZE CC_MESSAGE_SIZE

/* Define max size of receiving buffer. */
#define CC_RECV_BUF_MAX_SIZE CC_MESSAGE_SIZE

/* Define echo interval .*/
#define CC_ECHO_INTERVAL (2000)  /* Unit is millisecond. */

/* Message queue attributes. */
#define CC_QUEUE_MESSAGE_LEN CC_MESSAGE_SIZE


/**********************************************/
/*temp code for high performance of sever     */
/**********************************************/

#ifndef __Q_SOCKET_SERVER__
#define __Q_SOCKET_SERVER__
#include <errno.h>

#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>


#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef unsigned short WORD;
typedef unsigned int DWORD;
#define QSS_SIO_KEEPALIVE_VALS_TIMEOUT 30*60
#define QSS_SIO_KEEPALIVE_VALS_INTERVAL 5
#define QSS_SIO_KEEPALIVE_VALS_COUNT 3
#define MAX_THREADS 100
#define MAX_THREADS_MIN  10
#define MIN_WORKER_WAIT_TIMEOUT  20*1000
#define MAX_WORKER_WAIT_TIMEOUT  60*MIN_WORKER_WAIT_TIMEOUT
#define MAX_THREADPOOLS  32

#define MAX_BUF_SIZE 1024
/* ulimit -n opened FDs per process.记得修改哦，否则还是select效果,就不是epoll效果了哦，呵呵*/
#define BLOCKING_SEND_TIMEOUT 20

typedef void (*CSocketLifecycleCallback)(int cs,int lifecycle);//lifecycle:0:OnAccepted,-1:OnClose
typedef int (*BlockingSender_t)(void * senderBase,int cs, void * buf, size_t nbs);
typedef int (*InternalProtocolHandler)(struct epoll_event * event,BlockingSender_t _blockingSender,void * senderBase);//return -1:SOCKET_ERROR

struct QSSWORKER_PARAM_s{
 QSocketServer * qss;
 pthread_t th;
};
typedef struct QSSWORKER_PARAM_s QSSWORKER_PARAM;

struct QSocketServer{
  WORD passive;
  WORD port;//uint16_t
  WORD minThreads;
  WORD maxThreads;
  pthread_spinlock_t g_spinlock;//PTHREAD_PROCESS_PRIVATE
  volatile int lifecycleStatus;//0-created,1-starting, 2-running,3-stopping,4-exitSignaled,5-stopped
  int  workerWaitTimeout;//wait timeout
  volatile int workerCounter;
  volatile int currentBusyWorkers;
  volatile int CSocketsCounter;
  CSocketLifecycleCallback cslifecb;
  InternalProtocolHandler protoHandler;
  SOCKET server_s;
  SOCKADDR_IN serv_addr;
  int epollFD;//main epoller.
  int BSendEpollFD;//For blocking send.
};
typedef struct QSocketServer_s QSocketServer;

struct QSSEPollEvent_s {
  SOCKET client_s;
  SOCKADDR_IN client_addr;
  uint32_t curEvents;

  char buf[MAX_BUF_SIZE];
  DWORD numberOfBytesTransferred;
  char * data;

  int BSendEpollFDRelated;
  pthread_mutex_t writableLock;
  pthread_cond_t  writableMonitor;
};//for per connection
typedef struct QSSEPollEvent_s QSSEPollEvent;


void *epollWorkerRoutine(void *);
void *blockingSendEpollerRoutine(void *);
int createSocketServer(QSocketServer ** qss_ptr,WORD passive,WORD port,CSocketLifecycleCallback cslifecb,InternalProtocolHandler protoHandler,WORD minThreads,WORD maxThreads,int workerWaitTimeout);
int startSocketServer(QSocketServer *qss);
int shutdownSocketServer(QSocketServer *qss);
#endif

