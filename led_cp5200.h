/************************************************************************
* @file		  Public header file of CP5200 LEDS Scroll Board
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
// #include <signal.h>
// #include <sys/shm.h>


#define MAX_CP5K2_NUM			(8)
#define MAX_CP5K2_WINDOWS		(8)

typedef struct cp5k2_text_cb_s {
	unsigned char	*text;
	int				text_len;
	unsigned int	color;		// 0x00RRGGBB
	unsigned		effect;		// 0x00:still, 0x0B:left-out, 0x0D:flash
	unsigned		speed;		// the smaller, the faster, 0 ~100 (3)
	unsigned		hold_sec;	// ?? (3)
} cp5k2_text_cb_t;



#pragma pack(push)
typedef struct cp5k2_wn_spec_s {
	uint8_t		XH;		//	窗口的 x 坐标，高位在前
	uint8_t		XL;
	uint8_t		YH;		//	窗口的 y 坐标，高位在前
	uint8_t		YL;
	uint8_t		WH;		//	窗口的 w 坐标，高位在前
	uint8_t		WL;
	uint8_t		HH;		//	窗口的 h 坐标，高位在前
	uint8_t		HL;
} cp5k2_wn_spec_t;
#pragma pack(pop)


void cp5k2_init(void);

void cp5k2_destroy(void);
int cp5k2_device_register(unsigned dev_id, char *ip_addr, int port, unsigned long id_code);
int cp5k2_device_unregister(unsigned dev_id);
int cp5k2_send_CC_command(unsigned dev_id, uint8_t *cc_data, unsigned cc_len);

int cp5k2_splite_window(unsigned dev_id, int n_win, cp5k2_wn_spec_t *wn_spec);
int cp5k2_write_pure_text(unsigned dev_id, int window, cp5k2_text_cb_t *text_cb);
int cp5k2_resume_program(unsigned dev_id);
int cp5k2_save_data(unsigned dev_id);

///*******************************************
#ifdef _cplusplus
}
#endif
///*******************************************
