/*******************************************************************************
 * Copyright (c) 2012, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Ian Craggs - change delimiter option from char to string
 *    Al Stockdill-Mander - Version using the embedded C client
 *******************************************************************************/

/*
 
 stdout subscriber
 
 compulsory parameters:
 
  topic to subscribe to
 
 defaulted parameters:
 
	--host localhost
	--port 1883
	--qos 2
	--delimiter \n
	--clientid stdout_subscriber
	
	--userid none
	--password none

 for example:

    stdoutsub topic/of/interest --host iot.eclipse.org

*/

#include <stdio.h>
#include <signal.h>
#include <memory.h>

#include <sys/time.h>


#include "MQTTClient/MQTTClient.h"
#include "MqttPayload/pando_protocol.h"


volatile int toStop = 0;
int toSend = 1;
int recv_count = 0;

void FUNCTION_ATTRIBUTE decode_data(struct pando_buffer *buf, uint16_t payload_type)
{
    // 网关解包
	pando_protocol_decode(buf, payload_type);

    
    // 网关向子设备传递数据包,如果是向子设备发包，请用对应的设备send函数
    struct sub_device_buffer *device_buffer = pd_malloc(sizeof(struct sub_device_buffer));
    device_buffer->buffer_length = pando_get_package_length(buf);
    device_buffer->buffer = pd_malloc(device_buffer->buffer_length);

 
    // 从网关数据包中获取子设备数据包
    pd_memcpy(device_buffer->buffer, pando_get_package_begin(buf), device_buffer->buffer_length);

    // 获取数据完成，释放网关缓冲区
    pando_buffer_delete(buf);

    show_package(device_buffer->buffer, device_buffer->buffer_length);

    

    /* 以下是子设备解析代码 */
	struct pando_property data_body;
    uint8_t *buf_end = device_buffer->buffer + device_buffer->buffer_length;
    int i = 0;
    uint16_t tlv_type, tlv_length;
    uint8_t *value = pd_malloc(100);
    struct TLV *param = NULL;
    
	struct TLVs *property_block;
	
	// 2.子设备获取命令参数	
    
    uint32_t param1 = 0;
    uint8_t param2 = 0;
    char *param_bytes = NULL;
	while(1)
	{
        property_block = get_sub_device_property(device_buffer, &data_body);

        if (property_block == NULL)
        {
            pd_printf("reach end of buffer\n");
            break;
        }
        pd_printf("count: %d\n", data_body.params->count);
        
        if (data_body.params->count == 3)
        {
            param1 = get_next_uint32(property_block);
            show_package(&param1, sizeof(param1));
            param2 = get_next_uint8(property_block);
            show_package(&param2, sizeof(param2));
            param_bytes = get_next_bytes(property_block, &tlv_length);
            pd_memcpy(value, param_bytes, tlv_length);
            show_package(value, tlv_length);
        }
        else if (data_body.params->count == 2)
        {
            param2 = get_next_uint8(property_block);
            show_package(&param2, sizeof(param2));
            param_bytes = get_next_bytes(property_block, &tlv_length);
            pd_memcpy(value, param_bytes, tlv_length);
            show_package(value, tlv_length);
        }
    }

    pd_free(value);
	// 3. 删除子设备缓冲区
	delete_device_package(device_buffer);
}


