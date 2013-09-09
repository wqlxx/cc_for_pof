#ifndef _CC_COMMON_H_
#define _CC_COMMON_H_

#include <signal.h>


#define CC_MESSAGE_SIZE 2560
#define CC_INITIAL_XID 0
#define CC_MAX_PORT 52

enum cc_cmd_status_e {
    CC_CMD_NULL     = -1,
    CC_CMD_OK       = 0
};
typedef enum cc_cmd_status_e cc_cmd_status_t;


enum cc_state_e {
    CC_SERVICE_INVALID          = 0,
    CC_SERVICE_CONNECTING       = 1,
    CC_SERVICE_CONNECTED        = 2,
    CC_HELLO                    = 3,
    CC_REQUEST_FEATURE          = 4,
    CC_SET_CONFIG               = 5,
    CC_REQUEST_GET_CONFIG       = 6,
    CC_SECURE_CHANNEL_OK        = 7,
    CC_STATE_MAX                = 8        
};
typedef enum cc_state_e cc_state_t;


typedef void (*POF_TIMER_FUNC)(uint32_t timerid, int arg);


struct port_info_s{
	struct ofp_phy_port port;
	int valid;
};
typedef struct port_info_s port_info_t;


struct sw_details_s{
	uint8_t version;
	uint8_t n_tables;
    pid_t pid;
	uint32_t actions;
	uint32_t n_buffers;
	uint32_t capabilities;
	uint64_t dpid;//datapath_id come from switch_feature_request/reply
	port_info_t sw_port[CC_MAX_PORT];
};
typedef struct sw_details_s sw_details_t;


struct sw_info_s {
    int fd;
    sw_details_t sw_details; 
    struct itimerval val;
    POF_TIMER_FUNC timer_handler; /*use 'extern' to link form 'echo_handler'*/  
	CThread_pool* cc_thread_pool;
};
typedef struct sw_info_s sw_info_t;

struct server_info_s {
    sw_info_t sw_info;
    int fd;
	struct sockaddr_in addr;
};
typedef struct server_info_s server_info_t;

#endif
