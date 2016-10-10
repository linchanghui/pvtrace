/********************************************************************
 * File: symbols.c
 *
 * Symbols functions.  This file has functions for symbols mgmt
 *  (such as translating addresses to function names with 
 *  addr2line) and also connectivity matrix functions to keep
 *  the function call trace counts.
 *
 * Author: M. Tim Jones <mtj@mtjones.com>
 *
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "stack.h"
#include "symbols.h"
#include "tools.h"

func_t functions[MAX_FUNCTIONS];
unsigned int totals[MAX_FUNCTIONS];
unsigned int calls[MAX_FUNCTIONS][MAX_FUNCTIONS];

char imageName1[100];

void initSymbol(char *image) {
    int from, to;
    if (image != NULL) strcpy(imageName1, image);


    for (from = 0; from < MAX_FUNCTIONS; from++) {

        functions[from].address = 0;
        functions[from].funcName[0] = 0;
        totals[from] = 0;

        for (to = 0; to < MAX_FUNCTIONS; to++) {

            calls[from][to] = 0;

        }

    }

    return;
}


int lookupSymbol(unsigned int address) {
    int index;

    for (index = 0; index < MAX_FUNCTIONS; index++) {

        if (functions[index].address == 0) break;

        if (functions[index].address == address) return index;

    }

    assert(0);

    return 0;
}


int translateFunctionFromSymbol(unsigned int address, char *func) {
    FILE *p;
    char line[100];
    int len, i;
    int flag = 0;
    //从redis里读取方法名的缓存
//    if (!redis_context) {
//        struct timeval timeout = { 1, 500000 }; // 1.5 seconds
//        redis_context = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
//        if (redis_context == NULL || redis_context->err) {
//            if (redis_context) {
//                printf("Connection error: %s\n", redis_context->errstr);
//                redisFree(redis_context);
//            } else {
//                printf("Connection error: can't allocate redis context\n");
//            }
//            exit(1);
//        }
//    }
    reply = redisCommand(redis_context, "get %d", address);

    if (reply->type == REDIS_REPLY_STRING) {
//        func = reply->str; 修复bug,因为这里用的是地址赋值，后面如果reply对象释放掉，原来被赋的值就会消失
        copy_string(func, reply->str);
        flag = 1;
    }
    if(!flag) {
        if (reply == NULL ||
            reply->type == REDIS_REPLY_NIL ||
            reply->type == REDIS_REPLY_ERROR) {
        }
        freeReplyObject(reply);


        sprintf(line, "addr2line -e %s -f -s 0x%x", imageName1, address);


        //在命令行执行addr2line相关的命令
        p = popen(line, "r");

        if (p == NULL) {
            return 0;
        }
        else {

            len = fread(line, 99, 1, p);

            i = 0;
            while (i < strlen(line)) {

                if ((line[i] == 0x0d) || (line[i] == 0x0a)) {
                    func[i] = 0;
                    break;
                } else {
                    func[i] = line[i];
                }

                i++;

            }
            printf("address: %x ", address);
            printf("fushuju fnctionName: %s \n", func);
            pclose(p);

        }
    }


    reply = redisCommand(redis_context, "set %d %s", address, func);
    freeReplyObject(reply);
//    if(redis_context != NULL) redisFree(redis_context);

    return 1;
}


void addSymbol(unsigned int address) {
    int index;


    for (index = 0; index < MAX_FUNCTIONS; index++) {

        if (functions[index].address == address) return;

        //这个进来的方法，不存在就终止循环，此时index就是它的插入位置
        if (functions[index].address == 0) break;

    }

    // 如果方法名池满了,index会等于MAX_FUNCTIONS,这里就会进入断言
    // 找到或者插入新函数
    if (index < MAX_FUNCTIONS) {

        functions[index].address = address;

        translateFunctionFromSymbol(address, functions[index].funcName);

    } else {

        assert(0);

    }

    return;
}


void addCallTrace(unsigned int address) {

    if (stackNumElems()) {
        calls[lookupSymbol(stackTop())][lookupSymbol(address)]++;
    }

    return;
}


void emitSymbols(int pid_tid) {
    //文件名规则pid_tid_graph.dot
    char filename[100];
    char str[15];
    sprintf(str, "%d", pid_tid);
    strcpy(filename, str);
//  strcat(filename,str); 直接strcat为什么会报错
    strcat(filename, "_");
    strcat(filename, "graph.dot");

    int from, to;
    FILE *fp;

    fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Couldn't open graph.dot\n");
        exit(0);
    }

    fprintf(fp, "digraph %s {\n\n", imageName1);
    fprintf(fp, "  rankdir = \"LR\"\n");
    /* Identify node shapes */
    for (from = 0; from < MAX_FUNCTIONS; from++) {

        if (functions[from].address == 0) break;

        for (to = 0; to < MAX_FUNCTIONS; to++) {

            if (functions[to].address == 0) break;

            if (calls[from][to]) totals[from] += calls[from][to];

        }

        if (totals[from]) {
            if (calls[from][to] > 6) continue;
            fprintf(fp, "  %s [shape=plaintext]\n", functions[from].funcName);

        } else {
            fprintf(fp, "  %s [shape=plaintext]\n", functions[from].funcName);

        }

    }

    /* Emit call graph */

//    //固定to,去遍历from
//    for (to = 0; to < MAX_FUNCTIONS; to++) {
//
//        if (functions[to].address == 0) continue;
//        int count = 0;
//        for (from = 0; from < MAX_FUNCTIONS; from++) {
//            if (calls[from][to]) {
//                if (calls[from][to] > 6) continue;
//                //处理多函数调用同一个函数的问题，不是第一次调用的话就在名字后面加"_count"
//                if(count == 0) {
//                    fprintf(fp, "  %s -> %s [fontsize=\"10\"]\n",
//                            functions[from].funcName, functions[to].funcName);
//                } else {
//                    fprintf(fp, "  %s -> %s_%d [fontsize=\"10\"]\n",
//                            functions[from].funcName, functions[to].funcName, count);
//                }
//                count++;
//            }
//        }
//    }

    //固定from,去遍历to
    for (from = 0; from < MAX_FUNCTIONS; from++) {

        if (functions[from].address == 0) break;
        char *rank_str = "{ rank = same;\n";
        int add_rank_str = 0;
        for (to = 0; to < MAX_FUNCTIONS; to++) {

            if (calls[from][to]) {
                if (calls[from][to] > 6) continue;
                fprintf(fp, "  %s -> %s [fontsize=\"10\"]\n",
                        functions[from].funcName, functions[to].funcName
                        );
                rank_str = join(rank_str, "\"");
                rank_str = join(rank_str, functions[to].funcName);
                rank_str = join(rank_str, "\"; ");
                add_rank_str = 1;
            }
            if (functions[to].address == 0) break;

        }

        if(add_rank_str) {
            rank_str = join(rank_str, "\n};\n");
//            fprintf(fp, "  %s\n", rank_str);
        }
    }

    fprintf(fp, "\n}\n");

    fclose(fp);

    return;
}