/* event exactly the same as command */
void FUNCTION_ATTRIBUTE decode_command(struct pando_buffer *buf, uint16_t payload_type)
{
	// 网关解包
	pando_protocol_decode(buf, payload_type);

    // 网关向子设备传递数据包,如果是向子设备发包，请用对应的设备send函数
    struct sub_device_buffer *device_buffer = pd_malloc(sizeof(struct sub_device_buffer));
    device_buffer->buffer_length = pando_get_package_length(buf);
    device_buffer->buffer = pd_malloc(device_buffer->buffer_length);

 
    // 从网关数据包中获取子设备数据包
    pd_memcpy(device_buffer->buffer, pando_get_package_begin(buf), device_buffer->buffer_length);
 
    // 获取数据完成，释放网关缓冲区
    pando_buffer_delete(buf);




	/* 以下是子设备解析代码 */
	struct pando_command cmd_body;
	// 1.子设备解命令包, 返回参数区的起始位置 
	struct TLVs *cmd_params_block = get_sub_device_command(device_buffer, &cmd_body);
	pd_printf("sub id %02x, cmd num %02x, pri %02x, count: %d\n", 
        cmd_body.sub_device_id, cmd_body.command_id,
        cmd_body.priority, cmd_body.params->count);
	
	// 2.子设备获取命令参数
	uint16_t tlv_length;
    uint8_t *value = pd_malloc(100);

    uint8_t param1 = get_next_uint8(cmd_params_block);
    show_package(&param1, sizeof(param1));
    uint32_t param2 = get_next_uint32(cmd_params_block);
    show_package(&param2, sizeof(param2));
    char *param_bytes = get_next_bytes(cmd_params_block, &tlv_length);
    pd_memcpy(value, param_bytes, tlv_length);
    show_package(value, tlv_length);

    pd_free(value);
	// 3. 删除子设备缓冲区
	delete_device_package(device_buffer);
}


struct sub_device_buffer * FUNCTION_ATTRIBUTE construct_sub_device_data()
{
	struct sub_device_buffer *data_buffer = NULL;
	struct TLVs *params_block = NULL;
    int ret = 0;

	uint16_t property_num = 0x1516;
    uint16_t property_num2 = 0x1122;
    uint16_t flag = 0;

	uint32_t data_param1 = 0xa1b2c3d4;
	uint8_t data_param2 = 0xc1;
	char data_param3[] = "test data";


    //create buffer, remember delete
	data_buffer = create_data_package(flag);
	if (data_buffer == NULL)
	{
		pd_printf("Create data package failed.");
	}

    
	params_block = create_params_block();

	if (params_block == NULL)
	{
		pd_printf("Create first tlv param failed.\n");
		return NULL;
	}

    if (add_next_uint32(params_block, data_param1))
	{
		delete_params_block(params_block);
		pd_printf("Add next tlv param failed.\n");
		return NULL;
	}
    
	if (add_next_uint8(params_block, data_param2))
	{
		delete_params_block(params_block);
		pd_printf("Add next tlv param failed.\n");
		return NULL;
	}

    if (add_next_bytes(params_block, strlen(data_param3), data_param3))
	{
		delete_params_block(params_block);
		pd_printf("Add next tlv param failed.\n");
		return NULL;
	}
	
	add_next_property(data_buffer, property_num, params_block);

    // once the params block has been added to data package
    //you must delete it even you are going to add another block to the package
	delete_params_block(params_block);

    //add different property    
    params_block = create_params_block();
    if (params_block == NULL)
	{
		pd_printf("Create first tlv param failed.\n");
		return NULL;
	}
    
	if (add_next_uint8(params_block, data_param2))
	{
		delete_params_block(params_block);
		pd_printf("Add next tlv param failed.\n");
		return NULL;
	}

    if (add_next_bytes(params_block, strlen(data_param3), data_param3))
	{
		delete_params_block(params_block);
		pd_printf("Add next tlv param failed.\n");
		return NULL;
	}

    ret = add_next_property(data_buffer, property_num2, params_block);
    
    if (ret != 0)
    {
        delete_params_block(params_block);
        pd_printf("add_next_property failed.");
        return NULL;
    }

    //do not forget to delete params block again
    delete_params_block(params_block);
    finish_package(data_buffer);
    
    show_package(data_buffer->buffer, data_buffer->buffer_length);
	return data_buffer;
}


// 创建事件包的步骤
// 1. 使用create_params_block添加第一个参数
// 2. add_next_param直到添加完全部参数
// 3. create_event_package创建数据包，并delete_params_block来释放params_block
struct sub_device_buffer * FUNCTION_ATTRIBUTE construct_sub_device_event()
{
	struct sub_device_buffer *event_buffer = NULL;
	struct TLVs *params_block = NULL;

