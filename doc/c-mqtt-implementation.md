## 五、 C语言实现中数据包结构
本节所述内容容以MQTT为通信协议。  
根据数据包内容，将关键信息定义为payload，辅助信息定义为header
### 1. 数据包图示  
#### 服务器header   
如上文所述，服务器header必须包括以下部分
device id:设备id，由服务器分配的唯一的id  
timestamp：时间戳，由网关产生，指示数据包的生成时间  
token：用于服务器和网关校验数据包的来源，保证安全性    
payload type：表示数据包类型  
payload length：表示数据包携带的关键信息的长度   
而下图中缺少device id，payload type， payload length，原因如下  
1. mqtt协议连接时需要一个client id作为连接标识符，此时使用了device id作为client id。一个device只会创建一条mqtt连接，因此通过这条连接发送的数据包就隐含着device id。这种方式最多可以减少8字节的数据包开销  
2. mqtt协议头部中有topic字段，这个字段可用来标识payload type，现在使用c,d,e分别表示command, data, event  
3. mqtt头部中的有该协议本身的payload length字段。mqtt payload length = 服务器header + pando payload length。而下图中服务器header的长度是固定的，因此从mqtt的length中可以计算出pando payload length。这样又可以减少2字节的开销    
![接入设备-服务器](https://github.com/PandoCloud/pando-protocol/blob/master/doc/server-access.jpg)
 
#### 硬件子设备header  
![](https://github.com/PandoCloud/pando-protocol/blob/master/doc/access-hardware.jpg)  

#### Payload  
* **指令**  
![](https://github.com/PandoCloud/pando-protocol/blob/master/doc/command-payload.jpg)
* **事件**  
![](https://github.com/PandoCloud/pando-protocol/blob/master/doc/event-payload.jpg)
* **数据**
![](https://github.com/PandoCloud/pando-protocol/blob/master/doc/data-payload.jpg)
* **参数**  
![](https://github.com/PandoCloud/pando-protocol/blob/master/doc/tlv.jpg)

### 2. 数据结构
    
这部分主要描述二进制数据包的组成，及子设备数据包的结构，并简述MQTT包头中与设备相关的字段。  
* mqtt头部中与设备相关的字段  
  1. 使用device id(每个接入设备的唯一标识符，8字节长)的16进制转换成字符串作为mqtt connect的client id，用于告知服务器哪个设备上线了，并确定了接入设备与服务器之间唯一的tcp连接。  
如device id为0x0000 0A01，则client id为“A01”，前面不补0   
  2. 数据包使用数据包类型(事件、命令、数据)作为topic。  
在mqtt的topic采用"payload_type"表示，可取值“e”， “c”， “d”，不包含引号，分别表示event，command，data。  

* 二进制协议各字段的详细说明  
此处以C语言为例，详细说明各字段的意义  
  1. 首先是服务器与接入设备间的header    
    ```    
        struct mqtt_bin_header  
        {  
            uint8 flags;         /* 标志位 */  
            uint64_t timestamp;     /* 时间戳 */  
            uint8_t  token[16];     /* token */  
        };   
    ```
    * token是通过算法计算出来的校验令牌，用于防范数据包伪造，长度为16字节  
    * flag用于一些特殊的业务，如文件传输，p2p，以及一些未来可能需要扩展的业务。  
现在用到的场景是文件传输，当flag为1时，表示这个command携带了文件所在的uri，终端要下载此文件，并传输给子设备。  
其余flag暂时保留  

        | **业务**   | **FLAG** |
        | ---- | ---- |
        | 文件传输 | 1 |
  2. 接入设备与硬件层设备之间的header  
      
    现在的子设备与终端之间没有一套类似MQTT这种保证通信可靠性的协议。  
    因此在子设备通用框架设计完成前，仍然使用Pando早期的二进制数据结构，其中flags等字段未与mqtt_bin_header的统一长度，预计下个版本会进行统一。 
    * 二进制数据结构头部  
    ```
    struct device_header
    {
        uint8_t  magic;         /* 开始标志 (0x34) */
        uint8_t  crc;           /* 校验和 */
        uint16_t payload_type;  /* 载荷类型 */
        uint16_t payload_len;   /* 载荷长度 */
        uint16_t flags;         /* 标志位,作用与mqtt header中的flag一致
                                 ，长度会在下一版协议中更新 */
        uint32_t frame_seq;     /* 帧序列 */
    };
    ```
    * 子设备编号(subdevice_id)的控制  
一个终端会有多个子设备，且具有动态扩展更多子设备的能力。对于那些动态加入的子设备，是无法知道自身的id的。子设备也没有必要知道自身的id  
终端知道子设备的变化，包括动态添加的子设备。因此，子设备id由终端来管控，当终端收到子设备上报的数据包后，终端会打上子设备id，再转换成mqtt数据包，发往服务器    

  3. payload部分  
服务器、接入设备、硬件层设备，这三个层次间通信的payload没有区别，使用了统一的格式。  
目的是为了减少接入设备的开销，因为接入设备作用是为了转发硬件层的数据，而不牵涉具体业务逻辑。
接入设备只需要重新组建header部分，就可将数据进行转发。  
    ```
    struct pando_command
    {
        uint16_t sub_device_id   /* 子设备ID */
        uint16_t command_num     /* 命令编号 */
        uint16_t priority        /* 优先级 */
        TLVs     params[1]       /* 参数,必须有一个参数区，其count可以为0 */
    };

    struct pando_event
    {
        uint16_t sub_device_id   /* 子设备ID */
        uint16_t event_num       /* 事件编号 */
        uint16_t priority        /* 优先级 */
        TLVs     params[1]       /* 参数,必须有一个参数区，其count可以为0 */
    };

    struct pando_data
    {
        struct pando_property properties[1]; /* 属性数组,至少有一个属性 */
    };
    ```

    其中属性pando_property 的定义为:
    ```
    struct pando_property
    {
        uint16_t sub_device_id   /* 子设备ID */
        uint16_t property_num     /* 属性编号 */
        TLVs     params[1]       /* 参数,必须有一个参数区，其count可以为0 */
    };
    ```
    * TLVs与tlv的定义  
tlv是type、length、value的缩写，对一个信息参数进行描述。  
TLVs是一个或多个tlv的集合，其成员count表示包含了多少个tlv。
    TLVs
    ```
    struct TLVs
    {
        uint16_t count;          /*表示紧接着有多少个tlv元素*/
        tlv      tlv_params[0]   /*tlv 信息区*/
    };
    ```
    tlv只有当信息类型为bytes，uri等类型时，需要length字段，结构如下
    ```
    struct tlv
    {
        uint16_t type
        uint16_t length
        uint8_t  value[length]
    };
    ```
    如果类型为int，double等固定长度的类型时，tlv结构就简化为  
    ```
struct tlv
{
    uint16_t type
    uint8_t  value[length]
}
    ```
    类型及其枚举如下,终端与服务器要保持一致  
    ```
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
    ```

## 六、协议的实现  
MQTT协议为例，介绍从接入流程开始，如何完成连接和数据的交互。     
本节以两个数据包为例，演示数据包各字段的含义，尤其是tlv与TLVs的概念。  