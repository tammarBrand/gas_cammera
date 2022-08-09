#include "write_record.h"

void* write_record(void* data, void* arg){

    if(data==NULL)
        return NULL;
  /*  write_record_params* params=(write_record_params*)arg;
    write_record_data* w_data=(write_record_data*)data;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    printf("write\n");

    printf("Write packet %d (size=%5d)\n", w_data->data[0], w_data->size);


    fwrite(w_data->data, 1, w_data->size, params->f);
    fwrite(endcode, 1, sizeof(endcode), params->f);
    free_pkt_data(w_data->data);
    */
    return  data;


}
