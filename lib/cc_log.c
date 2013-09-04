#include "cc_log.h"

FILE *log_fp = NULL;

int
cc_set_log_file(char *filename){
	char *fm = NULL;
	if(filename){
		fm = filename;
	}else{
		fm = CC_LOG_FILE;
	}

	log_fp = fopen(fm, "w");
	if(!log_fp){
		CC_PRINT("Open %s failed!", filename);
		exit(0);
	}
	return CC_NO_ERROR;
}

int
cc_empty_log_file(){
    if(!log_fp){
        return CC_NO_LOG_FILE;
    }
    system(CC_EMPTY_LOG_FILE);

    return CC_NO_ERROR;
}

int
cc_delete_log_file(){
	if(!log_fp){
		return CC_NO_LOG_FILE;
	}
	fclose(log_fp);
	return CC_NO_ERROR;
}

