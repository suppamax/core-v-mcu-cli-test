#ifndef __IOT_TASK_H__
#define __IOT_TASK_H__

void at_check();
void at_getthingname();
void at_set_topic();
void at_connect();
void at_send_measure();

void iot_app( void *pParameter );

#endif
