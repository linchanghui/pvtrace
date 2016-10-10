//
// Created by linchanghui on 10/8/16.
//
#include "tools.h"
#include <string.h>

void copy_string(char *target, char *source) {
    while (*source) {
        *target = *source;
        source++;
        target++;
    }
    *target = '\0';
}

char *join(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+5);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    if (result == NULL) exit (1);

    strcpy(result, s1);
//    strcat(result, " ");
    strcat(result, s2);

    return result;
}