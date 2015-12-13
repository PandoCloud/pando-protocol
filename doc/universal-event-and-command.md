>该文档定义pando平台通用的指令和事件；

>eventid和operation为60000-65535之间的事件为平台保留事件和平台保留指令

### 平台通用事件
* 设备上线事件

```
eventid : 65535
subdeviceid : 0
eventparams : [设备心跳间隔（uint32,单位秒）]
```
* 命令送达反馈

```
eventid : 65534
subdeviceid : 0
eventparams: [命令序列号（uint64）, 命令执行结果（int32）， 结果描述（[]byte）] 
```
* 故障事件

```
eventid : 65533
subdeviceid : id
eventparams: [故障类型，故障描述]
```
* 设备下线事件

```
eventid : 65532
subdeviceid : 0
eventparams: []
```

### 平台通用指令
* restart软重启

```
subdeviceid: 0
operation: 65535
priority: 根据实际情况
commandparams: [](空)
```
* 事件送达反馈

```
subdeviceid: 0
operation: 65534
priority: 最高值
commandparams: [事件sequence]
```
* 配置更新

```
subdeviceid: 0
operation: 65533
priority: 最低值
commandparams: [配置文件url]
```
* reset恢复原厂设置

```
subdeviceid: 0
operation: 65532
priority: 最低值
commandparams: [](空)
```
* 时钟同步

```
subdeviceid: 0
operation: 65531
priority: 最低值
commandparams: [UNIX时间戳（毫秒级）]
```
* 通知（可交互终端）

```
subdeviceid: 0
operation: 65530
priority: 最高值
commandparams: [schema(bytes), message(bytes)]
```
* OTA升级

```
subdeviceid: 0
operation: 65529
priority: 最低值
commandparams: [程序包url, 升级文件md5]

```
* 实时数据查询

```
subdeviceid: 1 (如果只查询某一个特定子设备则为该子设备id，如果是查询所有子设备数据，则为65535)
operation: 65528
priority: 最高
commandparams: []

```

* 通过url下载文件

```
subdeviceid: 0 
operation: 65527
priority: 根据实际情况
commandparams: []

```