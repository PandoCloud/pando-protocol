//  Copyright (c) 2014 Pando. All rights reserved.
//  PtotoBuf:   ProtocolBuffer.h
//
//  Create By TangWenhan On 14/12/24.

#include "pando_protocol.h"

static int FUNCTION_ATTRIBUTE check_pdbin_header(struct mqtt_bin_header *bin_header);
static int FUNCTION_ATTRIBUTE init_device_header(struct device_header *header, struct mqtt_bin_header *bin_header,
    uint16_t payload_type, uint16_t payload_len);
static int FUNCTION_ATTRIBUTE init_pdbin_header(struct mqtt_bin_header *bin_header, struct device_header *header);


typedef long long unsigned int llui;

struct protocol_base protocol_tool_base_params;
static int file_cmd_sequence;	//也许会改成数组

struct pando_buffer * FUNCTION_ATTRIBUTE pando_buffer_create(int length, int offset)
{
	struct pando_buffer *protocol_buffer = (struct pando_buffer *)pd_malloc(sizeof(struct pando_buffer));

	if (protocol_buffer == NULL)
	{
		return NULL;
	}

	protocol_buffer->buff_len = length;
	protocol_buffer->offset = offset;
	protocol_buffer->buffer = (uint8_t *)pd_malloc(protocol_buffer->buff_len);

	if (protocol_buffer->buffer == NULL)
	{
		pd_free(protocol_buffer);
		return NULL;
	}

	pd_memset(protocol_buffer->buffer, 0, protocol_buffer->buff_len);
	return protocol_buffer;
}

void FUNCTION_ATTRIBUTE pando_buffer_delete(struct pando_buffer *pdbuf)
{
	if (pdbuf == NULL)
	{
		return;
	}
	if (pdbuf->buffer != NULL)
	{
	pd_free(pdbuf->buffer);
	}
	pd_free(pdbuf);
}


int FUNCTION_ATTRIBUTE pando_protocol_decode(struct pando_buffer *pdbuf, uint16_t payload_type)
{
    //get valid data position of buffer
	uint8_t *position = pdbuf->buffer + pdbuf->offset;
	uint8_t *pdbuf_end = pdbuf->buffer + pdbuf->buff_len;
	struct device_header sub_device_header;
	struct mqtt_bin_header *m_header = (struct mqtt_bin_header *)position;

    //check token
    if (check_pdbin_header(m_header))
	{
		return -1;
	}

    position += GATE_HEADER_LEN;    
	if (position > pdbuf_end)
	{
		pd_printf("Incorrect decode buffer length.\n");
		return -1;
	}
	
	init_device_header(&sub_device_header, m_header, payload_type, 
        pdbuf->buff_len - pdbuf->offset);

    //point to sub device packet header in the buffer
	position -= DEV_HEADER_LEN;	
	pd_memcpy(position, &sub_device_header, DEV_HEADER_LEN);
	
	//now from offset to the end of buffer, is sub device packet to send
	pdbuf->offset = pdbuf->offset + GATE_HEADER_LEN - DEV_HEADER_LEN;
	return 0;
}

int FUNCTION_ATTRIBUTE pando_protocol_encode(struct pando_buffer *pdbuf, uint16_t *payload_type)
{
	uint8_t *position = pdbuf->buffer + pdbuf->offset;
	uint8_t *buffer_end = pdbuf->buffer + pdbuf->buff_len;
	struct mqtt_bin_header m_header;
	struct device_header sub_device_header;
	struct device_header *header = (struct device_header*)position;
    
    if ((position += DEV_HEADER_LEN) > buffer_end)
    {
        pd_printf("Incorrect encode buffer length.\n");
        return -1;
    }

    pd_memcpy(payload_type, &(header->payload_type), sizeof(header->payload_type));
    *payload_type = net16_to_host(*payload_type);
    pd_memcpy(&(sub_device_header.flags), &(header->flags), sizeof(header->flags));
	sub_device_header.flags = net16_to_host(sub_device_header.flags);	
	
	if (init_pdbin_header(&m_header, &sub_device_header))
	{
		pd_printf("Init pdbin header failed.\n");
		return -1;
	}    
    
	
	position -= GATE_HEADER_LEN;
	pd_memcpy(position, &m_header, GATE_HEADER_LEN);
	pdbuf->offset = pdbuf->offset + DEV_HEADER_LEN - GATE_HEADER_LEN;
	return 0;
}

int FUNCTION_ATTRIBUTE check_pdbin_header(struct mqtt_bin_header *bin_header)
{
	if (pd_memcmp(bin_header->token, protocol_tool_base_params.token, sizeof(protocol_tool_base_params.token)))
	{
		pd_printf("Token error.\n");
		return -1;
	}

	return 0;
}

int FUNCTION_ATTRIBUTE init_device_header(struct device_header *header, struct mqtt_bin_header *bin_header,
    uint16_t payload_type, uint16_t payload_len)
{
	protocol_tool_base_params.sub_device_cmd_seq++;
	header->frame_seq = host32_to_net(protocol_tool_base_params.sub_device_cmd_seq);
	header->magic = MAGIC_HEAD_SUB_DEVICE;
	header->crc = 0x46;	
	header->flags = host16_to_net(bin_header->flags);
	header->payload_len = host16_to_net(payload_len);
	header->payload_type = host16_to_net(payload_type);

	return 0;
}

