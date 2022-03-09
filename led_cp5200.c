
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include "led_cp5200_private.h"

#define LOG_INFO	1
#define LOG_WARN	1
#define LOG_DBG		0
#define LOG_VERBOSE	0
#define LOG_FUNC	0
#include "infile_debug.h"

static pthread_mutex_t _mtx = PTHREAD_MUTEX_INITIALIZER;

typedef struct cp5k2_device_conf_s {
	unsigned		used;

	struct in_addr	sock_addr;
	int				sockfd;

	uint8_t			*cmd_header;
} cp5k2_device_conf_t;

static cp5k2_device_conf_t 	_dev_conf[MAX_CP5K2_NUM];

#define CP5K2_CMD_HEADER_LEN		(17)
static uint8_t cp5k2_cmd_header[CP5K2_CMD_HEADER_LEN] = {
	'U', 'U', 'U', 'U',		/* [0~3],4 bytes: ID CODE, ((( 高字节在前 ))) */
	'U', 'U', 				/* [4~5],2 bytes: 數據長度，从”包类型”开始，到“数据校验和”的数据字节数, 低位在前*/
	0x00, 0x00,				/* [6~7],2 bytes: Reserved */
	0x68, 0x32,				/* [8~9],2 bytes: 包类型,卡类型码 */
	0xFF,					/* [10], 1 bytes: 卡 ID */
	0x7B,					/* [11], 1 bytes: 协议码 */
	0x01,					/* [12], 1 bytes: 是否要返回确认信息，1 要，0 不要 */
	'U', 'U', 				/* [13~14] 2 bytes: 包数据长度, 表示后面“CC...”内容部分的长度，低字节在前*/
	0x00, 0x00,				/* [15~16] 2 bytes: 包序号+最末包序号 */
};


