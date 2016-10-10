/********************************************************************
 * File: stack.c
 *
 * Simple stack implementation.
 *
 * Author: M. Tim Jones <mtj@mtjones.com>
 *
 */

#include <assert.h>

#define MAX_ELEMENTS	10000

static int stack[MAX_ELEMENTS];
static int index;

void stackInit( void )
{
  index = 0;

  return;
}


int stackNumElems( void )
{
  return index;
}


unsigned int stackTop( void )
{
  assert( index > 0 );

  return (stack[index-1]);
}


void stackPush( unsigned int value )
{
  assert ( index < MAX_ELEMENTS );

  stack[index] = value;
  index++;

  return;
}


unsigned int stackPop( void )
{
  unsigned int value;

  //这里的返回值好像也没用，我就直接返回，为了处理堆栈不全的情况
  if(index <= 0) return 0;
//  assert( index > 0 );

  index--;
  value = stack[index];

  return value;
}

