/************************************************************************
* @file		  Private header file of CP5200 LEDS Scroll Board
* @brief
* @version    0.00.01
* @author     Wu, Charles
* @copyright (c) 2020 Edimax Technology Co., Ltd. All right reserved.
************************************************************************/

#pragma once

///*******************************************
#ifdef _cplusplus
exetern "C" {
#endif
///*******************************************

#include <errno.h>
#include <fcntl.h> // for open
#include <unistd.h>	// getopt and close

#include <stdio.h>
#include <stdint.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <assert.h>
#include <pthread.h>

#include "led_cp5200.h"

#pragma pack(push)
typedef struct cp5k2_cmd_splite_windows_s {
	uint8_t				cc_id;	// 0x01
	uint8_t				n_windows;	// 1 ~ 8
	cp5k2_wn_spec_t		wn_conf[MAX_CP5K2_WINDOWS];
} cp5k2_cmd_splite_windows_t;
#pragma pack(pop)


///*******************************************
#ifdef _cplusplus
}
#endif
///*******************************************
