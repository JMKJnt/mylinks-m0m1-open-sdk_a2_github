/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file omniconfig.h 
*   \brief define omniconfig result and interface functions 
*   \author Montage
*/
#ifndef OMNICONFIG_FUNC_H
#define OMNICONFIG_FUNC_H

#ifdef CONFIG_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#endif

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

/*-----------------------------------------------------------------------------+
| System Function (Let users choose CONFIG_FREERTOS)                                |
+-----------------------------------------------------------------------------*/

void* omnicfg_thread_new(	const char *name,
						void (*thread)(void *arg),
						void *arg, int stacksize, int prio) 
{
#if defined(CONFIG_FREERTOS)
	TaskHandle_t xCreatedTask;
	portBASE_TYPE xResult;
	void* xReturn;
	
	xResult = xTaskCreate( thread, name, stacksize, arg, prio, &xCreatedTask );
	
	if( xResult == pdPASS )
	{
		xReturn = xCreatedTask;
	}
	else
	{
		xReturn = NULL;
	}
	
	return (void*)xReturn;
#endif
}

void omnicfg_thread_end()
{
#if defined(CONFIG_FREERTOS)
	vTaskDelete(NULL);
#endif
}

void omnicfg_msleep(unsigned int ms) 
{
#if defined(CONFIG_FREERTOS)
	if(0 == ms)
		vTaskDelay(0);
	else
		vTaskDelay(ms/portTICK_PERIOD_MS);
#endif
}

#endif /*OMNICONFIG_FUNC_H*/
