/********************************************************************
 * File: trace.c
 *
 * main function for the pvtrace utility.
 *
 * Author: M. Tim Jones <mtj@mtjones.com>
 *
 */
#include<stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "symbols.h"
#include "stack.h"
#include <libconfig.h>
#include "intlist.h"




char *config_file_name = "/config.txt";
const char *redis_key;
int read_redis = 0;
int trace_level = 0;

//ignore_t **ignore_list;
//int ignore_list_count;

config_t cfg;
/*Returns all parameters in this structure */
config_setting_t *setting;

redisContext *redis_context;
redisReply *reply;

bool isvalueinarray(int val, int *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return true;
    }
    return false;
}

void getConfig() {
    /*Initialization */
    config_init(&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file(&cfg, config_file_name)) {
//        printf("\n%s:%d - %s", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return ;
    }

    /* 获取redis 存储key名称. */
    if (config_lookup_string(&cfg, REDIS_KEY, &redis_key))
        printf("\nredis_key: %s\n", redis_key);
    else
        printf("\nNo %s setting in configuration file.", REDIS_KEY);

    /* 获取redis读取标志位. */
    if (config_lookup_bool(&cfg, REDIS_FLAG, &read_redis) != 0);


    /* 获取跟踪层级 */
    if (config_lookup_int(&cfg, TRACE_LEVEL, &trace_level))
        printf("\trace_level: %i\n", trace_level);
    else
        printf("\nNo %s setting in configuration file.", TRACE_LEVEL);

    /* 获取忽略的方法名列表 */
//    config_setting_t* ignore_list_t = config_lookup(&cfg, IGNORE_LIST);
//
//    ignore_list_count = config_setting_length(ignore_list_t);
//    ignore_list = malloc(ignore_list_count * sizeof(ignore_t *));
//
//    int i;
//    for (i = 0; i < ignore_list_count; i++)
//    {
//        ignore_list[i] = malloc(sizeof(ignore_t));
//        const char *elem = config_setting_get_string_elem(ignore_list_t, i);
//    }

}

void run(char *filename) {
    FILE *tracef;
    FILE *select_trc;
    char type;
    unsigned int address;
    unsigned int callsite;

    int pid_tid;
//    int unused_addr;
    FILE *fp;

    getConfig();

    //redis connect init
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    redis_context = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
    if (redis_context == NULL || redis_context->err) {
        if (redis_context) {
            printf("Connection error: %s\n", redis_context->errstr);
            redisFree(redis_context);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }
    if (read_redis) {
        fp = fopen("pvtrace_redis.txt", "w");
        if (fp == NULL) exit(-1);

        do {
            //这里去做写文件的操作
            reply = redisCommand(redis_context, "RPOP %s", redis_key);

            if (reply->type == REDIS_REPLY_STRING) {
                fprintf(fp, "%s\n", reply->str);
            }
            if(reply == NULL ||
               reply->type == REDIS_REPLY_NIL ||
               reply->type == REDIS_REPLY_ERROR) {
                freeReplyObject(reply);
                break;
            }
            freeReplyObject(reply);

        }while (true);
        fclose(fp);
        tracef = fopen("pvtrace_redis.txt", "r");
    } else {
        tracef = fopen(filename, "r");
    }



    if (tracef == NULL) {
        printf("Can't open %s\n", filename);
        exit(-1);
    }

    int *pid_tid_list;

    int count = 0;
//    while (!feof(tracef)) {
//        fscanf(tracef, "%c%x,%x,%d\n", &type, &address,&callsite ,&pid_tid);
//        /* 初始化 */
//        if (count == 0) {
//            begin(&pid_tid_list, pid_tid);
////            pid_tid_list = (int *)malloc(sizeof(int));
////            pid_tid_list[0] = pid_tid;
//            count++;
//        }
//
//        if(!isvalueinarray(pid_tid, pid_tid_list, count)) {
//            count++;
//            /* 动态的新增一个数组元素空间 */
//
//            add(&pid_tid_list, pid_tid);
////            pid_tid_list = realloc(pid_tid_list, count);
////            pid_tid_list[count-1] = pid_tid;
//        }
//
//    }
    rewind(tracef);


    int i;
//    for (i=0; i<count; i++) {
//        select_trc = fopen("SELECT.trc", "w");
        stackInit();
        int level = 0;
        while (!feof(tracef)) {
            char str[1024];
            int unknown_function_num;

            fscanf(tracef, "%c,%x,%d\n", &type, &address, &pid_tid);
            //每次挑一个tid,然后再做全文件扫描
//            if (pid_tid_list[i] == pid_tid) {
//                if(trace_level != 0 && level > trace_level) break;
                if (type == '>') {
                    level++;

                    addSymbol(address);

                    addCallTrace(address);

                    stackPush(address);

                } else if (type == '<') {
                    /* Function Exit */
                    level--;
                    (void) stackPop();

                }
//                //todo 这些内容再写到一个文件里
//                if(select_trc != NULL)
//                    fprintf(select_trc, "%c%x,%x,%d\n", type, address,callsite, pid_tid);
//            }
        }
//        fclose(select_trc);
        //todo 对文件名执行那段shell
        char shell[1024];
//        snprintf(shell,1024, "awk -F, '{print $1}' SELECT.trc | sed \"s/[<>]//\" | sort | uniq -c | sort -n | awk '{ if($1<=6) print $2}' >> FUNCS_ADDR");
//        printf("test: %s\n",shell);
//        system(shell);
//        snprintf(shell,1024, "for addr in `cat FUNCS_ADDR`; do echo \"$addr `addr2line -e $INFORMIXDIR/bin/oninit -f $addr | xargs`\" | sed \"s/\\/vobs\\/tristarm//\" >> FUNCS_INFO; done;\n");
//        printf("test: %s\n",shell);
//        system(shell);

        snprintf(shell,1024, "groovy /home/linchanghui/Downloads/pvtrace/pvtrace/pvtrace_script/src/generate_function_table.groovy");
        system(shell);

//        int ret = remove("SELECT.trc");
//        ret = remove("FUNCS_ADDR");
//        ret = remove("FUNCS_INFO");



        //todo 根据ANA_FUNCS生成puml的格式，这里要去把groovy的那个翻译成一个方法
        /*这些要初始化*/
//        emitSymbols(pid_tid_list[i]);
//        initSymbol(NULL);
//        rewind(tracef);
//    }



//    if (read_redis) {
//        //这里去做删文件的操作
//
//    }
    if(redis_context != NULL) redisFree(redis_context);

    fclose(tracef);
    config_destroy(&cfg);
//    free(pid_tid_list);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: pvtrace <image>\n\n");
        exit(-1);
    }
    initSymbol(argv[1]);

    if (argc > 2) {
        for (int i = 2; i < argc; ++i) {
            run(argv[i]);
        }
    } else if (argc == 2) {
        run("trace.txt");
    }



    return 0;
}

