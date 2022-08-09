#ifndef GAS_CAMMERA_H
#define GAS_CAMMERA_H
#include <dlfcn.h>
#include <assert.h>
#include "functions_tools.h"
#include "stage_lib.h"

#include "capture.h"
#include "rgb_converter.h"
#include "yuv_converter.h"
#include "encode.h"
#include "write_record.h"


enum status_bits{
    INIT=1,
    RECORD_START=2,
    RECORD_STOP=4,
    SNAPSHOT=8,
    CAPTURE=16,
    RGB_CONVERTER=32,
    YUV_CONVERTER=64,
    ENCODE=128,
    WRITE_RECORD=256
};

typedef struct handler{

    int* status_bits;
    int stages_number;
    stage* stage_head;
    stage_handler* stage_handle;
    snapshot_t* snap;
    record_t * record;
    record_tools *tools;

}handler;


typedef struct {
    void* (*create_gas_handler)();
    void (*free_handler)(void* handle);
    int (*start_record)(void* handler);
    int (*stop_record)(void* handler);
    int (*start_streaming)(streaming_t*,char * file_name);
    int (*stop_streamig)(streaming_t*);
    int (*do_snapshot)(void*);
    char* (*get_dll_version)();
    char* (*get_video_statics)(record_t*);
    int* (*get_status)(void*handle);
}gazapi_t;


#endif // GAS_CAMMERA_H
