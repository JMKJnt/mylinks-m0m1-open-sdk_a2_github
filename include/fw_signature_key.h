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

#ifndef _FW_SIGHNATURE_KEY_H_
#define _FW_SIGHNATURE_KEY_H_

#ifdef CONFIG_FW_SIGN_AESENC	
	#define FW_AESKEY "qwertyuiasdfghjl"
#elif CONFIG_FW_SIGN_ECCENC
	#define FW_ECCKEY "03:69:d5:ba:05:36:5b:32:ea:5b:d1:13:88:8c:4f:f1:14:ae:ff:14:88:37:86:d5:56:43:e2:23:b4:20:45:00:36"
#endif

#endif
