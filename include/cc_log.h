#ifndef __CC_LOG_H__
#define __CC_LOG_H__ 1


#include <stdio.h>
#include <stdlib.h>

#include "cc_error.h"
#include "cc_log.h"
#include "cc_common.h"

#define CC_LOG_FILE "/usr/local/var/log/c_contoller_for_pof.log";
#define CC_EMPTY_LOG_FILE "cat /dev/null > /usr/local/var/log/c_contoller_for_pof.log"

extern FILE *log_fp;

#define CC_PRINT(fmt, ...)                          \
    do{                                             \
        printf(fmt"\n", ##__VA_ARGS__);             \
    }while(0);

#define CC_OUTPUT_LOG_FILE(fmt, ...)                \
    do{                                             \
        if(log_fp){                                 \
            fprintf(log_fp, fmt, ##__VA_ARGS__);    \
        }                                           \
    }while(0)


#define CC_ERROR_PRINT_HEAD                                                            \
            CC_PRINT("%s|%s|ERROR|%s|%d: ", __TIME__, __DATE__, __FILE__, __LINE__)     

#ifdef CC_DEBUG_PRINT_ON
#define CC_DEBUG_PRINT_F(fmt,...)	                                                    \
    do{                                                                                 \                                                       \
            CC_OUTPUT_LOG_FILE("%s(%d): "fmt"\n", __FILE__ ,__LINE__, ##__VA_ARGS__);    \
    }while(0);
        
#else 
#define CC_DEBUG_PRINT_F(fmt,...)
#endif //CC_DEBUG_PRINT_ON

#endif //__CC_LOG_H__