int FUNCTION_ATTRIBUTE init_pdbin_header(struct mqtt_bin_header *bin_header, struct device_header *header)
{
	bin_header->flags = host16_to_net(header->flags);
	bin_header->timestamp = host64_to_net((uint64_t)pd_get_timestamp());
	pd_memcpy(bin_header->token, protocol_tool_base_params.token, 
                sizeof(protocol_tool_base_params.token));

	return 0;
}

int FUNCTION_ATTRIBUTE pando_protocol_init(struct protocol_base init_params)
{

	if ((struct protocol_base *)pd_memcpy(&protocol_tool_base_params, &init_params, sizeof(init_params)) == NULL)
	{
		pd_printf("pando_protocol_init failed.\n");
		return -1;
	}
	return 0;
}



int FUNCTION_ATTRIBUTE pando_protocol_get_sub_device_id(struct pando_buffer *buf, uint16_t *sub_device_id)
{
    uint8_t *pos = buf->buffer + GATE_HEADER_LEN + buf->offset;
    
	if (pos == NULL)
	{
		pd_printf("subdevice null\n");
		return -1;
	}
	else
	{
        pd_memcpy(sub_device_id, pos, sizeof(*sub_device_id));
        *sub_device_id = net16_to_host(*sub_device_id);
		return 0;
	}

}

int FUNCTION_ATTRIBUTE pando_protocol_set_sub_device_id(struct pando_buffer *buf, uint16_t sub_device_id)
{
    uint8_t *pos = buf->buffer + GATE_HEADER_LEN + buf->offset;
    
	if (pos == NULL)
	{
		pd_printf("subdevice null\n");
		return -1;
	}
	else
	{
		pd_printf("set sub_device_id %d\n", sub_device_id);
        sub_device_id = host16_to_net(sub_device_id);
        pd_memcpy(pos, &sub_device_id, sizeof(sub_device_id));
		return 0;
	}

#if 0
    uint8_t *pos = NULL;
    int rc = 0;
    uint16_t tmp_devid = sub_device_id;
    pos = buf->buffer + GATE_HEADER_LEN + buf->offset;
    
	if (buf == NULL || sub_device_id > 127*128)
	{
		pd_printf("subdevice null, subdevice id :%d\n", sub_device_id);
		return -1;
	}
	else
	{
		pd_printf("sub_device_id %d\n", sub_device_id);

        do
    	{
    		char d = tmp_devid % 128;
    		tmp_devid /= 128;
    		/* if there are more digits to encode, set the top bit of this digit */
    		if (tmp_devid > 0)
    			d |= 0x80;
    		pos[rc++] = d;
    	} while (tmp_devid > 0);

        /* sub device using 2 bytes space.*/
        if (rc == 1)
        {
            for(;pos >= buf->buffer; pos--)
            {
               *(pos + 1) = *pos;
            }

            buf->offset += 1;
        }
        
		return 0;
	}
#endif
}


uint64_t FUNCTION_ATTRIBUTE pando_protocol_get_cmd_sequence()
{
	return protocol_tool_base_params.command_sequence;
}

void FUNCTION_ATTRIBUTE save_file_sequence()
{
	file_cmd_sequence = protocol_tool_base_params.sub_device_cmd_seq;
}

int FUNCTION_ATTRIBUTE is_file_feedback(uint32_t sequence)
{
	return (sequence == file_cmd_sequence)?1:0;
}

#if 0
char *FUNCTION_ATTRIBUTE pando_protocol_get_uri(struct pando_buffer *pdbuf)
{
	if (pdbuf == NULL)
	{
		return NULL;
	}

	struct sub_device_buffer device_buffer;
	device_buffer.buffer_length = pdbuf->buff_len - pdbuf->offset;
	device_buffer.buffer = pdbuf->buffer + pdbuf->offset;

	struct pando_command cmd_body;
	struct TLV *cmd_param = get_sub_device_command(&device_buffer, &cmd_body);
	
	int i = 0;
	uint16_t tlv_type, tlv_length;
	for (i = 0; i < cmd_body.params->count; i++)
	{
		pd_memcpy(&tlv_type, &(cmd_param->type), sizeof(tlv_type));
		tlv_type = net16_to_host(tlv_type);

		pd_memcpy(&tlv_length, &(cmd_param->length), sizeof(tlv_length));
		tlv_length = net16_to_host(tlv_length);

		if (tlv_type == TLV_TYPE_URI)
		{
			return (char *)cmd_param->value;
		}

		cmd_param = (struct TLV *)((uint8_t *)cmd_param + sizeof(struct TLV) + tlv_length);
	}

	return NULL;
}
#endif


//pdbuf points to the buffer contains command from server ,it's big endian.
uint16_t FUNCTION_ATTRIBUTE pando_protocol_get_payload_type(struct pando_buffer *pdbuf)
{
	if (pdbuf == NULL)
	{
		return -1;
	}

	struct device_header *gateway_header = (struct device_header *)(pdbuf->buffer + pdbuf->offset);
	return net16_to_host(gateway_header->payload_type);
}

int FUNCTION_ATTRIBUTE is_pando_file_command(struct pando_buffer *pdbuf)
{
    struct mqtt_bin_header *header = (struct mqtt_bin_header *)(pdbuf->buffer + pdbuf->offset);

    if (header->flags == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t FUNCTION_ATTRIBUTE *pando_get_package_begin(struct pando_buffer *buf)
{
    return (buf->buffer + buf->offset);
}
uint16_t FUNCTION_ATTRIBUTE pando_get_package_length(struct pando_buffer *buf)
{
    return (buf->buff_len - buf->offset);
}



