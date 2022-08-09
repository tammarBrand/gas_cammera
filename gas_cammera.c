#include "gas_cammera.h"

void* libHandle;
stage_t * stage_api;


void  GAS_API_init(handler* my_handler){
    init_rgb_matrix();

    //connect to stage library
    libHandle= dlopen("/home/tammar/sambashare/build-stage_lib-Desktop-Debug/libstage_lib.so.1.0",RTLD_LAZY);
    assert(libHandle);
    stage_api = (stage_t*)dlsym(libHandle,"stage_api");

    //creates required queues
    Queue* q1=createQueue(10);
    Queue* q2=createQueue(10);
    Queue* q3=createQueue(10);
    Queue* q4=createQueue(10);

    rgb_converter_params * rgb_converter_p=(rgb_converter_params *)malloc(sizeof(rgb_converter_params));
    if(rgb_converter_p==NULL){
        printf("error mallocing\n");
        exit(0);
    }
    rgb_converter_p->record=my_handler->record;
    rgb_converter_p->snap=my_handler->snap;

    encode_params * encode_p=(encode_params*)malloc(sizeof(encode_params));
    if(encode_p==NULL){
        printf("error mallocing\n");
        exit(0);
    }
    encode_p->record=my_handler->record;
    encode_p->tools=my_handler->tools;

    write_record_params* write_record_p=(write_record_params*)malloc(sizeof(write_record_params));
    if(write_record_p==NULL){
        printf("error mallocing\n");
        exit(0);
    }
    write_record_p->f=my_handler->tools->f;

    //create required stages
    stage* st5=stage_api->create_stage(q4,NULL,write_record,(void*)(write_record_p),NULL);
    stage* st4=stage_api->create_stage(q3,q4,encode,(void*)(encode_p),st5);
    stage* st3=stage_api->create_stage(q2,q3,yuv_converter,(void*)(my_handler->record),st4);
    stage* st2=stage_api->create_stage(q1,q2,rgb_converter,(void*)rgb_converter_p,st3);
    stage* st1=stage_api->create_stage(NULL,q1,capture,(void*)(my_handler->record),st2);

    my_handler->stage_head=st1;
    my_handler->stages_number=5;
    my_handler->record->is_record_on=0;
    my_handler->snap->is_snap_on=0;
    *(my_handler->status_bits)=0;
    (*(my_handler->status_bits))|=INIT;

    my_handler->stage_handle=stage_api ->create_handler(st1);
}

void* GAS_API_create_gas_handler(){
    handler* my_handler=(handler*)malloc(sizeof(handler));
    if(my_handler==NULL){
        printf("error mallocing\n");
        exit(0);
    }
    my_handler->status_bits=(int*)malloc(sizeof(int));
    if(my_handler->status_bits==NULL)
    {
        printf("error mallocing\n");
        exit(0);
    }

    my_handler->record=(record_t*)malloc(sizeof (record_t));
    if(my_handler->record==NULL)
    {
        printf("error mallocing\n");
        exit(0);
    }
    my_handler->snap=(snapshot_t*)malloc(sizeof(snapshot_t));

    if(my_handler->snap==NULL)
    {
        printf("error mallocing\n");
        exit(0);
    }
    my_handler->tools=(record_tools*)malloc(sizeof(record_tools));
    if(my_handler->tools==NULL)
    {
        printf("error mallocing\n");
        exit(0);
    }
    GAS_API_init(my_handler);

    return (void*)my_handler;
}

void free_stages_and_queues_api(void* my_stage){
    stage *prev, *head=(stage*)my_stage;
    prev=head;
    Queue* q;

    while(prev!=NULL){
        head=prev->next;
        q=prev->destQu;
        freeQueue(q);
        stage_api->free_stage(prev);
        prev=head;
    }

}

void GAS_API_free_handler(void* handle){
    handler* my_handler=(handler*)(handle);

    //free all stages parametrs
    if(my_handler->status_bits==NULL){
        printf("----double free----\n");
    }
    else{
        free(my_handler->status_bits);
    }
    if(my_handler->record==NULL){
        printf("----double free----\n");
    }
    else{
        free(my_handler->record);
    }
    if(my_handler->snap==NULL){
        printf("----double free----\n");
    }
    else{
        free(my_handler->snap);
    }
    if(my_handler->tools==NULL){
        printf("----double free----\n");
    }
    else{
        free(my_handler->tools);
    }

    if(my_handler==NULL)
        return;
    (*(my_handler->status_bits))&=~INIT;
    free_stages_and_queues_api(my_handler->stage_head);
    free(my_handler);
}

void init_record_t(record_t* record){

    //open json file, init, ect
    record->is_record_on=1;
    record->height=24;
    record->width=36;
    record->file_name=strdup("my_rec.ts");

}

