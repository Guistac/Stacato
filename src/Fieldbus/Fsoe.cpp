#include "EtherCatDevice.h"

#include "fsoeapp.h"

uint16_t fsoeapp_generate_session_id(void * app_ref){
    //(void)app_ref;
    return (uint16_t)(rand() % 0xffff);
}

//FSoE stack send data to black channel
void fsoeapp_send(void * app_ref, const void * buffer, size_t size){
    /* We pass a local FSOE app ref as app_ref , set at FSoE create */
    EtherCatDevice* device = (EtherCatDevice*)app_ref;
    memcpy(device->identity->outputs + device->fsoe_offset_outputs, buffer, size);
}

//FSoE stack send data to black channel
size_t fsoeapp_recv(void * app_ref, void * buffer, size_t size){
    /* We pass a local FSOE app ref as app_ref , set at FSoE create */
    EtherCatDevice* device = (EtherCatDevice*)app_ref;
    memcpy(buffer, device->identity->inputs + device->fsoe_offset_inputs, size);
    return size;
}

//FSoE stack user API error callback
void fsoeapp_handle_user_error(void * app_ref, fsoeapp_usererror_t user_error){
    EtherCatDevice* device = (EtherCatDevice*)app_ref;
    const char* msg = fsoeapp_user_error_description(user_error);
    Logger::critical("FSOE ERROR | Device {} | {}", device->getName(), msg);
}