#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/wait.h>

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

int listener;

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
int 
cc_set_nonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}
/*
int 
set_reuseaddr(int sockfd)
{

	int flag = 1;
    int ret;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag));
  	if ( ret < 0 ) {
    	//error( "Failed to set socket options ( fd = %d, ret = %d, errno = %s [%d] ).",fd, ret, strerror( errno ), errno );
    	return CC_ERROR;
 	}
	return CC_NO_ERROR;

}
*/

int
cc_set_nodelay(int sockfd)
{

    int flag = 1;
  	int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof( flag ) );
  	if ( ret < 0 ) {
    	//error( "Failed to set socket options ( fd = %d, ret = %d, errno = %s [%d] ).",fd, ret, strerror( errno ), errno );
    	return CC_ERROR;
 	}
	return CC_NO_ERROR;

}

int 
cc_set_recvbuf(int fd, size_t size)
{
    int ret;
    ret = setsockopt(fd,SOL_SOCKET,SO_RCVBUF,&size,sizeof(size));
    return ret;
}


int 
cc_set_socket_fd(void)
{
	int fd;
	int connect_status;
	fd = socket(AF_INET,SOCK_STREAM,0);
	return fd;
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
cc_process(int fd)
{
    
}


int 
cc_server_conn_create(server_info_t *si)
{
	
	if((si->fd = cc_set_socket_fd()) < 0)
	{
		printf("|ERR|socket create failed\n");
		return 	CC_ERROR;
	}

	memset(si->addr,0,sizeof(struct sockaddr_in));
	si->addr.sin_family = AF_INET;
	si->addr.sin_addr.s_addr = inet_addr("127.0.0.1");//get_local_ip_main();
	si->addr.sin_port = CC_CONTROLLER_PORT;
	
	int flag = 1;
	int ret;
	
	ret = setsockopt(si->fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof( flag ))
	if ( ret < 0 ) 
	{
    	CC_ERROR_PRINT_HEAD;
        CC_PRINT("socket reused failed\n");
    	return CC_ERROR;
  	}

	//ret = setsockopt(cc_socket->fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof( flag ));
	ret = cc_set_nodelay(si->fd);
	if ( ret < 0 ) 
	{
	    CC_ERROR_PRINT_HEAD;
    	CC_PRINT("socket set_nodelay failed\n");
    	return CC_ERROR;
  	}

	ret = cc_set_nonblocking(si->fd);
	if ( ret < 0 ) 
	{
	    CC_ERROR_PRINT_HEAD;
    	CC_PRINT("socket set_nonblocking failed\n");
    	return CC_ERROR;
  	}

	ret = bind(si->fd,(struct sockaddr*)&si->addr,sizeof(struct sockaddr_in));
	if(ret < 0)
	{
	    CC_ERROR_PRINT_HEAD;
    	CC_PRINT("socket bind failed");
		close(si->fd);
    	return CC_ERROR;
  	}

	if(listen(si->fd, CC_LENGTH_OF_LISTEN_QUEUE))
	{
	    CC_ERROR_PRINT_HEAD;
    	CC_PRINT("socket listen failed");
		close(si->fd);
    	return CC_ERROR;
  	}

	return si->fd;
}


int 
cc_conn_accept(server_info_t *si)
{
	struct sockaddr_in switch_addr;
	socklen_t addr_len;
	pid_t pid;
	int accept_fd;
	int ret;
	socklen_t *len;
	list_element_t *new_elem;
	sw_info_t *new_info;
    
	addr_len = sizeof(struct sockaddr_in);
	accept_fd = accept(main_server.fd,(struct sockaddr*)&switch_addr,&addr_len);
	if(accept_fd < 0)
	{
	    CC_ERROR_PRINT_HEAD;
    	CC_PRINT("accept failed");
		close(accept_fd);
		return CC_ERROR;
	}else if( accept_fd > FD_SETSIZE ) {
		close(accept_fd);
	} else {
		cc_set_nonblocking(accept_fd);
		cc_set_recvbuf(accept_fd, CC_MAX_SOCKET_BUFF);
		cc_set_nodelay(accept_fd);

		pid = fork();
		if(pid < 0)
		{
			//TODO: close the listen socket
			//printf("|ERR|fork failed\n");
	        CC_ERROR_PRINT_HEAD;
    	    CC_PRINT("create child process failed!");
			close(accept_fd);
			return CC_ERROR;
		}

		if(pid == 0)
		{
			//debug_wait("/home/ovs/debug");
			
			sw_info_t *sw_info = (sw_info_t*)malloc(sizeof(sw_info_t));;
			cc_init_sw_info(sw_info);

			sw_info->pid = getpid();
			/*here we can add a function to build
		 	*a file to restore the cc_sw_info with 
			 *a special name, such as "sw_$pid.txt".
		 	*then main loop can throught search these files
		 	*to make a list which can be used to build a virtual network manager
	     		*/
			sw_info->fd = accept_fd;

			if( accept_fd < CC_ACCEPT_FD)
			{
				dup2(accept_fd, CC_ACCEPT_FD);//avoid the fd is smaller than 3,0 is for standard input, is for standard output 2 is for standard error
				close(accept_fd);
				accept_fd = CC_ACCEPT_FD;
			}

			struct timeval timeout;
			fd_set writefds;
			fd_set readfds;
			pool_init(sw_info->cc_thread_pool, CC_MAX_THREAD_NUM);
			while(1)
			{
				FD_ZERO(&readfds);
				FD_ZERO(&writefds);
				FD_SET(accept_fd,&readfds);
				FD_SET(accept_fd,&writefds);
				timeout.tv_sec = CC_CONN_TIMEOUT_SEC;
				timeout.tv_usec = CC_CONN_TIMEOUT_USEC;
				ret = select(accept_fd+1, &readfds, &writefds, NULL, &timeout);
				if( ret == -1 )
				{
					if( errno == EINTR )				
						continue;
					else
						return CC_ERROR;
				}else if( ret == 0 ){
					continue;
				}else{
					if(FD_ISSET(accept_fd,&readfds))
						//cc_of_handler_recv_event(cc_sw_info);
						//cc_recv_from_secure_channel(cc_sw_info);
					if(FD_ISSET(accept_fd,&writefds))
						//cc_of_handler_send_event(cc_sw_info);
						//cc_flush_to_secure_channel(cc_sw_info);
				}
			}
			cc_finalize_sw_info(sw_info);
			return CC_NO_ERROR;
			/*may be we should throw a signal to parent to delete the
			*the record of this switch 
			*/
		}else{
			return CC_NO_ERROR;
		}
	}
}

int
cc_polling(cc_socket* cc_socket)
{
	int ret;
	fd_set listen_fdset;
	int max_fd = cc_socket->fd + 1;

	FD_ZERO(&listen_fdset);
	FD_SET(cc_socket->fd, &listen_fdset);
	while(1)
	{
		
		FD_ZERO(&listen_fdset);
		FD_SET(cc_socket->fd,&listen_fdset);
		ret = select(max_fd,&listen_fdset,NULL,NULL,0);
		if( ret == -1 )
		{
			if( errno == EINTR )				
				continue;
			else
				return CC_ERROR;
		}else if( ret == 0 ){
			continue;
		}else{
			if(FD_ISSET(cc_socket->fd, &listen_fdset))
			{
				sw_info *cc_sw_info;
				ret = cc_conn_accept(cc_socket , cc_sw_info);
				if( ret < 0 ){
					log_err_for_cc("accept failed!");
					return CC_ERROR;
				}
				//ret = cc_insert_sw_info(sw_info_table, cc_sw_info);
				if( ret < 0 ){
					return CC_ERROR;
				}
			}
		}
	}
	return CC_SUCCESS;
}


/*use double fork to create the child process*/
#if 0
int
cc_finish_init(void)
{
    int new_fd, kdpfd, nfds, n, ret, curfds;
    pid_t pid, pid_s; /*20130905 add*/
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

    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        CC_ERROR_PRINT_HEAD;
        CC_PRINT("socket");
        return CC_ERROR;
    } else
        CC_PRINT("socket create success\n");

    set_nonblocking(listener);
    set_nodelay(listener);
    set_reuseaddr(listener);
    
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myport);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listener, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == CC_ERROR) {
        CC_ERROR_PRINT_HEAD;
        CC_PRINT("bind");
        perror("bind");
        return CC_ERROR;
    } else
        CC_PRINT("socket bind success\n");

    if (listen(listener, lisnum) == -1) {
        CC_ERROR_PRINT_HEAD;
        CC_PRINT("listen");
        return CC_ERROR;

    } else {
        CC_PRINT("listen success\n");
    }

    
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
            CC_PRINT("epoll_wait failed!");
            break;
        }
        CC_PRINT("nfds is %d\n", nfds);
        //for (n = 0; n < nfds; ++n) {
            if (events[0].data.fd == listener) {
                new_fd = accept(listener, (struct sockaddr *) &their_addr,
                                &len);
                CC_PRINT("new fd is %d", new_fd);
                if (new_fd < 0) {
                    CC_DEBUG_PRINT_F("accept");
                    continue;
                } else {
                    CC_PRINT("there is a connect from %s:%d, alloced socket: %d\n",\
                        inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);
                }

                /*here use double fork to create child process*/
                if(pid = fork() < 0){
                    CC_ERROR_PRINT_HEAD;
                    CC_PRINT("first fork error!");
                    return CC_ERROR;
                }else if(pid == 0){
                    CC_PRINT("first child pid\n");
                    if((pid_s = fork())<0){
                        CC_ERROR_PRINT_HEAD;
                        CC_PRINT("second fork error!");
                        return CC_ERROR;                        
                    }else if(pid_s > 0){
                        CC_PRINT("first child exit pid is %d", getpid());
                        close(new_fd);
                        exit(0);
                    }
                    CC_PRINT("second child pid\n");

                    handle_message(new_fd);
                    CC_PRINT("pid is %d exit", getpid());
                    exit(0);
                }
            }
        //}
    }
    CC_PRINT("cc_finish exiting\n");
    close(listener);
    return 0;
}
#endif