	uint16_t event_num = 0x0911;
    uint16_t flag = 0x0011;
	uint32_t event_param1 = 0xa1b2c3d4;
	uint8_t event_param2 = 0xc1;
	char event_param3[] = "test data";

    event_buffer = create_event_package(flag);
    if (NULL == event_buffer)
	{
		pd_printf("Create event package failed.");
        return NULL;
	}
    
	params_block = create_params_block();

	if (params_block == NULL)
	{
		pd_printf("Create first tlv param failed.\n");
		return NULL;
	}
	
	if (add_next_param(params_block, TLV_TYPE_UINT8, sizeof(event_param2), &event_param2))
	{
		delete_params_block(params_block);
		pd_printf("Add next tlv param failed.\n");
		return NULL;
	}

    if (add_next_param(params_block, TLV_TYPE_UINT32, sizeof(event_param1), &event_param1))
	{
		delete_params_block(params_block);
		pd_printf("Add next tlv param failed.\n");
		return NULL;
	}

	if (add_next_param(params_block, TLV_TYPE_BYTES, sizeof(event_param3), event_param3))
	{
		delete_params_block(params_block);
		pd_printf("Add next tlv param failed.\n");
		return NULL;
	}
	// 3. create event with flag, priority and tlv params
    add_event(event_buffer, event_num, 0, params_block);
	
	delete_params_block(params_block);
    finish_package(event_buffer);
    
	show_package(event_buffer->buffer, event_buffer->buffer_length);
	return event_buffer;
}

struct pando_buffer * FUNCTION_ATTRIBUTE construct_data_package_to_server(uint16_t *payload_type)
{
	struct pando_buffer *gateway_data_buffer = NULL;
	struct sub_device_buffer *device_data_buffer = NULL;
    uint16_t buf_len = 0;
    
	//构建子设备数据包
	device_data_buffer = construct_sub_device_data();

    buf_len = GATE_HEADER_LEN - DEV_HEADER_LEN + device_data_buffer->buffer_length;    
	gateway_data_buffer = pando_buffer_create(buf_len, GATE_HEADER_LEN - DEV_HEADER_LEN);
    
	if (gateway_data_buffer->buffer == NULL)
	{
		pd_printf("constuct_event_package_to_server malloc failed.\n");
		return NULL;
	}

    printf("len %d, offset %d\n", gateway_data_buffer->buff_len, gateway_data_buffer->offset);
	//复制子设备事件包内容，并释放
	pd_memcpy(
		gateway_data_buffer->buffer + gateway_data_buffer->offset,
		device_data_buffer->buffer, device_data_buffer->buffer_length);
    
	delete_device_package(device_data_buffer);
  
	if (pando_protocol_encode(gateway_data_buffer, payload_type))
	{
		pd_printf("pando_protocol_encode error.\n");
	}

	pando_protocol_set_sub_device_id(gateway_data_buffer, 112);
	show_package(gateway_data_buffer->buffer, gateway_data_buffer->buff_len);
	return gateway_data_buffer;
}

// 创建事件包步骤：
// 1. 获得子设备包construct_sub_device_event
// 2. 然后创建总缓冲区
// 3. 将构建好的子设备数据包拷贝至缓冲区,用offset向encode函数指明子设备数据包中缓冲区中的起始位置
// 4. 释放子设备数据包
// 5. 添加子设备id，返回事件包
struct pando_buffer * FUNCTION_ATTRIBUTE constuct_event_package_to_server(uint16_t *payload_type)
{
	struct pando_buffer *gateway_event_buffer = NULL;
	struct sub_device_buffer *sub_device_event_buffer = NULL;

    uint16_t buf_len = 0;
       
	//构建子设备事件包
	sub_device_event_buffer = construct_sub_device_event();
    
    buf_len = GATE_HEADER_LEN  - DEV_HEADER_LEN + sub_device_event_buffer->buffer_length;    
	gateway_event_buffer = pando_buffer_create(buf_len, GATE_HEADER_LEN - DEV_HEADER_LEN);

	if (gateway_event_buffer->buffer == NULL)
	{
		pd_printf("constuct_event_package_to_server malloc failed.\n");
		return NULL;
	}
	
