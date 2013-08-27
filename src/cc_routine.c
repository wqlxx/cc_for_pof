/*FSM*/
#define "cc_common.h"

static int
cc_sc_main_routine()
{
    while(1)
    {
        switch(/*state*/){
            case CC_SERVICE_CONNECTED:
                /*waiting for hello msg, this msg always send from switch*/

            case CC_HELLO:
                /*send feature request msg to switch*/

            case CC_REQUEST_FEATURE:
                /*wait for freature reply*/
                
            case CC_SET_CONFIG:
                /*if got feature reply and send set config*/

            case CC_REQUEST_GET_CONFIG:

            case CC_SECURE_CHANNEL_OK:
                /*send echo request , echo reply, packet in and other openflow msg please check openflow-specific-1.3*/
            default:
                CC_DEBUG_PRINT_F("wrong cc state!");
                state = CC_SERVICE_INVALID;
                cc_reconnect();
        }

        
}


