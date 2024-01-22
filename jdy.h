#ifndef JDY_H
#define JDY_H
#include "stdint.h"
/**
 使用案例
 void main(){
    JDY_DEVICE JDY_25M;
    JDY_25M.BAUD = B115200;
 }
*/


// 工作模式 默认是5
typedef enum __ROLE
{
    Slave,
    Master,
    BLE_radio,
    iBeacon,
    iBeacon_pogopin,
    MESH,
    MESH_net,
    multiSlave,
    multiMaster,
    KeyTag
} _ROLE;

// 波特率 默认是4  (9600bps)
typedef enum __BAUD
{
    B2400 = 2U,
    B4800 = 3U,
    B9600 = 4U,
    B19200 = 5U,
    B38400 = 6U,
    B57600 = 7U,
    B115200 = 8U
}_BAUD;

// 发射功率 默认是4
typedef enum __POWR
{
    n_15dbm,
    n_0dbm,
    p_2dbm,
    p_3dbm,
    p_4dbm
}_POWR;

// 查询--连接状态
// MESH状态，只有在网络内有中心机的情况下才有效，
// 特别说明组网数量多时不建议用户打开中心机会造成网络负荷
typedef enum __STAT
{
    NotConnect,
    Connected,
    Netted,
    ConnectedAndNetted
}_STAT;

// 开机睡眠或唤醒 默认是1
typedef enum __STARTEN
{
    sleep,
    weakup
}_STARTEN;

// 串口输出状态 默认是1
typedef enum __ENLOG
{
    off,
    on
}_ENLOG;

// 是否打开密码连接方式 默认是0
typedef enum __TYPE
{
    None,
    NeedPIN_without_Banding,
    NeedPIN_with_Banding
}_TYPE;


// 组网设备类型 默认是0（路由器） 1(终端设备)
typedef enum __MCLSS
{
    Router,     //路由器（支持中继）
    Terminal    //终端设备（无中继）
}_MCLSS;


// JDY25设备
typedef struct _JDY_DEVICE
{
    uint8_t* LADDR;          //MAC地址
    uint16_t MADDR;          //短地址
    uint16_t NETID = (uint16_t) 1189U;       //组网ID 默认 1189
    uint8_t  NAME[18];       //广播名称最长 18Byte 默认 JDY-25M 
    uint8_t* PIN = (uint8_t*) "1234"; //连接PIN 默认 1234
    _TYPE       TYPE;        //是否需要PIN
    _ENLOG      ENLOG;       //串口输出状态    
    _BAUD       BAUD = B9600;//波特率  
    _STARTEN    STARTEN;     //开机睡眠设置
    _STAT       STAT;        //读取连接状态
    _MCLSS      MCLSS;       //组网设备类型
    _ROLE       ROLE;        //工作模式设置
    _POWR       POWR;        //发射功率
}JDY_DEVICE;



#endif
