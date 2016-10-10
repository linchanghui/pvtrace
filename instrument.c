/********************************************************************
 * File: instrument.c
 *
 * Instrumentation source -- link this with your application, and
 *  then execute to build trace data file (trace.txt).
 *
 * Author: M. Tim Jones <mtj@mtjones.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>
#include <hiredis.h>
#include <zlog.h>

#define REDIS_KEY "redis_key"
/* Function prototypes with attributes */
void main_constructor(void)
        __attribute__ ((no_instrument_function, constructor));

void main_destructor(void)
        __attribute__ ((no_instrument_function, destructor));

void __cyg_profile_func_enter(void *, void *)
        __attribute__ ((no_instrument_function));

void __cyg_profile_func_exit(void *, void *)
        __attribute__ ((no_instrument_function));

int (*callback)() = NULL;


//static FILE *fp;

char *config_file_name = "/config.txt";
char *redis_key;
config_t cfg;
/*Returns all parameters in this structure */
config_setting_t *setting;

redisContext *redis_context;
redisReply *reply;


zlog_category_t *c;

void register_function(int (*get_trace_id)()){
    callback = get_trace_id;
}


void main_constructor(void) {
//    fp = fopen("trace.txt", "w");
//    if (fp == NULL) exit(-1);

    /*Initialization */
    config_init(&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file(&cfg, config_file_name)) {
        printf("\n%s:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return ;
    }

    /* Get the configuration file name. */
    if (config_lookup_string(&cfg, REDIS_KEY, &redis_key))
        printf("\nFile Type: %s\n", redis_key);
    else
        printf("\nNo %s setting in configuration file.", REDIS_KEY);
    config_destroy(&cfg);

    //redis connect init
//    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
//    redis_context = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
//    if (redis_context == NULL || redis_context->err) {
//        if (redis_context) {
//            printf("Connection error: %s\n", redis_context->errstr);
//            redisFree(redis_context);
//        } else {
//            printf("Connection error: can't allocate redis context\n");
//        }
//        exit(1);
//    }

    zlog_init("/etc/instrument.conf");

    c = zlog_get_category("instrument");

}


void main_deconstructor(void) {
//    fclose(fp);
//    if(redis_context != NULL) redisFree(redis_context);

    zlog_fini();

}



void __cyg_profile_func_enter( void *this, void *callsite )
{

    printf("%x:in\n",(int *)this);
//    if(redis_context != NULL && redis_key != NULL) {
        if(access("/tmp/IFX_TRACE", 0) != 0) return;

        int trace_id=-1;

        if( (void *)callback != (void *)NULL ) trace_id = callback();

        if(NULL != this) {

            char buf[1024];

//            if(trace_id==-1){
//                snprintf(buf,1024, ">%x,%x\n",(int *)this, (int *)callsite);
//            }else{
//                snprintf(buf,1024, ">%x,%x,%d\n",(int *)this, (int *)callsite, trace_id);
//            }
            if(trace_id==-1){
                sprintf(buf, ">,%x\n",(int *)this);
            }else{
                sprintf(buf, ">,%x,%d\n",(int *)this, trace_id);
            }
//            reply = redisCommand(redis_context, "LPUSH ids %s", buf);
            //todo 这部分应该要做个返回的值的判断，不过reply的返回值好像不同的语句不同，我还没弄错清楚，还没做
            //printf("SET: %s\n", reply->str);
//            freeReplyObject(reply);

            zlog_info(c,buf);
        }
//    }
}



void __cyg_profile_func_exit( void *this, void *callsite )
{
    printf("%x:out\n",(int *)this);
//    if(redis_context != NULL && redis_key != NULL) {
        if(access("/tmp/IFX_TRACE", 0) != 0) return;

        int trace_id=-1;

        if( (void *)callback != (void *)NULL ) trace_id = callback();

        if(NULL != this) {

            char buf[1024];

//            if(trace_id==-1){
//                snprintf(buf,1024, "<%x,%x\n",(int *)this, (int *)callsite);
//            }else{
//                snprintf(buf,1024, "<%x,%x,%d\n",(int *)this, (int *)callsite, trace_id);
//            }
            if(trace_id==-1){
                sprintf(buf, "<,%x\n",(int *)this);
            }else{
                sprintf(buf, "<,%x,%d\n",(int *)this, trace_id);
            }
//            reply = redisCommand(redis_context, "llen ids");
//            reply = redisCommand(redis_context, "LPUSH ids %s", buf);
            //todo 这部分应该要做个返回的值的判断，不过reply的返回值好像不同的语句不同，我还没弄错清楚，还没做
            //printf("SET: %s\n", reply ->str);
//            freeReplyObject(reply);

            zlog_info(c,buf);
        }
//    }
}



