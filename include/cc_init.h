#ifndef _CC_INIT_H_
#define _CC_INIT_H_

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "cc_common.h"
#include "cc_log.h"


#define CC_VERSION "0x01"
#define COMMAND_STR_LEN 30
#define HELP_STR_LEN    50
#define CC_STRING_MAX_LEN 100
#define COMMAND_NUM 8
#define CC_LENGTH_OF_LISTEN_QUEUE 20
#define CC_MAX_SOCKET_BUFF 1024
#define CC_CONN_TIMEOUT_SEC 0
#define CC_CONN_TIMEOUT_USEC 0

#define CC_MAX_THREAD_NUM 3
#define CC_ACCEPT_FD 3
#define CC_CHILD 1
#define CC_PARENT 0

/* Define the string length of IPv4 address. */
#define CC_IP_ADDRESS_STRING_LEN 20

/* Define the server's port number. */
#define CC_CONTROLLER_PORT 6633

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

#define CC_SWITCH_MAX_NUM 20


struct cc_command_line_s{
	char cmd_str[COMMAND_STR_LEN];
	char help_str[HELP_STR_LEN];
} pof_start_commands;
typedef struct cc_command_line_s cc_command_line_t;



#define MAXBUF 1024
#define MAXEPOLLSIZE 10000


int cc_set_init_config(int argc, char **argv);
//int cc_finish_init(void);
int cc_server_conn_create(server_info_t *si);
int cc_conn_accept(server_info_t *si); 

#endif //__CC_INIT_H__
