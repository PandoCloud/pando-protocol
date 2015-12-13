## device api 设备接口


> 说明：设备相关api由设备代理(网关)向服务器发起
> 设备在第一次成功联网后第一时间向服务器注册自己的信息
> 设备通过设备登陆接口获取设备token

#### 设备注册
*请求方式*
```
POST
```
*请求URL*
```
/v1/devices/registration
```
*参数*
```
无
```
*请求头*
```
无
```
*请求内容*
```
{
  // 产品id及key,平台分配给厂商
  "product_key": "3d6few3ac31w7a6d3f",
  // 设备序列号,设备唯一硬件标识
  "device_code": "4d3e2a5d3fff",
  // 设备类型,如wifi, id见说明
  "device_type": 1,
  // 设备硬件型号,如“esp8266”, "MI3"
  "device_module": "", 
  // 可选,ios设备token,ios设备必填,其余类型设备不需要填写
  "ios_device_token": "45deeeee", 
  // 网关版本
  "version": "0.1.0"
}
```
*返回内容*
```
{
  // 返回码
  "code": 0,
  // 正确或错误信息
  "message": "", 
  // 如果成功,将返回设备id及设备密码
  "data": { 
    // 设备id
    "device_id": 12324, 
    // 设备密码
    "device_secret": "3d6few3ac31w7a6d3f", 
    // 设备激活码,用来绑定设备
    "device_key": "34ffffffff",
    // 设备标识符
    "device_identifier": "64-64-fe4efe"
  }
}
```
>  说明

设备类型id

|  id  | 设备类型 |
|------|---------|
|  1   | wifi设备  |
|  2   | ios手机   |
|  3   | android手机| 

#### 设备登录
*请求方式*
```
POST
```
*请求URL*
```
/v1/devices/authentication
```
*参数*
```
无
```
*请求头*
```
无
```
*请求内容*
```
{
  // 设备id
  "device_id": 123,
  // 设备密码
  "device_secret": "fsfwefewf23r2r32r23rfs",
  // 协议类型,不填写表示默认二进制协议
  "protocol": "mqtt"
}
```
*返回内容*
```
{
  //返回码
  "code": 0, 
  //正确或错误信息
  "message": "", 
  "data": { 
    // 设备token
    "access_token": "3sffefefefefsf", 
    // 接入服务器地址+端口
    "access_addr": "202.114.0.242:8080", 
    // 当前数据序列号
    "data_sequence": 1111,
    // 当前事件序列号
    "event_sequence": 111
  }
}
```


#### 获取局域网设备访问授权
*请求方式*
```
GET
```
*请求URL*
```
/v1/lan/devices/{identifier}/authority
```
*参数*
```
无
```
*请求头*
```
Access-Token:3eeeeeee...   
```
//自己的token

*请求内容*
```
无
```
*返回内容*
```
{
  //返回码
  "code": 0, 
  //正确或错误信息
  "message": "", 
  "data": { 
    // 目标设备的token
    "access_token": "3sffefefefefsf...", 
    "device_config": {
      "objects": [
        {
            "id": 3, // 对象id
            "no": 1, // 对象编号
            "label": "atmosphere",  // 对象别名
            "part": 1,  // 所述部件（子设备id）
            "status": [  // 对象所含状态列表
                {
                    "value_type": 7, // 状态值类型，见说明
                    "name": "temperature" // 状态名称
                },
                {
                    "value_type": 7,
                    "name": "humidity"
                },
                {
                    "value_type": 7,
                    "name": "air-quality"
                }
            ]
        }
      ],
      "commands": [
        {
          "no": 1,
          "name": "switch",  // 命令名
          "part": 1,
          "priority": 0, // 优先级
          "params": [{  // 命令参数列表
            {
              "value_type": 7,
              "name": "temperature"
            },
            {
              "value_type": 7,
              "name": "humidity"
            },
            {
              "value_type": 7,
              "name": "air-quality"
            }
          }]
        }
      ],
      "events": [
        {
          "no": 1,
          "name": "alert", // 事件名
          "part": 1,
          "priority": 0, // 优先级
          "params": [{  // 事件参数列表
            {
              "value_type": 7,
              "name": "temperature"
            },
            {
              "value_type": 7,
              "name": "humidity"
            },
            {
              "value_type": 7,
              "name": "air-quality"
            }
          }]
        }
      ]
    }
  }
}
```
*说明*

value_type id表

|  id  | 值类型 |
|------|---------|
|  1   | FLOAT64  |
|  2   | FLOAT32   |
|  3   | INT8| 
|  4   | INT16  |
|  5   | INT32   |
|  6   | INT64| 
|  7   | UINT8  |
|  8   | UINT16   |
|  9   | UINT32| 
|  10   | UINT64  |
|  11   | BYTES   |
|  12   | URI| 
|  13   | BOOL| 