///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
static cp5k2_device_conf_t *get_devce_conf(unsigned dev_id)
{
	if (dev_id > MAX_CP5K2_NUM-1) {
		error_printf("Invalid dev_id= %d !!\n", dev_id);
		return NULL;
	}
	return (&_dev_conf[dev_id]);
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
static void format_CC_len(cp5k2_device_conf_t *p, unsigned cc_len)
{
	unsigned len = cc_len & 0xFFFFu;

	assert(p);

	//-- 低位在前
	p->cmd_header[13] = (uint8_t)(len & 0xFFu);

	len >>= 8;
	p->cmd_header[14] = (uint8_t)(len & 0xFFu);

	debug_printf("format CC_len %d -> %02X %02X \n", cc_len, p->cmd_header[13], p->cmd_header[14] );
}


///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
static void format_packet_len(cp5k2_device_conf_t *p, unsigned pkt_len)
{
	unsigned len = pkt_len & 0xFFFFu;

	assert(p);

	//-- 低位在前
	p->cmd_header[4] = (uint8_t)(len & 0xFFu);

	len >>= 8;
	p->cmd_header[5] = (uint8_t)(len & 0xFFu);

	debug_printf("format pkt_len %d -> %02X %02X \n", pkt_len, p->cmd_header[4], p->cmd_header[5] );
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
static void format_id_code(cp5k2_device_conf_t *p, unsigned long id_code)
{
	unsigned long code;
	assert(p);

	code = id_code;

	//-- 高字节在前
	p->cmd_header[3] = (uint8_t)(code & 0xFFu);

	code >>= 8;
	p->cmd_header[2] = (uint8_t)(code & 0xFFu);

	code >>= 8;
	p->cmd_header[1] = (uint8_t)(code & 0xFFu);

	code >>= 8;
	p->cmd_header[0] = (uint8_t)(code & 0xFFu);
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
static void calculate_chksum(uint8_t chksum[2], uint8_t *data, int cc_len)
{
	// 从“包类型” 0x68 到“包数据”所有字节相加的和。
	uint16_t u16_sum = 0;
	uint8_t *d = data + 8;	// skip ID_code(4), pkt_len(2), reserve(2)
	int len = (CP5K2_CMD_HEADER_LEN - 8) + cc_len;
	while(len-- > 0) {
		u16_sum += *d++;
	}
	chksum[0] = (uint8_t)(u16_sum & 0xFF);
	chksum[1] = (uint8_t)((u16_sum >> 8) & 0xFF);

	debug_printf("chksum %04X -> %02X %02X \n", u16_sum, chksum[0], chksum[1]);
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
static int connect_device(cp5k2_device_conf_t *p, char *ip_addr, int port)
{
	int sockfd;
	int rc;

	//-- create client socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd) {
		error_printf("Create socket failed!!\n");
		return -1;
	}
	p->sockfd = sockfd;

	//-- configure server address
    struct sockaddr_in sock_addr;
    bzero(&sock_addr, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(ip_addr);
    sock_addr.sin_port = htons(port);

	verbose_printf("Connecting server addr= %s, port= %d \n", ip_addr, port);

	//-- FIXME: 開機會失敗 先 delay 一下
	int timeout_sec = 10;
	while(timeout_sec--) {
		rc = connect(sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
		if ( -1 == rc ) {
			warn_printf("Trying [%d] connecting to server at %s:%d failed!! errno=%d, %s\n", timeout_sec, ip_addr, port, errno, strerror(errno));
			sleep(1);
			continue;
		}
		break;
	}

	if (rc < 0) {
		error_puts("Failed to connect to LED Board!!\n");
		return -1;
	}

	verbose_printf("Server connected ...\n");
	return 0;
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
int cp5k2_device_register(unsigned dev_id, char *ip_addr, int port, unsigned long id_code)
{
	cp5k2_device_conf_t *p;
	if (dev_id > MAX_CP5K2_NUM-1) {
		error_printf("Invalid dev_id= %d !!\n", dev_id);
		return -1;
	}

	pthread_mutex_lock(&_mtx);

	p = &_dev_conf[dev_id];
	if (p->used) {
		pthread_mutex_unlock(&_mtx);

		warn_printf("Device id= %d is used.\n", dev_id);
		return -1;
	}

	if (connect_device(p, ip_addr, port) < 0) {
		pthread_mutex_unlock(&_mtx);

		error_printf("Connect device id= %d failed!!\n", dev_id);
		return -1;
	}

	p->used = 1;

	pthread_mutex_unlock(&_mtx);

	format_id_code(p, id_code);


	return 0;
}


///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
int cp5k2_device_unregister(unsigned dev_id)
{
	cp5k2_device_conf_t *p;
	if (dev_id > MAX_CP5K2_NUM-1) {
		error_printf("Invalid dev_id= %d !!\n", dev_id);
		return -1;
	}

	pthread_mutex_lock(&_mtx);

	p = &_dev_conf[dev_id];
	if (p->used) {
		if (p->sockfd >= 0) {
			close(p->sockfd);
			p->sockfd = -1;
		}
	}
	p->used = 0;

	pthread_mutex_unlock(&_mtx);

	return 0;
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
static int receive_response(cp5k2_device_conf_t *p)
{
	fd_set fdsr;
	struct timeval timeout;
	int rc;

	FD_ZERO(&fdsr);					// clear socket readset
	FD_SET(p->sockfd, &fdsr);		// fdsr --> socket readset

	timeout.tv_sec  = 2;
	timeout.tv_usec = 0;
	rc = select(p->sockfd+1, &fdsr /*readset*/, NULL/*writeset*/, NULL/*exceptset*/, &timeout);

	if (-1 == rc) {
		error_printf("SELECT client socket error - errono=%d, %s \n", errno, strerror(errno));
	}
	else if (0 == rc) {
		// Timeout
		error_puts("SELECT timeout!!\n");
	}

	uint8_t rbuf[256];
	rc = recv(p->sockfd, (void*)rbuf, sizeof(rbuf), 0);
	if (rc == 256) {
		warn_printf("Large response packet!!\n");
	}

	// debug_printf("Response code= %02X ...\n", rbuf[12]);

	return rc;
}



///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
int cp5k2_send_CC_command(unsigned dev_id, uint8_t *cc_data, unsigned cc_len)
{
	cp5k2_device_conf_t *p;
	if (dev_id > MAX_CP5K2_NUM-1) {
		error_printf("Invalid dev_id= %d !!\n", dev_id);
		return -1;
	}
	p = &_dev_conf[dev_id];

	//-- total size to send = header + CC_len + Chksum
	unsigned buf_size = CP5K2_CMD_HEADER_LEN + cc_len + 2;
	uint8_t *buf = (uint8_t *)malloc(buf_size);
	assert(buf);
	// debug_printf("buf_size= %d\n", buf_size);

	// 包数据长度
	format_CC_len(p, cc_len);

	// 网络数据长度 = buf_size - 4(id_code) - 2(pkt_lem) - 2(reserved)
	format_packet_len(p, (buf_size- 8));

	uint8_t *p_send = buf;
	memcpy(p_send, p->cmd_header, CP5K2_CMD_HEADER_LEN);
	p_send += CP5K2_CMD_HEADER_LEN;

	memcpy(p_send, cc_data, cc_len);
	p_send += cc_len;

	#if 0	// DBUG only
	do {
		uint8_t *d = buf;
		printf("---- packet data dump, buf_size= %d  ------\n", buf_size);
		for (int i=0; i<buf_size-2; i++) {
			printf("%02X ", *d++);
		}
		printf("\n");
	} while(0);
	#endif

	//-- caluclate CheckSum
	uint8_t chksum[2];
	calculate_chksum(chksum, buf, cc_len);

	*p_send++ = chksum[0];
	*p_send = chksum[1];

	#if 0	// DBUG only
	do {
		uint8_t *d = buf;
		printf("---- packet data dump, buf_size= %d  ------\n", buf_size);
		for (int i=0; i<buf_size; i++) {
			printf("%02X ", *d++);
		}
		printf("\n");
	} while(0);
	#endif

	//-- SEND CP5200 CC command
	if (send(p->sockfd, buf, buf_size, 0) != buf_size) {
		error_printf("Socket send failed!!\n");
		free(buf);
		return -1;
	}

	//-- Get Response
	//receive_response(p);
	usleep(100*1000);


	if (buf) free(buf);
	return buf_size;
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
void cp5k2_init(void)
{
	memset(_dev_conf, 0, sizeof(cp5k2_device_conf_t) * MAX_CP5K2_NUM);

	for (int i=0; i<MAX_CP5K2_NUM; i++) {
		uint8_t *p= (uint8_t *)malloc(CP5K2_CMD_HEADER_LEN);
		_dev_conf[i].cmd_header = p;
		_dev_conf[i].sockfd = -1;

		memcpy(p, cp5k2_cmd_header, CP5K2_CMD_HEADER_LEN);
	}
}


///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
void cp5k2_destroy(void)
{
	cp5k2_device_conf_t *p = _dev_conf;
	for (int i=0; i<MAX_CP5K2_NUM; i++) {
		if (p->cmd_header) free(p->cmd_header);
		if (p->sockfd < 0) close(p->sockfd);

		++p;
	}
}


///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
// int cp5k2_write_text(unsigned dev_id, unsigned char *text, int text_len)
int cp5k2_demo_text(unsigned dev_id)
{
	uint8_t cc_data[] = {
		0x12, 0x00, 0x0B, 0x05,
		0x03, 0x00, 0x03, 0x02,
		0x00, 0xFF, 0x00,
		0xBF, 0xEB, 0xC3, 0xD1, 0xA4, 0xA4, 0x00
	};

	return cp5k2_send_CC_command(dev_id, cc_data, sizeof(cc_data) );
}


///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :	dev_id
/// @param    :	window
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
int cp5k2_write_pure_text(unsigned dev_id, int window, cp5k2_text_cb_t *text_cb)
{
	cp5k2_device_conf_t *conf;

	conf = get_devce_conf(dev_id);
	assert(conf);

	uint8_t attr[] = {
		0x12, /*[0] cmd id*/
		0x00 /*[1]window*/, 0x0B /*[2]effect*/, 0x05 /*[3]center-aligned*/,
		0x03 /*[4]speed*/,
		0x00, 0x05, /*[5~6]hold_sec*/
		0x02, /*[7] font-size, 2=16pt*/
		0x00, 0xFF, 0x00	/*[8~10]: color R/G/B */
		/* [11~] Text ended with \0*/
	};

	int attr_size = sizeof(attr);
	int buf_size = attr_size + text_cb->text_len;
	uint8_t *cc_buf = malloc(buf_size);
	assert(cc_buf);
	uint8_t *p_send = cc_buf;

	//-- set window
	attr[1] = (uint8_t)window;

	//-- set effect
	unsigned effect = (text_cb->effect & 0xFFu);
	attr[2] = (uint8_t)effect;

	//-- set speed
	unsigned speed = (text_cb->speed & 0xFFu);
	speed = !speed ? 3 : speed;
	attr[4] = (uint8_t)speed;

	//-- set color
	unsigned color = text_cb->color & 0x00FFFFFFu;
	attr[10] = (uint8_t)(color & 0xFFu);	// Blue
	color >>= 8;
	attr[9] = (uint8_t)(color & 0xFFu);	// Green
	color >>= 8;
	attr[8] = (uint8_t)(color & 0xFFu);	// Red

	//-- set cc_data
	memcpy(p_send, attr, attr_size);
	p_send += attr_size;

	memcpy(p_send, text_cb->text, text_cb->text_len);

	int rc = cp5k2_send_CC_command(dev_id, cc_buf, buf_size);

	if(cc_buf) free(cc_buf);
	return rc;
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
int cp5k2_splite_window(unsigned dev_id, int n_win, cp5k2_wn_spec_t *wn_spec)
{
	cp5k2_device_conf_t *dev_conf;

	dev_conf = get_devce_conf(dev_id);
	assert(dev_conf);

	int wn_num = n_win;
	int wn_spec_size = sizeof(cp5k2_wn_spec_t);
	int cc_len = 1 + 1 + (wn_num * wn_spec_size);
	uint8_t *cc_data = (uint8_t *)malloc(cc_len);
	assert(cc_data);

	cp5k2_cmd_splite_windows_t *cmd_splite = (cp5k2_cmd_splite_windows_t *)cc_data;
	cmd_splite->cc_id = 0x01;
	cmd_splite->n_windows = wn_num;
	memcpy(cmd_splite->wn_conf, wn_spec, (wn_num * wn_spec_size));

	int rc = cp5k2_send_CC_command(dev_id, cc_data, cc_len);

	if (cc_data) free(cc_data);
	return rc;
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
int cp5k2_save_data(unsigned dev_id)
{
	cp5k2_device_conf_t *dev_conf;

	dev_conf = get_devce_conf(dev_id);
	assert(dev_conf);

	uint8_t	cc_data[4] = {
		0x07,	/*[0]cc_cmd=0x07 请求控制卡保存各窗口的数据*/
		0x00,	/*[1] 0x00/0x01: Save/Clear windows flash data*/
		0x00, 0x00	/*[2:3] reserved */
	};

	int rc = cp5k2_send_CC_command(dev_id, cc_data, 4);
	return rc;
}

///--------------------------------------------------------------------------------------
/// @brief    :
/// @param    :
/// @return   :
/// @details  :
///--------------------------------------------------------------------------------------
int cp5k2_resume_program(unsigned dev_id)
{
	uint8_t cc=0x06;
	return cp5k2_send_CC_command(dev_id, &cc, 1);
}