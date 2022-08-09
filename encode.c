#include "encode.h"

static int start_time=0;

void* encode_it(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE* f);
uint64_t GetMHClock(void);
void* encode(void* data, void* arg){
    if(data==NULL)
        return NULL;

    printf("encode\n");
    YUV* yuv=(YUV*)data;
    encode_params* params=(encode_params*) arg;
    int ret;

    params->tools->frame = av_frame_alloc();
    if (!params->tools->frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    params->tools->frame->format = params->tools->c->pix_fmt;
    params->tools->frame->width  = params->tools->c->width;
    params->tools->frame->height = params->tools->c->height;


    ret = av_frame_get_buffer(params->tools->frame, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }

    params->tools->frame->data[0]=(uint8_t*)yuv->y;
    params->tools->frame->data[1]=(uint8_t*)yuv->u;
    params->tools->frame->data[2]=(uint8_t*)yuv->v;

    if (params->tools->start_time==0){
        params->tools->frame->pts=0;
        start_time=GetMHClock();
    }
    else
        params->tools->frame->pts = GetMHClock()-params->tools->start_time;


    encode_it(params->tools->c, params->tools->frame, params->tools->pkt, params->tools->f);
 /*     write_record_data* write_data=(write_record_data*)malloc(sizeof(write_record_data));
    if(write_data==NULL)
        exit(0);
    write_data->data=(uint8_t *)malloc(params->tools->pkt->size*sizeof(uint8_t));
    if(write_data->data==NULL)
        exit(0);
  memccpy(write_data->data,params->tools->pkt->data,params->tools->pkt->size,sizeof(uint8_t));
    write_data->size=params->tools->pkt->size;
    av_packet_unref(params->tools->pkt);*/
    free_yuv(yuv);

    return data;


}

void* encode_it(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE* f)
{

    int ret;
    ret = avcodec_send_frame(enc_ctx, frame);

    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }
    while (ret >= 0){
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return NULL;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        fwrite(pkt->data, 1,pkt->size, f);
    }
}
uint64_t GetMHClock(void){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (uint64_t)tv.tv_sec * 1000000L + (uint64_t)tv.tv_usec;
}