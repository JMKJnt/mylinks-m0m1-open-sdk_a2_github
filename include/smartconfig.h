/*
 * Copyright (c) 2004-2005, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: pt.h,v 1.7 2006/10/02 07:52:56 adam Exp $
 */

/**
 * \addtogroup pt
 * @{
 */

/**
 * \file
 * Protothreads implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __SMARTCONFIG_H__
#define __SMARTCONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup WiFi_APIs WiFi Related APIs
  * @brief WiFi APIs
  */

/** @addtogroup WiFi_APIs
  * @{
  */

/** \defgroup Smartconfig_APIs Smartconfig APIs
  * @brief SmartConfig APIs
  *
  * SmartConfig can only be enabled in station only mode.
  * Please make sure the target AP is enabled before enable SmartConfig.
  *
  */

/** @addtogroup Smartconfig_APIs
  * @{
  */

typedef enum {
    SC_STATUS_WAIT = 0,             /**< waiting, do not start connection in this phase */
    SC_STATUS_FIND_CHANNEL,         /**< find target channel, start connection by APP in this phase */
    SC_STATUS_GETTING_SSID_PSWD,    /**< getting SSID and password of target AP */
    SC_STATUS_LINK,                 /**< connecting to target AP */
    SC_STATUS_LINK_OVER,            /**< got IP, connect to AP successfully */
} sc_status;

typedef enum {
    SC_TYPE_MLINK = 0,       /**< protocol: MLINK */
    SC_TYPE_AIRKISS,            /**< protocol: AirKiss */
    SC_TYPE_MLINK_AIRKISS,   /**< protocol: MLINK and AirKiss */
} sc_type;


struct _ssid_pwd 
{
  unsigned char ssid[33];
  unsigned char pwd[65];
};

typedef void (*sc_callback_t)(sc_status status, void *pdata);

const char *smartconfig_get_version(void);

int smartconfig_start(sc_callback_t cb);

void smartconfig_stop(void);


#ifdef __cplusplus
}
#endif

#endif
/** @} */
