#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "cc_common.h"
#include "cc_init.h"
#include "cc_error.h"

int cc_cmd_set_controller_ip = CC_CMD_NULL;
int cc_cmd_set_controller_port = CC_CMD_NULL;
int cc_cmd_set_config_file = CC_CMD_NULL;
int cc_cmd_auto_clear = CC_CMD_NULL;
int cc_cmd_auto_reconnect = CC_CMD_NULL;

/* Controller ip. */
char cc_controller_ip_addr[CC_IP_ADDRESS_STRING_LEN] = "127.0.0.1";

/* Controller port. */
int cc_controller_port = CC_CONTROLLER_PORT_NUM;

/* The max retry time of cnnection. */
int cc_conn_max_retry = CC_CONNECTION_MAX_RETRY_TIME;

/* The retry interval of cnnection if connect fails. */
int cc_conn_retry_interval = CC_CONNECTION_RETRY_INTERVAL;

/* Openflow action id. */
uint32_t cc_xid = CC_INITIAL_XID;

int cc_switch_max_num = CC_SWITCH_MAX_NUM;

char cc_cmd_config_file[100] = "";

const cc_command_line_t cmds[COMMAND_NUM] = {
	{"-i", "IP address of the eth0."},
	{"-p", "Connection port number. Default is 6633."},
	{"-f", "Set config file."},
	{"-m, --man-clear", "Don't auto clear the resource when disconnect."},
	{"-l, --log-file", "Create log file:"},
	{"              ", "/usr/local/var/log/cc_controller.log."},
	{"-h, --help", "Print help message."},
	{"-v, --version", "Print the version number of c_controller_for_pof."},
};



static void 
cc_disp_help(void)
{
	uint32_t i;

	CC_PRINT("Usage: cc_main [options] [target] ...");
	CC_PRINT("Options:");

	for(i=0; i<COMMAND_NUM; i++){
		CC_PRINT("  %-30s%-50s", cmds[i].cmd_str, cmds[i].help_str);
	}

	CC_PRINT("\nReport bugs to <wqlxx@aliyun.com>");

	return;
}

static int
cc_check_root(void)
{
	/* Root id = 0 */
	if(geteuid() == 0){
		return CC_NO_ERROR;
	}else{
		return CC_ERROR;
	}    
}

/* Set the Controller's IP address. */
static int
cc_set_controller_ip(char *ip_str)
{
	strcpy(cc_controller_ip_addr, ip_str);
	return CC_NO_ERROR;
}

/* Set the Controller's port. */
int
cc_set_controller_port(uint16_t port)
{
	cc_controller_port = port;
	return CC_NO_ERROR;
}

int
cc_set_init_config(int argc, char **argv)
{
    int ret;

    char opt[] = "i:p:f:mlhv";
    struct option long_options[] = {
                {"man-clear", 0, NULL, 'm'},
                {"log-file", 0, NULL, 'l'},
                {"help", 0, NULL, 'h'},
                {"version", 0, NULL, 'v'},
                {NULL, 0, NULL, 0}
    };

    int ch;

    while((ch = getopt_long(argc, argv, opt, long_options, NULL)) != -1){
        switch(ch){
               case '?':
                    exit(0);
                    break;
               case 'i':
                    cc_set_controller_ip(optarg);
                    cc_cmd_set_controller_ip = CC_CMD_OK;
                    break;
               case 'p':
                    cc_set_controller_port(atoi(optarg));
                    cc_cmd_set_controller_port = CC_CMD_OK;
                    break;
               case 'f':
                    strncpy(cc_cmd_config_file, optarg, CC_STRING_MAX_LEN - 1);
                    cc_cmd_set_config_file = CC_CMD_OK;
                    break;
               case 'm':
                    cc_cmd_auto_clear = CC_CMD_OK;
                    break;
               case 'r':
                    cc_cmd_auto_reconnect = CC_CMD_OK;
                    break;
               case 'l':
                    if(CC_NO_ERROR != cc_check_root()){
                        CC_DEBUG_PRINT_F("permission deny for open log file!");
                        CC_PRINT("permission deny for open log file!");
                        exit(0);
                    }
                    cc_set_log_file(NULL);
                    break;
               case 'h':
                    cc_disp_help();
                    exit(0);
                    break;
               case 'v':
                    CC_PRINT("Version: %s", CC_VERSION);
                    exit(0);
               default:
                    break;
        }
    }
    CC_DEBUG_PRINT_F("Init config finished");

    return CC_NO_ERROR;
}