	//复制子设备事件包内容，并及时释放
	pd_memcpy(
		gateway_event_buffer->buffer + gateway_event_buffer->offset,
		sub_device_event_buffer->buffer, 
		sub_device_event_buffer->buffer_length);
    
	delete_device_package(sub_device_event_buffer);

	if (pando_protocol_encode(gateway_event_buffer, payload_type))
	{
		pd_printf("pando_protocol_encode error.\n");
	}
	
	pando_protocol_set_sub_device_id(gateway_event_buffer, 0x7788);
	show_package(gateway_event_buffer->buffer, gateway_event_buffer->buff_len);
	return gateway_event_buffer;
}


void usage()
{
	printf("MQTT stdout subscriber\n");
	printf("Usage: stdoutsub topicname <options>, where options are:\n");
	printf("  --host <hostname> (default is localhost)\n");
	printf("  --port <port> (default is 1883)\n");
	printf("  --qos <qos> (default is 2)\n");
	printf("  --delimiter <delim> (default is \\n)\n");
	printf("  --clientid <clientid> (default is hostname+timestamp)\n");
	printf("  --username none\n");
	printf("  --password none\n");
	printf("  --showtopics <on or off> (default is on if the topic has a wildcard, else off)\n");
	exit(-1);
}


void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}


struct opts_struct
{
	char* clientid;
	int nodelimiter;
	char* delimiter;
	enum QoS qos;
	char* username;
	char* password;
	char* host;
	int port;
	int showtopics;
} opts =
{
	(char*)"stdout-subscriber", 0, (char*)"\n", QOS2, NULL, NULL, (char*)"localhost", 1883, 0
};


void getopts(int argc, char** argv)
{
	int count = 2;
	
	while (count < argc)
	{
		if (strcmp(argv[count], "--qos") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "0") == 0)
					opts.qos = QOS0;
				else if (strcmp(argv[count], "1") == 0)
					opts.qos = QOS1;
				else if (strcmp(argv[count], "2") == 0)
					opts.qos = QOS2;
				else
					usage();
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--host") == 0)
		{
			if (++count < argc)
				opts.host = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--port") == 0)
		{
			if (++count < argc)
				opts.port = atoi(argv[count]);
			else
				usage();
		}
		else if (strcmp(argv[count], "--clientid") == 0)
		{
			if (++count < argc)
				opts.clientid = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--username") == 0)
		{
			if (++count < argc)
				opts.username = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--password") == 0)
		{
			if (++count < argc)
				opts.password = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--delimiter") == 0)
		{
			if (++count < argc)
				opts.delimiter = argv[count];
			else
				opts.nodelimiter = 1;
		}
		else if (strcmp(argv[count], "--showtopics") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "on") == 0)
					opts.showtopics = 1;
				else if (strcmp(argv[count], "off") == 0)
					opts.showtopics = 0;
				else
					usage();
			}
			else
				usage();
		}
		count++;
	}
	
}


void messageArrived(MessageData* md)
{
	MQTTMessage* message = md->message;
	if (opts.showtopics)
		printf("topic  %.*s\t", md->topicName->lenstring.len, md->topicName->lenstring.data);
	if (opts.nodelimiter)
		printf("delimiter %.*s", (int)message->payloadlen, (char*)message->payload);
	else
		printf("delimiter is %.*s%s", (int)message->payloadlen, (char*)message->payload, opts.delimiter);
	//fflush(stdout);

    char *payload_type_c = strchr(md->topicName->lenstring.data, '/');
    uint16_t type = 0;
    
    if (payload_type_c == NULL)
    {
        printf("Found payload type failed.\n");
        return;
    }
    else
    {
        printf("\n%c, len %d\n", *(payload_type_c + 1), (int)message->payloadlen);
    }

    switch (*(payload_type_c + 1))
    {
        case 'c':
            type = PAYLOAD_TYPE_COMMAND;
            break;
        case 'e':
            type = PAYLOAD_TYPE_EVENT;
            break;
        case 'd':
            type = PAYLOAD_TYPE_DATA;
            break;
        default:
            printf("Wrong type of message: %c\n", *(payload_type_c + 1));
            return;
    }
    
    struct pando_buffer *msg_payload = NULL;
    msg_payload = pando_buffer_create(message->payloadlen,0);
    pd_memcpy(msg_payload->buffer, message->payload, msg_payload->buff_len);

    printf("before decode, %d\n", recv_count);
    show_package(msg_payload->buffer + msg_payload->offset,
        msg_payload->buff_len - msg_payload->offset);
    pando_protocol_decode(msg_payload, type);

    recv_count++;
    printf("after decode, %d\n", recv_count);
    show_package(msg_payload->buffer + msg_payload->offset,
        msg_payload->buff_len - msg_payload->offset);
    
}


