/*******************************************************
 * File name：sub_device_protocol.h
 * Author:    Zhao Wenwu
 * Versions:  0.1
 * Description: APIs for sub device.
 * History:
 *   1.Date:
 *     Author:
 *     Modification:    
 *********************************************************/

#ifndef SUB_DEVICE_PROTOCOL_TOOL_H
#define SUB_DEVICE_PROTOCOL_TOOL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "common_functions.h"
#include "pando_endian.h"

#define MAGIC_HEAD_SUB_DEVICE 0x34
#define PAYLOAD_TYPE_COMMAND 1
#define PAYLOAD_TYPE_EVENT	 2
#define PAYLOAD_TYPE_DATA	 3

#define	TLV_TYPE_FLOAT64 1 
#define TLV_TYPE_FLOAT32 2 
#define	TLV_TYPE_INT8    3
#define	TLV_TYPE_INT16   4
#define	TLV_TYPE_INT32   5
#define TLV_TYPE_INT64   6 
#define	TLV_TYPE_UINT8   7
#define TLV_TYPE_UINT16  8
#define TLV_TYPE_UINT32  9
#define TLV_TYPE_UINT64 10
#define	TLV_TYPE_BYTES  11
#define	TLV_TYPE_URI    12
#define	TLV_TYPE_BOOL   13

#define DEV_HEADER_LEN (sizeof(struct device_header))

#pragma pack(1)

/* a device packet is made up with header and payload */
struct device_header
{
    uint8_t  magic;         /* magic number (0x34) */
    uint8_t  crc;           /* crc of whole packet with crc is zero */
    uint16_t payload_type;  /* type of packet, event, command, data */
    uint16_t payload_len;   /* length of packet without header */
    uint16_t flags;         /* flags for extending protocol  */
    uint32_t frame_seq;     /* frame sequence between device and gateway */
};

/* TLV contains the real information of payload.
   There are 13 types, such as int8, unint16, length is sizeof the type. 
   If type such as int32 can present length of itself, TLV is made up of type 
   and value.
*/
struct TLV 
{
	uint16_t type;
	uint16_t length;
	uint8_t value[];
};

/* Count presents packet how many tlv informations in a packet */
struct TLVs
{
    uint16_t count;
    //struct TLV tlv[];
};

/* Payload of command */
struct pando_command
{
	uint16_t sub_device_id;   /* ID of device */
	uint16_t command_num;     /* different command number presents different operation */
	uint16_t priority;        /* device should handle high priority command first */
	struct TLVs params[1];    /* params for command operation */
};

/* Payload of event */
struct pando_event
{
	uint16_t sub_device_id;   /* ID of device */
	uint16_t event_num;       /* Different event number presents different type event */
    uint16_t priority;        /* Device should report high priority event first */
	struct TLVs params[1];    /* Params for event to report */
};

/* Payload of data */
struct pando_property
{
	uint16_t sub_device_id;   /* ID of device */
	uint16_t property_num;	  /* Different property number presents different type data property */
	struct TLVs params[1];    /* Params for data to report */
};

struct sub_device_buffer
{
	uint16_t buffer_length;
	uint8_t *buffer;
};

struct sub_device_base_params
{
	uint32_t event_sequence;
	uint32_t data_sequence;
	uint32_t command_sequence;
};
#pragma pack()


/*******************************************************
 * Description: Initialize sequence number to 0, it's unnecessary now.
 * param
        base_params: sequence of data, event, command.
 * return: 0 if success, -1 if failed.
 *********************************************************/
int init_sub_device(struct sub_device_base_params base_params);

/*******************************************************
 * Description: Create a block buffer before adding tlv params. 
        It's necessary to delete the buffer when complete creating package.
 * param
 * return: Buffer of params block.
 *********************************************************/
struct TLVs *create_params_block();

/*******************************************************
 * Description: Create package buffer, need add params block to finish package.
 * param
        flags: Extend for new features.
 * return: Buffer of package.
 *********************************************************/
struct sub_device_buffer *create_command_package(uint16_t flags);

/*******************************************************
 * Description: Create package buffer, need add params block to finish package.
 * param
        flags: Extend for new features.
 * return: Buffer of package.
 *********************************************************/
struct sub_device_buffer *create_event_package(uint16_t flags);

/*******************************************************
 * Description: Create package buffer, need add params block to finish package.
 * param
        flags: Extend for new features.
 * return: Buffer of package.
 *********************************************************/
struct sub_device_buffer *create_data_package(uint16_t flags);

/*******************************************************
 * Description: Calculate payload length and crc to finish package construct.
 * param
        package_buf: buffer contains device package.
 * return:
 *********************************************************/
int finish_package(struct sub_device_buffer *package_buf);

/*******************************************************
 * Description: Add data params block and property number to package.
 * param
        data_package: buffer contains device package.
        property_num: property number, indicate data type.
        next_data_params: all data params, count means how many params.
 * return: 0 if success, -1 if failed. 
 *********************************************************/
int add_next_property(struct sub_device_buffer *data_package, uint16_t property_num, 
    struct TLVs *next_data_params);

