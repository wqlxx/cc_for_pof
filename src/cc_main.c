//#include "cc_ncx_slab.h"
#include "cc_init.h"

static server_info_t main_server;


int
main(int argc, char **argv)
{
    int ret;

	signal(SIGPIPE, SIG_IGN);
    ret = cc_set_init_config(argc, argv);
    if(ret == CC_ERROR)
        CC_OUTPUT_LOG_FILE("init config error, code is %d", ret);
    ret = cc_finish_init();
    if(ret == CC_ERROR)
        CC_OUTPUT_LOG_FILE("finish config error, code is %d", ret);
    
    cc_server_conn_create(&main_server);
    cc_conn_accept(&main_server); 

    return 0;
    
}
