#ifndef JDY_H
#define JDY_H
#include "main.h"
#include "stdio.h"
#include "string.h"

#include "bsp_ble.h"
#include "bsp_dbg.h"

// 指令集合
typedef struct __JDY_CMD_TypeDef
{
    char* OK;
    char* add;
    char* equ;
    char* over;
    char* AT;
    char* RESET;
    char* VERSION;
    char* LADDR;
    char* NAME;
    char* PIN;
    char* TYPE;
    char* BAUD;
    char* DISC;
    char* STAT;
    char* STARTEN;
    char* ADVIN;
    char* POWR;
    char* ROLE;
    char* SLEEP;
    char* DEFAULT;
    char* INQ;
    char* STOP;
    char* CONN;
    char* CONA;
    char* BAND;
    char* CLRBAND;
    char* SRBAND;
    char* MESH;
    char* KEY;
    char* NETID;
    char* MADDR;
    char* MCLSS;
    char* FRIEND;
    char* CLRFRIEND;
    char* FRTYPE;
    char* IBUUID;
    char* MAJOR;
    char* MINOR;
    char* IBSING;
    char* ENLOG;
    char* DATA;
    char* CUIO;
    char* TGIO;
    char* FUNC;
    char* SUBTYPE;
    char* LEARN;
    char* LEAVAL;
    char* DEVCLSS;
    char* KLABEL;
    char* KLTIME;
    char* KLRSSI;
    char* ALED;
    char* RSSVAL;
    char* RSSI;
    char* RITYPE;
    char* PAISTART;
    char* GSEND;
    char* RSSVLS;
}_JDY_CMD_TypeDef;


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
}_ROLE;

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

// 开机睡眠或唤醒 默认是1 这个测试发现jdy-25m不反应，暂时不用
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
    noPIN,
    NeedPIN_without_Banding,
    NeedPIN_with_Banding
}_TYPE;


// 组网设备类型 默认是0（路由器） 1(终端设备)
typedef enum __MCLSS
{
    Router,     //路由器（支持中继）
    Terminal    //终端设备（无中继）
}_MCLSS;

// 串口状态机
typedef enum __STATE
{
    JDY_idle,
    JDY_listening,
    JDY_datahandling
}_STATE;

// JDY库的状态反馈
typedef enum _JDY_StateDef
{
    JDY_OK,
    JDY_ERR,
    JDY_TMOUT,
    JDY_Waitting
}JDY_StateDef;

// JDY25设备
typedef struct _JDY_DEVICE
{
    /******* 特殊用途 ********/
    _JDY_CMD_TypeDef  cmd;
    char        isInited;
    char        dbgflag;
    _STATE      state;

    /******* enum 类型 ********/
    _ENLOG      ENLOG;      //串口输出状态    
    _TYPE       TYPE;       //是否需要PIN
    _BAUD       BAUD;       //波特率        // 默认 B9600
    _MCLSS      MCLSS;      //组网设备类型
    _ROLE       ROLE;       //工作模式设置
    _POWR       POWR;       //发射功率
    _STAT       STAT;       //读取连接状态

    /******* char* 类型 ********/
    // 留出一位用于 字符串 型末尾的 '\0'
    char        LADDR[49];  //MAC地址
    char    	MADDR[5];   //短地址
    char    	NETID[5];   //组网ID        // "1189"  
    char        NAME[19];   //广播名称      // "JDY-25M"
    char        PIN[7];     //连接PIN       // "123456"
    
}JDY_Def;




#define JAY_MAX_AT_SIZE       ((uint16_t)49U)
#define JAY_MAX_Para_SIZE     ((uint16_t)49U)
#define JAY_MAX_Recv_SIZE     ble_MAX_datarev_buffsize
#define JDY_Listening_timeout   ((uint32_t)3000U)   // 最大超时1000ms


extern JDY_Def JDY;

JDY_StateDef __jdy_AT_OKcheck(void);
JDY_StateDef __jdy_AT_CMD(const char* cmdin, const char* param);
JDY_StateDef __jdy_AT_Query(const char* cmdin, char* param, size_t paramlen);
JDY_StateDef __jdy_AT_Query_with_enum(const char* cmdin, int* num);
JDY_StateDef __jdy_AT_Query_with_char(const char* cmdin, char* c, size_t paramlen);
JDY_StateDef __jdy_AT_Query_Selectable(const  char* cmdin);
void __jdy_StrCut(char* str, size_t len);

JDY_StateDef jdy_SNED_and_EQUERY(const char* cmdin, const char* param);
JDY_StateDef jdy_SEND_CMD(const char *cmdin, const char *param);
JDY_StateDef jdy_SEND_MESH(const char* data);
JDY_StateDef jdy_EQUERY(const char* cmdin);

void jdy_init(void);
void jdy_loop(void);
void jdy_it_ble_handle(void);
void jdy_it_dbg_handle(void);



// void _jdy_txData_cat(const char* cmdin);

#endif