/*******************************************************
 * Description: Add command params block, priority and command number to package.
 * param
        command_package: buffer contains device package.
        command_num: command number, indicate command type.
        priority: high priority command should be processed first.
        command_params: all command params, count means how many params.
 * return: 0 if success, -1 if failed.
 *********************************************************/
int add_command(struct sub_device_buffer *command_package, uint16_t command_num,
    uint16_t priority, struct TLVs *command_params);

/*******************************************************
 * Description: Add event params block, priority and event number to package.
 * param
    out:
        event_package: buffer contains device package.
    in:
        event_num: event number, indicate event type.
        priority: high priority event should be uploaded to server first.
        event_params: all event params, count means how many params.
 * return: 0 if success, -1 if failed.

 *********************************************************/
int add_event(struct sub_device_buffer *event_package, uint16_t event_num,
    uint16_t priority, struct TLVs *event_params);

/*******************************************************
 * Description: Get command from device buffer directly.
 * param
    in:
        device_buffer: buffer contains device package.
    out:
        command_body: include command number, priority and sub device id.
 * return: 0 if success, -1 if failed.
 *********************************************************/
struct TLVs *get_sub_device_command(struct sub_device_buffer *device_buffer, struct pando_command *command_body);

/*******************************************************
 * Description: Get event from device buffer directly.
 * param
    in:
        device_buffer: buffer contains device package.
    out:
        event_body: include event number, priority and sub device id. 
 * return: 0 if success, -1 if failed.
 *********************************************************/
struct TLVs *get_sub_device_event(struct sub_device_buffer *device_buffer, struct pando_event *event_body);

/*******************************************************
 * Description: Get data from device buffer directly, this function should be called 
    repeatedly to get all data property.
 * param
    in:
       device_buffer: buffer contains device package.
    out:
        property_body: include data property number and sub device id.
 * return: If all properties have been processed, return NULL, else return data params block.
 *********************************************************/
struct TLVs *get_sub_device_property(struct sub_device_buffer *device_buffer, struct pando_property *property_body);

/*******************************************************
 * Description: Get type of package from device buffer directly.
 * param
    in:
       device_buffer: buffer contains device package.
    out:
 * return: Type of package.
 *********************************************************/
uint16_t get_sub_device_payloadtype(struct sub_device_buffer *package);

/*******************************************************
 * Description: Delete device buffer after package has been sent to server.
 * param
    in:
        device_buffer: buffer contains device package.
    out:
 * return:
 *********************************************************/
void delete_device_package(struct sub_device_buffer *device_buffer);

/*******************************************************
 * Description:
 * param
 * return:
 *********************************************************/
void delete_params_block(struct TLVs *params_block);

/*******************************************************
 * Description: Judge the command has file to process.
 * param
    in:
        device_buffer: buffer contains device package.
 * return: 1 if command with file, else 0.
 *********************************************************/
int is_device_file_command(struct sub_device_buffer *device_buffer);

/*******************************************************
 * Description: Functions to get param from params block, all params must be decode 
    in order, it's not reentrant.
 * param
    in:
        params: params block.
 * return: param to be process.
 *********************************************************/
uint8_t     get_next_uint8(struct TLVs *params);
uint16_t    get_next_uint16(struct TLVs *params);
uint32_t    get_next_uint32(struct TLVs *params);
uint64_t    get_next_uint64(struct TLVs *params);
int8_t      get_next_int8(struct TLVs *params);
int16_t     get_next_int16(struct TLVs *params);
int32_t     get_next_int32(struct TLVs *params);
int64_t     get_next_int64(struct TLVs *params);
float       get_next_float32(struct TLVs *params);
double      get_next_float64(struct TLVs *params);
uint8_t     get_next_bool(struct TLVs *params);
void        *get_next_uri(struct TLVs *params, uint16_t *length);
void        *get_next_bytes(struct TLVs *params, uint16_t *length);

/*******************************************************
 * Description: Functions to add param into params block.
 * param
    in:
        next_value: value to be added.
    out: 
        params_block: 
 * return: 0 if success, -1 if failed.
 *********************************************************/
int add_next_param(struct TLVs *params_block, uint16_t next_type, uint16_t next_length, void *next_value);
int add_next_uint8(struct TLVs *params, uint8_t next_value);
int add_next_uint16(struct TLVs *params, uint16_t next_value);
int add_next_uint32(struct TLVs *params, uint32_t next_value);
int add_next_uint64(struct TLVs *params, uint64_t next_value);
int add_next_int8(struct TLVs *params, int8_t next_value);
int add_next_int16(struct TLVs *params, int16_t next_value);
int add_next_int32(struct TLVs *params, int32_t next_value);
int add_next_int64(struct TLVs *params, int64_t next_value);
int add_next_float32(struct TLVs *params, float next_value);
int add_next_float64(struct TLVs *params, double next_value);
int add_next_bool(struct TLVs *params, uint8_t next_value);
int add_next_uri(struct TLVs *params, uint16_t length, void *next_value);
int add_next_bytes(struct TLVs *params, uint16_t length, void *next_value);

#ifdef __cplusplus
}
#endif
#endif