int main(int argc, char** argv)
{
	int rc = 0;
    struct pando_buffer *bin_buf;
    uint16_t payload_type = 0;
    MQTTMessage msg;
	unsigned char buf[500];
	unsigned char readbuf[500];
    char payload_type_c[] = {'c', 'e', 'd'};
    
    bin_buf = construct_data_package_to_server(&payload_type);
    decode_data(bin_buf, PAYLOAD_TYPE_DATA);

    /* command test, event is same as command except the type */
    bin_buf = constuct_event_package_to_server(&payload_type);
    decode_command(bin_buf, PAYLOAD_TYPE_COMMAND);
    return 0;
    
	if (argc < 2)
		usage();
	
	char* topic = argv[1];

    getopts(argc, argv);
    
	if (strchr(topic, '#') || strchr(topic, '+'))
		opts.showtopics = 1;
	if (opts.showtopics)
		printf("topic is %s\n", topic);

		

	Network n;
	Client c;

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);

	NewNetwork(&n);
	ConnectNetwork(&n, opts.host, opts.port);
	MQTTClient(&c, &n, 1000, buf, 100, readbuf, 100);
 
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = opts.clientid;
	data.username.cstring = opts.username;
	data.password.cstring = opts.password;

	data.keepAliveInterval = 10;
	data.cleansession = 1;
	printf("Connecting to %s %d\n", opts.host, opts.port);
	
	rc = MQTTConnect(&c, &data);
	printf("Connected %d\n", rc);
    
    printf("Subscribing to %s\n", topic);
	rc = MQTTSubscribe(&c, topic, opts.qos, messageArrived);
	printf("Subscribed %d\n", rc);

	while (!toStop)
	{        
		MQTTYield(&c, 1000);
        
        if (toSend)
        {
            if (strstr(topic, "/d"))
            {
                /* data test */
                bin_buf = construct_data_package_to_server(&payload_type);
                //decode_data(bin_buf, PAYLOAD_TYPE_DATA);
            }

            if (strstr(topic, "/e"))
            {
                /* command test */
                bin_buf = constuct_event_package_to_server(&payload_type);
                //decode_command(bin_buf, PAYLOAD_TYPE_COMMAND);
            }
            msg.qos = opts.qos;
            msg.retained = 0;
            msg.dup = 0;
            
            msg.payload = bin_buf->buffer + bin_buf->offset;
            msg.payloadlen = bin_buf->buff_len - bin_buf->offset;

            /* 
                        before publish a message, you need to generate a topic with payload_type
                        max device id is 8 bytes, hex to char need 8*2 bytes, '/' need 1 byte, type need 1 byte 
                    */
            char publish_topic[8*2 + 1 + 1];
            memset(publish_topic, 0, sizeof(publish_topic));
            sprintf(publish_topic, "%s/%c", opts.clientid, payload_type_c[payload_type - 1]);
            printf(publish_topic);
            
            rc = MQTTPublish(&c, publish_topic, &msg);
            toSend = 0;
            pd_free(bin_buf);
            printf("published rc %d, len %d\n", rc, (int)msg.payloadlen);
        }

	}
	
	printf("Stopping\n");

	MQTTDisconnect(&c);
	n.disconnect(&n);

	return 0;
}


