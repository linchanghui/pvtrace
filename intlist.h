//
// Created by linchanghui on 10/7/16.
//

#ifndef PVTRACE_LIST_H
#define PVTRACE_LIST_H

#endif //PVTRACE_LIST_H
int tempcount=0;
void begin(int **n, int value)
{
    *n=malloc(sizeof(int));

    if(*n==NULL)
    {
        printf("Error in malloc!");
        return;
    }

    (*n)[0]=value;

    printf("Added %d \n", (*n)[0]);
}

void add(int **n, int numToAdd)
{
    static int sizeCount=0;
    sizeCount++;
    tempcount=sizeCount;

    int *temp;

    temp=realloc(*n, (sizeCount+1) * sizeof(int));

    if(temp==NULL)
    {
        printf("Error in realloc!");
        return;
    }

    *n=temp;

    (*n)[sizeCount]=numToAdd;

    printf("Added %d \n", (*n)[sizeCount]);

}
