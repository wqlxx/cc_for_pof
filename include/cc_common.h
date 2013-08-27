enum cc_cmd_status_e {
    CC_CMD_NULL = -1,
    CC_CMD_OK = 0
};
typedef enum cc_cmd_status_e cc_cmd_status_t;

#define CC_MESSAGE_SIZE 2560

#define CC_INITIAL_XID 0

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