void init_tools(record_tools* tools, record_t *record){

    int ret;

    tools->codec=avcodec_find_encoder(AV_CODEC_ID_MPEG2VIDEO);
    if (!tools->codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    tools->c = avcodec_alloc_context3(tools->codec);
    if (!tools->c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    tools->pkt = av_packet_alloc();
    if (!tools->pkt)
        exit(1);

    // put sample parameters
    tools->c->bit_rate = 400000;
    // resolution must be a multiple of two
    tools->c->width = record->width;
    tools->c->height = record->height;
    //frames per second
    tools->c->time_base = (AVRational){1, 25};
    tools->c->framerate = (AVRational){25, 1};
    tools->c->gop_size = 5;
    tools->c->max_b_frames = 1;
    // c->pix_fmt = AV_PIX_FMT_YUV420P;
    tools->c->pix_fmt=AV_PIX_FMT_YUV420P;
    ret = avcodec_open2(tools->c, tools->codec, NULL);
    if (ret < 0) {
        //fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
        exit(1);
    }
    tools->f = fopen(record->file_name, "wb");
    if (!tools->f) {
        fprintf(stderr, "Could not open %s\n",record->file_name);
        exit(1);
    }
    tools->frame = av_frame_alloc();
    if (!tools->frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    tools->frame->format = tools->c->pix_fmt;
    tools->frame->width  = tools->c->width;
    tools->frame->height = tools->c->height;

    ret = av_frame_get_buffer(tools->frame, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }
    ret = av_frame_make_writable(tools->frame);
    if (ret < 0)
        exit(1);

    tools->start_time=0;
    return;
}

int GAS_API_start_record(void* handle){

    printf ("====start_record!====\n");

    handler* my_handler=(handler*)(handle);
    (*(my_handler->status_bits))|=RECORD_START;

    init_record_t(my_handler->record);
    init_tools(my_handler->tools, my_handler->record);

    int stages_number=5;
    int is_pipe=1;
    stage_api->start_pipe(my_handler->stage_handle, stages_number,is_pipe);

    return 1;

}

void close_record_tool(record_tools* tools){

    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    fwrite(endcode, 1, sizeof(endcode),tools->f);
    fclose(tools->f);
    avcodec_free_context(&(tools->c));
    av_frame_free(&(tools->frame));
    av_packet_free(&(tools->pkt));
}

int GAS_API_stop_record(void* handle){

    printf ("====stop_record====\n\r");

    handler* my_handler=(handler*)(handle);
    my_handler->record->is_record_on=0;
    (*(my_handler->status_bits))|=RECORD_STOP;
    stage_api->stop_pipe(my_handler->stage_handle);

    close_record_tool(my_handler->tools);

    (*(my_handler->status_bits))&=~RECORD_START;
    (*(my_handler->status_bits))&=~RECORD_STOP;

    return 0;
}

int GAS_API_do_snapshot(void* handle){
    printf("GAZ_API_do_snapshot\n");
    handler* my_handler=(handler*)(handle);
    (*(my_handler->status_bits))|=SNAPSHOT;

    my_handler->snap->height=36;
    my_handler->snap->width=24;
    my_handler->snap->type=strdup("jpg");
    my_handler->snap->file_name="image.jpg";
    time_t t=time(NULL);
    struct tm tm=*localtime(&t);
    int n=25;//sprintf(NULL,"%d-%02d-%02d %02d:%02d:%02d\0", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    char* name=(char*)malloc(n+5);
    sprintf(name,"%d-%02d-%02d %02d:%02d:%02d.", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    /*
     * takes out data from json file and initilize the snapshot_t object
     */

    my_handler->snap->file_name=name;
    my_handler->snap->type=strdup("jpg");
    strcat(my_handler->snap->file_name,my_handler->snap->type);
    my_handler->snap->is_snap_on=1;
    if(((*(my_handler->status_bits))&RECORD_START )&& !((*(my_handler->status_bits))&RECORD_STOP))
    {
        (*(my_handler->status_bits))&=~SNAPSHOT;
        return 1;
    }
    int stages_number=2;
    int is_pipe=0;

    stage_api->start_pipe(my_handler->stage_handle,stages_number,is_pipe);
    (*(my_handler->status_bits))&=~SNAPSHOT;
    return 1;
}
int GAS_API_start_streaming(streaming_t* stream,char * file_name){
    printf("GAZ_API_start_streaming\n");
    return 1;
}
int GAS_API_stop_streamig(streaming_t*stream){
    printf("GAZ_API_stop_streamig");
    return 1;
}
char* GAS_API_get_dll_version(){
    printf("GAZ_API_get_dll_version");
    return NULL;
}
char* GAS_API_get_video_statics(record_t* r){
    printf("GAZ_API_get_video_statics");
    return NULL;
}
int* GAS_API_get_status(void* handle){
    printf("GAZ_API_get_status");
    handler* my_handler=(handler*)(handle);
    return my_handler->status_bits;
}

gazapi_t p_gaz_api= {
    .create_gas_handler=GAS_API_create_gas_handler,
    .free_handler=GAS_API_free_handler,
    .start_record=GAS_API_start_record,
    .stop_record=GAS_API_stop_record,
    .start_streaming=GAS_API_start_streaming,
    .stop_streamig=GAS_API_stop_streamig,
    .do_snapshot=GAS_API_do_snapshot,
    .get_dll_version=GAS_API_get_dll_version,
    .get_video_statics=GAS_API_get_video_statics,
    .get_status=GAS_API_get_status
};

