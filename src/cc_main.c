//#include "cc_ncx_slab.h"
#include "cc_init.h"


int
main(int argc, char argv)
{
    int ret;

    ret = cc_set_init_config(argc, argv);
    ret = cc_finish_init();

    return 0;
    
}
