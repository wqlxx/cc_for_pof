enum cc_error_pof_e{
    CC_ERROR = -1,
    CC_NO_ERROR,

    CC_INIT_FAILURE,
    CC_NO_LOG_FILE,

    CC_CREATE_SOCKET_FAILURE,
    CC_BIND_SOCKET_FAILURE,
    CC_LISTEN_SOCKET_FAILURE,
    CC_ACCEPT_SOCKET_FAILURE,
    CC_READ_SOCKET_FAILURE,
    CC_WRITE_SOCKET_FAILURE,
    
    CC_MALLOC_FALIURE

    
};
typedef enum cc_error_pof_e cc_error_pof_t;


    
