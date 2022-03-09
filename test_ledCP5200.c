/************************************************************/
/* This is a stream socket client sample program for UNIX   */
/* domain sockets. This program creates a socket, connects  */
/* to a server, sends data, then receives and prints a      */
/* message from the server.                                 */
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <libgen.h>

#define LOG_INFO 1
#define LOG_DBG 1
#define LOG_WARN 1
#define LOG_VERBOSE 1
#define LOG_FUNC 0
#include "infile_debug.h"

#include "led_cp5200.h"

#include "big5_strings.c"


static void ledScroll_Detecting(unsigned ledID)
{
	cp5k2_text_cb_t txt;

	//-----------------------------
	//-- "辨識中..請勿跟車.." ＠ Lower Window
	//-----------------------------
	memset(&txt, 0, sizeof(cp5k2_text_cb_t));
	txt.color = 0x00FFFF00;
	txt.effect = 0x0B;	// left rolling
	txt.speed = 2;
	txt.text = big5_DETECTING;
	txt.text_len = sizeof(big5_DETECTING);
	cp5k2_write_pure_text(ledID, 1, &txt);
}


static void ledScroll_Maintaining(unsigned ledID)
{
	cp5k2_text_cb_t txt;

	//-----------------------------
	//-- "系統維護中..請刷卡.." ＠ Lower Window
	//-----------------------------
	memset(&txt, 0, sizeof(cp5k2_text_cb_t));
	txt.color = 0x00FF00FF;
	txt.effect = 0x0B;	// left rolling
	txt.speed = 2;
	txt.text = big5_MAINTAINING;
	txt.text_len = sizeof(big5_MAINTAINING);
	cp5k2_write_pure_text(ledID, 1, &txt);
}


static ledScroll_ShowPlateID(unsigned ledID, char *plateID)
{
	cp5k2_text_cb_t txt;

	//-----------------------------
	//-- 顯示 "車牌號碼" ＠ Lower Window
	//-----------------------------
	memset(&txt, 0, sizeof(cp5k2_text_cb_t));
	txt.color = 0x000000FF;
	txt.effect = 0x00;	// freeze still
	txt.speed = 10;
	txt.text = plateID;
	txt.text_len = sizeof(plateID);
	cp5k2_write_pure_text(ledID, 1, &txt);

}


int main(int argc, char *argv[])
{
	cp5k2_init();

	int dev_id = 0;
	cp5k2_device_register(dev_id, "192.168.2.130", 5200, 0xFFFFFFFF);

	// cp5k2_demo_text(dev_id);
	// cp5k2_resume_program(dev_id);


	//-----------------------------
	//-- Splite 2 windows
	//-----------------------------
#if 1
	cp5k2_wn_spec_t wn_spec[2], *p_wnspec;
	p_wnspec = &wn_spec[0];
	p_wnspec->XH = 0x00;	// x = 0
	p_wnspec->XL = 0x00;
	p_wnspec->YH = 0x00;	// y = 0
	p_wnspec->YL = 0x00;
	p_wnspec->WH = 0x00;	// w = 64
	p_wnspec->WL = 64;
	p_wnspec->HH = 0x00;	// h = 16
	p_wnspec->HL = 16;

	p_wnspec = &wn_spec[1];
	p_wnspec->XH = 0x00;	// x = 0
	p_wnspec->XL = 0x00;
	p_wnspec->YH = 0x00;	// y = 16
	p_wnspec->YL = 16;
	p_wnspec->WH = 0x00;	// w = 64
	p_wnspec->WL = 64;
	p_wnspec->HH = 0x00;	// h = 16
	p_wnspec->HL = 16;
	cp5k2_splite_window(dev_id, 2, wn_spec);
	sleep(1);
#endif

	cp5k2_text_cb_t txt;
	//-----------------------------
	//-- "訊舟科技" ＠ Upper Window
	//-----------------------------
	memset(&txt, 0, sizeof(cp5k2_text_cb_t));
	txt.color = 0x00FF0000;
	txt.effect = 0x00;
	txt.text = big5_EDIMAX;
	txt.text_len = sizeof(big5_EDIMAX);
	cp5k2_write_pure_text(dev_id, 0, &txt);
	sleep(1);


	//-----------------------------
	//-- "車牌辨識" ＠ Upper Window
	//-----------------------------
	memset(&txt, 0, sizeof(cp5k2_text_cb_t));
	txt.color = 0x00FFFF00;
	txt.effect = 0x0D;
	txt.speed = 10;
	txt.text = big5_LPR;
	txt.text_len = sizeof(big5_LPR);
	cp5k2_write_pure_text(dev_id, 1, &txt);
	sleep(3);

	for (int i=0; i<5; i++) {
		//-----------------------------
		//-- "辨識中..請勿跟車.." ＠ Lower Window
		//-----------------------------
		ledScroll_Detecting(dev_id);
		sleep(3);

		//-----------------------------
		//-- 顯示 "車牌號碼" ＠ Lower Window
		//-----------------------------
		ledScroll_ShowPlateID(dev_id, "AQX-3159");
		sleep(5);

		//-----------------------------
		//-- "系統維護中..請刷卡.." ＠ Lower Window
		//-----------------------------
		ledScroll_Maintaining(dev_id);
		sleep(3);
		
	}
	

	cp5k2_device_unregister(dev_id);
	cp5k2_destroy();
}
