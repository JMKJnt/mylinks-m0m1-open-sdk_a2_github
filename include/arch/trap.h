/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief 
*   \author Montage
*/

#ifndef TRAP_H
#define TRAP_H

typedef void (*ifunc)(void *);
typedef struct irq_handler
{
    ifunc hnd;
    unsigned int id;
} irq_handler;

void set_ex_handler(unsigned int exp, void (*handler) (), void *id);
void trap_init();

#endif
