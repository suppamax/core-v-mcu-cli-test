#include <app/include/iot_task.h>
#include "hal/include/hal_udma_i2cm_reg_defs.h"
#include "drivers/include/udma_uart_driver.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/unistd.h>

static char thing_name[37] = {0};
static uint8_t i2c_buffer[256] = {0};

void busy_sleep(const unsigned int n)
{
	volatile unsigned int i;
	for (i = 0; i < n*1000000; i++);
}

void at_connect()
{
    char msg[32];
    char ok[] = "OK";

	strcpy(msg, "AT+CONNECT\n");
    while (1) {
    	udma_uart_flush(1);
    	udma_uart_writeraw(1, strlen(msg), msg);
    	udma_uart_read_mod(1, sizeof(msg), msg);
    	if (strncmp(msg, ok, sizeof(ok)-1) == 0)
    		break;
    	strcpy(msg, "AT+CONNECT\n");
    	busy_sleep(10);
    }
	CLI_printf("ESP connected\r\n");
}

void at_getthingname()
{
    char msg[96];
    char ok[] = "OK";

	strcpy(msg, "AT+CONF? ThingName\n");
    while (strncmp(msg, ok, sizeof(ok)-1)) {
    	strcpy(msg, "AT+CONF? ThingName\n");
    	udma_uart_flush(1);
    	udma_uart_writeraw(1, strlen(msg), msg);
    	udma_uart_read_mod(1, sizeof(msg), msg);
    }
    strncpy(thing_name, &msg[3], 36);
    thing_name[36] = '\0';
	CLI_printf("Thing name: %s\r\n", thing_name);

}

void at_check()
{
    char msg[32];
    char ok[] = "OK\n";

	strcpy(msg, "AT\n");
    while (strncmp(msg, ok, sizeof(ok)-1)) {
    	strcpy(msg, "AT\n");
    	udma_uart_flush(1);
    	udma_uart_writeraw(1, strlen(msg), msg);
    	udma_uart_read_mod(1, sizeof(msg), msg);
    	CLI_printf("received: %s\r\n", msg);
    }
	CLI_printf("AT check done\r\n");
}

void at_set_topic()
{
    char root_topic[] = "AT+CONF Topicroot=test\n";
    char topic1[] = "AT+CONF Topic1=data\n";
    char ok[] = "OK\n";
    char resp[32];

    resp[0] = '\0';

    while (strncmp(resp, ok, sizeof(ok)-1)) {
    	udma_uart_flush(1);
    	udma_uart_writeraw(1, strlen(root_topic), root_topic);
    	udma_uart_read_mod(1, sizeof(resp), resp);
    }

    resp[0] = '\0';

    while (strncmp(resp, ok, sizeof(ok)-1)) {
    	udma_uart_flush(1);
    	udma_uart_writeraw(1, strlen(topic1), topic1);
    	udma_uart_read_mod(1, sizeof(resp), resp);
    }

	CLI_printf("Topic configured\r\n");
}

void at_send_measure()
{
    char msg[256];
    char ok[] = "OK\n";
    char resp[32];

    int temp;
	int tempi;
	int tempd;

	udma_i2cm_read(1, 0x96, 0x00, 2, i2c_buffer, false);
	temp = (i2c_buffer[0] << 8) + i2c_buffer[1];
	tempi = (temp >> 7);
	tempd = (temp & 0x7F) * 625;
	CLI_printf("The temperature here is %d.%d C\r\n", tempi, tempd);

	sprintf(msg, "AT+SEND1 { \"id\" : \"%s\", \"num\" : %d.%d }\n", thing_name, tempi, tempd);
    resp[0] = '\0';

	udma_uart_flush(1);
	udma_uart_writeraw(1, strlen(msg), msg);
	udma_uart_read_mod(1, sizeof(resp), resp);
	if (strncmp(resp, ok, sizeof(ok)-1))
		CLI_printf("Error sending measurement\r\n");
}

void iot_app( void *pParameter )
{
	(void)pParameter;

	udma_uart_open(1, 115200);
	udma_uart_open(0, 115200);

	while (1) {
		at_check();
		at_getthingname();
		at_set_topic();
		while (1) {
			at_connect();
			at_send_measure();
			busy_sleep(10);
		}
	}
}
