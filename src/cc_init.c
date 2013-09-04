#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "cc_common.h"

#define CC_VERSION "0x01"
#define COMMAND_STR_LEN 30
#define HELP_STR_LEN    50
#define CC_STRING_MAX_LEN 100
#define COMMAND_NUM 8

char cc_cmd_input_config_file[CC_STRING_MAX_LEN] = "\0";

struct cc_command_line_s{
	char cmd_str[COMMAND_STR_LEN];
	char help_str[HELP_STR_LEN];
} pof_start_commands;
typedef struct cc_command_line_s cc_command_line_t;

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
cc_disp_help(void){
	uint32_t i;

	CC_PRINT("Usage: cc_main [options] [target] ...");
	CC_PRINT("Options:");

	for(i=0; i<COMMAND_NUM; i++){
		CC_PRINT("  %-30s%-50s", cmds[i].cmd_str, cmds[i].help_str);
	}

	CC_PRINT("\nReport bugs to <wqlxx@aliyun.com>");

	return;
}

static void
cc_check_root(void){
	/* Root id = 0 */
	if(geteuid() == 0){
		return CC_NO_ERROR;
	}else{
		return CC_ERROR;
	}    
}

/* Set the Controller's IP address. */
int
cc_set_controller_ip(char *ip_str){
	strcpy(cc_controller_ip_addr, ip_str);
	return CC_NO_ERROR;
}

/* Set the Controller's port. */
int
cc_set_controller_port(uint16_t port){
	cc_controller_port = port;
	return CC_NO_ERROR;
}

int
cc_set_init_config(argc, argv)
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

    while((ch=getopt_long(argc, argv, opt, long_options, NULL)) != -1){
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
    CC_DEBUG_PRINT_F("Init finished");
    return CC_NO_ERROR;
}



