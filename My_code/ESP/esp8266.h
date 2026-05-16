#ifndef _ESP8266_H_
#define _ESP8266_H_





#define REV_OK		0	//bug־
#define REV_WAIT	1	//bug־



#define MODE "1"        		//esp的模式  0主机  1从机  2混合
#define SSID "realme"   		//wifi名称
#define PWD "2156889631"		//wifi密码
#define USER_NAME "DHT11"       //onenet平台设备名
#define DEVICE_ID "WWH12JbzM9"  //onenet平台设备ID
#define TOKEN "version=2018-10-31&res=products%2FWWH12JbzM9%2Fdevices%2FDHT11&et=1799574660&method=md5&sign=Mc4IGxjtC166AdiyfLnKMg%3D%3D"   //onenet平台TOKEN
#define HTTP "mqtts.heclouds.com" //onenet平台网址

#define RST "AT+RST\r\n"  		//重启设备
#define AT "AT\r\n"				//打招呼
#define CWMODE "AT+CWMODE="MODE"\r\n"  //设置设备模式
#define WIFI_CMD "AT+CWJAP=\""SSID"\",\""PWD"\"\r\n"		//连接wifi
#define MQTTUSERCFG "AT+MQTTUSERCFG=0,1,\""USER_NAME"\",\""DEVICE_ID"\",\""TOKEN"\",0,0,\"\"\r\n"		//云平台配置
#define MQTTCONN "AT+MQTTCONN=0,\""HTTP"\",1883,1\r\n"		//连接云平台
#define MQTTSUB "AT+MQTTSUB=0,\"$sys/" DEVICE_ID "/" USER_NAME "/thing/property/post/reply\",0\r\n"	//AT指令：订阅主题
#define MQTTSUB_SET "AT+MQTTSUB=0,\"$sys/" DEVICE_ID "/" USER_NAME "/thing/property/set\",0\r\n"    //AT指令：回应Onenet平台下发的数据
#define CIPSNTPCFG "AT+CIPSNTPCFG=1,8,\"ntp.ntsc.ac.cn\"\r\n"  //连接国家授时中心——连接上后需要等待至少 2秒 让时间同步
#define CIPSNTPTIME "AT+CIPSNTPTIME?\r\n"	//获取当前时间




void ESP8266_Init(void);

void ESP8266_Clear(void);

void ESP8266_Clear_cpy(void);

void ESP8266_SendData(unsigned char *data, unsigned short len);

unsigned char *ESP8266_GetIPD(unsigned short timeOut);

_Bool ESP8266_Send(char *cmd, char *res);


#endif
