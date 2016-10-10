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
#include <pthread.h>
#include <unistd.h>
#include <syscall.h>


/* Function prototypes with attributes */
void main_constructor( void )
	__attribute__ ((no_instrument_function, constructor));

void main_destructor( void )
	__attribute__ ((no_instrument_function, destructor));

void __cyg_profile_func_enter( void *, void * ) 
	__attribute__ ((no_instrument_function));

void __cyg_profile_func_exit( void *, void * )
	__attribute__ ((no_instrument_function));

char* join3(char *s1, char *s2);

// 为了处理多进程这里用数组？
//static FILE *fp;
pthread_mutex_t mut; //互斥锁类型

void (*callback)() = NULL;

double register_function( void (*in_main_func)())
{
	callback = in_main_func;
//	in_main_func();

}

void function_needing_callback()
{
//	printf("function_needing_callback");
	if(callback != NULL) callback();

}
void main_constructor( void )
{
	//我觉得这个函数在多进程的情况下只会被访问一次，所以要把这里的文件打开去掉，加到__cyg_profile_func_enter具体方法里，用线程号做文件名
	//最后的测试了下，会访问多次，但是这多次访问都是主进程，所以等于是只有一次访问
}

void main_deconstructor( void )
{
}




void __cyg_profile_func_enter( void *this, void *callsite )
{
function_needing_callback();

//	printf("begin __cyg_profile_func_enter, pid:%d, tid:%d\n", syscall(SYS_gettid), getpid());

	pthread_mutex_lock(&mut); //加锁，用于对共享变量操作
	char* filename = "";
	char pid[15];
	sprintf(pid, "%d", syscall(SYS_gettid));
//	filename = join3(pid, "_");
	char tid[15];
	sprintf(tid, "%d", getpid());
	filename = join3(filename, tid);
	filename = join3(filename, "_");
	filename = join3(filename, "trace.txt");

	FILE *fp = fopen( filename, "a" );
	if (fp == NULL) exit(-1);

	fprintf(fp, "E%p\n", (int *)this);

	fclose( fp );

	pthread_mutex_unlock(&mut); //解锁
}


void __cyg_profile_func_exit( void *this, void *callsite )
{
//	printf("begin __cyg_profile_func_exit, pid:%d, tid:%d", syscall(SYS_gettid), getpid());

	pthread_mutex_lock(&mut); //加锁，用于对共享变量操作
	char* filename = "";
	char pid[15];
	sprintf(pid, "%d", syscall(SYS_gettid));
//	filename = join3(pid, "_");
	char tid[15];
	sprintf(tid, "%d", getpid());
	filename = join3(filename, tid);
	filename = join3(filename, "_");
	filename = join3(filename, "trace.txt");

	FILE *fp = fopen( filename, "a" );
	if (fp == NULL) exit(-1);

	fprintf(fp, "X%p\n", (int *)this);

	fclose( fp );
	pthread_mutex_unlock(&mut); //解锁
}


/*方法三，调用C库函数,*/
char* join3(char *s1, char *s2)
{
	char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
	//in real code you would check for errors in malloc here
	if (result == NULL) exit (1);

	strcpy(result, s1);
	strcat(result, s2);

	return result;
}