/*
setnonblocking
*/
int setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

/*
handle_message used as a test.
*/
int 
handle_message(int new_fd)
{
    char buf[MAXBUF + 1];
    int len;
    /* 开始处理每个新连接上的数据收发 */
    bzero(buf, MAXBUF + 1);
    /* 接收客户端的消息 */
    len = recv(new_fd, buf, MAXBUF, 0);
    if (len > 0)
        printf
            ("%d接收消息成功:'%s'，共%d个字节的数据\n",
             new_fd, buf, len);
    else {
        if (len < 0)
            printf
                ("消息接收失败！错误代码是%d，错误信息是'%s'\n",
                 errno, strerror(errno));
        close(new_fd);
        return -1;
    }
    /* 处理每个新连接上的数据收发结束 */
    return len;
}

int
cc_finish_init(void)
{
    int listener, new_fd, kdpfd, nfds, n, ret, curfds;
    socklen_t len;
    struct sockaddr_in my_addr, their_addr;
    unsigned int myport, lisnum;
    struct epoll_event ev;
    struct epoll_event events[MAXEPOLLSIZE];
    struct rlimit rt;
/*
    if (argv[1])
        myport = atoi(argv[1]);
    else
        myport = 7838;

    if (argv[2])
        lisnum = atoi(argv[2]);
    else
        lisnum = 2;
*/
    myport = cc_controller_port;
    lisnum = cc_switch_max_num;

    
    rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE;
    if (setrlimit(RLIMIT_NOFILE, &rt) == -1) {
        CC_ERROR_PRINT_HEAD;
        CC_PRINT("setrlimit");
        return CC_ERROR;
    }
    else
        CC_OUTPUT_LOG_FILE("set rlimit success\n");

    if ((listener = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        CC_ERROR_PRINT_HEAD;
        CC_PRINT("socket");
        return CC_ERROR;
    } else
        CC_PRINT("socket create success\n");

    setnonblocking(listener);

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(myport);
    my_addr.sin_addr.s_addr = cc_controller_port;

    if (bind(listener, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == CC_ERROR) {
        CC_ERROR_PRINT_HEAD;
        CC_PRINT("bind");
        return CC_ERROR;
    } else
        CC_PRINT("socket bind success\n");

    if (listen(listener, lisnum) == -1) {
        CC_ERROR_PRINT_HEAD;
        CC_PRINT("listen");
        return CC_ERROR;

    } else
        CC_PRINT("listen success\n");

    kdpfd = epoll_create(MAXEPOLLSIZE);
    len = sizeof(struct sockaddr_in);
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listener;
    if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, listener, &ev) < 0) {
        CC_ERROR_PRINT_HEAD;
        CC_DEBUG_PRINT_F("epoll set insertion error: fd=%d\n", listener);
        return CC_ERROR;

    } else
        CC_PRINT("socket add epoll success\n");
    curfds = 1;
    while (1) {
        nfds = epoll_wait(kdpfd, events, curfds, -1);
        if (nfds == -1) {
            CC_ERROR_PRINT_HEAD;
            CC_PRINT("epoll_wait");
            break;
        }
        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listener) {
                new_fd = accept(listener, (struct sockaddr *) &their_addr,
                                &len);
                if (new_fd < 0) {
                    CC_DEBUG_PRINT_F("accept");
                    continue;
                } else
                    CC_PRINT("there is a connect from %d:%d，alloced socket: %d\n",\
                        inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);

                setnonblocking(new_fd);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = new_fd;
                if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, new_fd, &ev) < 0) {
                    CC_ERROR_PRINT_HEAD;
                    CC_PRINT("add socket '%d' to epoll failed %s\n",
                            new_fd, strerror(errno));
                    return CC_ERROR;
                }
                curfds++;
            } else {
                ret = handle_message(events[n].data.fd);
                if (ret < 1 && errno != 11) {
                    epoll_ctl(kdpfd, EPOLL_CTL_DEL, events[n].data.fd,
                              &ev);
                    curfds--;
                }
            }
        }
    }
    close(listener);
    return 0;
}

