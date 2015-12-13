//  Copyright (c) 2014 Pando. All rights reserved.
//  PtotoBuf:   ProtocolBuffer.h
//
//  Brief:
//
//  Create By TangWenhan On 14/12/24.

#ifndef PANDP_PROTOCOL_TOOL_H
#define PANDP_PROTOCOL_TOOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "common_functions.h"
#include "sub_device_protocol.h"
#include "pando_endian.h"

#define MAX_PANDO_TOKEN_SIZE 16

#define MAGIC_HEAD_PANDO 0x7064
#define BIN_PANDO_TAG 0x0001
#define EMPTY_HEART_BEAT 0xffff
#define FLAG_TCP_XXX	//是否现在就要定义具体的flag

#define GATE_HEADER_LEN (sizeof(struct mqtt_bin_header))

#pragma pack(1)

struct mqtt_bin_header
{
    uint8_t flags;
    uint64_t timestamp;
    uint8_t token[MAX_PANDO_TOKEN_SIZE];
};


/*载荷数据的开头，必须有子设备ID，以供Gateway转发和聚合*/
struct pando_payload
{
	uint16_t sub_device_id;
};


/* 缓存结构，用于在网关程序内部流转
 * buffer总是从pando_header开始的。 
 * buffer + offset到buffer + buff_len之间的数据，就是decode或者encode要处理的
*/
struct pando_buffer
{
	uint16_t buff_len;   /* 缓冲区长度 */
	uint16_t offset;     /* 有效数据的偏移量 */
	uint8_t  *buffer;    /* 缓冲区 */
};

/*protocol模块基本数据的结构*/
struct protocol_base
{
	uint64_t device_id;      /* 设备ID */
	uint64_t event_sequence; /* 网关的事件序列 */
	uint64_t data_sequence;  /* 网关的数据序列 */
	uint64_t command_sequence;	/* 网关收到的命令序列 */
	uint32_t sub_device_cmd_seq; /* 保存与子设备交互命令时的序列 */
	uint8_t  token[MAX_PANDO_TOKEN_SIZE];      /* token */
};
#pragma pack()


/*面向网关的接口*/

/* 初始化协议基本参数
 * 成功返回0，错误返回 -1 */
int pando_protocol_init(struct protocol_base init_params);

uint8_t *pando_get_package_begin(struct pando_buffer *buf);
uint16_t pando_get_package_length(struct pando_buffer *buf);


/* 动态新建一个空的缓冲区。length为缓冲区长度，offset进行简单地赋值。
 * 成功则返回缓冲区，错误则返回NULL。*/
struct pando_buffer* pando_buffer_create(int length, int offset);

/* 释放缓冲区动态创建的buffer，释放缓冲区本身 */
void pando_buffer_delete(struct pando_buffer *pdbuf);

/* 将网关与子设备交互的包，转换成网关与接入服务器交互的包
 * 成功返回0，错误返回 -1 */
int pando_protocol_encode(struct pando_buffer *pdbuf, uint16_t *payload_type);

/* 将网关与接入服务器交互的包，转换成网关与子设备交互的包
 * 通过command传出command id, command目标子设备id, command priority
 * 成功返回0，错误返回 -1 */
int pando_protocol_decode(struct pando_buffer *pdbuf, uint16_t payload_type);

/* 网关获取command的序列号，用于反馈机制
 * 得到的是最近一次decode的command的序列
 */
uint64_t pando_protocol_get_cmd_sequence();

/* 获取或设置子设备ID，必须传入payload的起始位置
*/
int pando_protocol_get_sub_device_id(struct pando_buffer *buf, uint16_t *sub_device_id);

int pando_protocol_set_sub_device_id(struct pando_buffer *buf, uint16_t sub_device_id);


/* get command type after gateway completes decoding the command from server. */
uint16_t pando_protocol_get_payload_type(struct pando_buffer *pdbuf);

#ifdef __cplusplus
}
#endif
#endif //PANDP_PROTOCOL_TOOL_H



