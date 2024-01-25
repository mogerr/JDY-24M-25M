#include "jdy.h"
#include <stdlib.h>

JDY_Def JDY = {
    .cmd = {
        .OK = "OK",
        .add = "+",
        .equ = "=",
        .over = "\r\n",
        .AT = "AT",
        .RESET = "RESET",
        .VERSION = "VERSION",
        .LADDR = "LADDR",
        .NAME = "NAME",
        .PIN = "PIN",
        .TYPE = "TYPE",
        .BAUD = "BAUD",
        .DISC = "DISC",
        .STAT = "STAT",
        .STARTEN = "STARTEN",
        .ADVIN = "ADVIN",
        .POWR = "POWR",
        .ROLE = "ROLE",
        .SLEEP = "SLEEP",
        .DEFAULT = "DEFAULT",
        .INQ = "INQ",
        .STOP = "STOP",
        .CONN = "CONN",
        .CONA = "CONA",
        .BAND = "BAND",
        .CLRBAND = "CLRBAND",
        .SRBAND = "SRBAND",
        .MESH = "MESH",
        .KEY = "KEY",
        .NETID = "NETID",
        .MADDR = "MADDR",
        .MCLSS = "MCLSS",
        .FRIEND = "FRIEND",
        .CLRFRIEND = "CLRFRIEND",
        .FRTYPE = "FRTYPE",
        .IBUUID = "IBUUID",
        .MAJOR = "MAJOR",
        .MINOR = "MINOR",
        .IBSING = "IBSING",
        .ENLOG = "ENLOG",
        .DATA = "DATA",
        .CUIO = "CUIO",
        .TGIO = "TGIO",
        .FUNC = "FUNC",
        .SUBTYPE = "SUBTYPE",
        .LEARN = "LEARN",
        .LEAVAL = "LEARN",
        .DEVCLSS = "DEVCLSS",
        .KLABEL = "KLABEL",
        .KLTIME = "KLTIME",
        .KLRSSI = "KLRSSI",
        .ALED = "ALED",
        .RSSVAL = "RSSVAL",
        .RSSI = "RSSI",
        .RITYPE = "RITYPE",
        .PAISTART = "PAISTART",
        .GSEND = "GSEND",
        .RSSVLS = "RSSVLS",
    },
    .LADDR = {0U},
    .MADDR = {0U},
    .NETID = {0U},
    .NAME = {0U},
    .PIN = {0U},
    .isInited = 0U,
    .dbgflag = 0U
};

char  JDY_recvDATA[JAY_MAX_Recv_SIZE] = { 0U };
char  JDY_tx_AT_DATA[JAY_MAX_AT_SIZE] = { 0U };

char JDY_dbg_strbuf[JAY_MAX_AT_SIZE] = { 0U };

JDY_StateDef test = JDY_ERR;

/**
 * @brief 中断处理函数，放在ble串口接收完成中断中执行
 */
void jdy_it_ble_handle(void)
{
    if (JDY.state == JDY_listening)
    {
        memset((char*) JDY_recvDATA, 0, strlen(JDY_recvDATA));
        strcpy((char*) JDY_recvDATA, (const char*) ble_datarev_buff);

        // 标志位
        JDY.state = JDY_datahandling;
    }
    return;
}
/**
 * @brief 中断处理函数，放在dbg串口接收完成中断中执行
 */
void jdy_it_dbg_handle(void)
{
    if (JDY.dbgflag == 0U)
    {
        if (strlen((char*) dbg_datarev_buff) < sizeof(JDY_dbg_strbuf))
        {
            strcpy(JDY_dbg_strbuf, (char*) dbg_datarev_buff);

            // 标志位
            JDY.dbgflag = 1U;
        }
    }
    return;
}


/**
 * @brief 仅发送 AT 设置指令
 *  ----- 发送 AT+cmdin+param+结束符
 *  ----- 一定接收到 OK 才算成功
 * @param cmdin 指令
 * @param Param 参数不能为 空 或者 "\0" ，必须有值
 * @return JDY_StateDef
 */
JDY_StateDef __jdy_AT_CMD(const char* cmdin, const char* param)
{
    uint32_t k = 0U;

    if (JDY.state != JDY_idle)
    {
        return JDY_Waitting;
    }

    // 开启蓝牙串口接收
    ble_recv_start_DMA();

    // 状态机
    JDY.state = JDY_listening;

    // 数据预处理   
    strcpy(JDY_tx_AT_DATA, JDY.cmd.AT); // "AT"    
    if (cmdin != JDY.cmd.AT)
    {
        strcat(JDY_tx_AT_DATA, JDY.cmd.add);// "+"
        strcat(JDY_tx_AT_DATA, cmdin);// "cmdin"
        if (param != '\0' || param != "\0") {
            strcat(JDY_tx_AT_DATA, param);// "param"
        }
    }
    strcat(JDY_tx_AT_DATA, JDY.cmd.over); // "\r\n"

    // 蓝牙串口发送 
    // dbg_send((uint8_t*)JDY_tx_AT_DATA);
    ble_send((uint8_t*) JDY_tx_AT_DATA);

    k = HAL_GetTick();

    // blocking 方式查询状态 or 超时
    while (JDY.state == JDY_listening)
    {
        // 没接收到数据
        if ((HAL_GetTick() - k) > JDY_Listening_timeout)
        {
            JDY.state = JDY_idle;
            return JDY_TMOUT;
        }
    }
    // 接收到数据了
    if (JDY.state == JDY_datahandling)
    {
        // 和 "OK" 作比较
        if ((strcmp((const char*) JDY_recvDATA, (const char*) "OK\r\n") == 0)
            || (strcmp((const char*) JDY_recvDATA, (const char*) "+OK\r\n") == 0))
        {
            JDY.state = JDY_idle;
            return JDY_OK;
        }
    }
    JDY.state = JDY_idle;
    return JDY_ERR;
}

JDY_StateDef __jdy_AT_OKcheck(void)
{
    return __jdy_AT_CMD(JDY.cmd.AT, '\0');
}


/**
 * @brief AT 查询指令
 * @param cmdin 指令
 * @param param 返回的参数, char 型，请根据需要的类型，将其转换后再使用。
 *              (有bug，可能会溢出，使用时确认好字符串长度，或者使用__jdy_AT_Query_return_char()函数)
 * @param paramlen param 的长度， paramlen = sizeof() 包含了结尾的'\0'
 * @return JDY_StateDef
 */
JDY_StateDef __jdy_AT_Query(const char* cmdin, char* param, size_t paramlen)
{
    uint32_t k = 0U;
    char  cmdBuff[JAY_MAX_AT_SIZE] = { 0U };
    char  cmdBuff2[JAY_MAX_AT_SIZE] = { 0U };
    char  paramBuff[JAY_MAX_Para_SIZE] = { 0U };    // paramlen = sizeof() 包含了结尾的'\0'
    // char  recvBuff[JAY_MAX_Recv_SIZE] = { 0U };

    if (JDY.state != JDY_idle)
    {
        return JDY_Waitting;
    }

    // paramlen = sizeof() 包含了结尾的'\0'
    if (paramlen > JAY_MAX_Para_SIZE)
    {
        return JDY_ERR;
    }

    // 开启蓝牙串口接收
    ble_recv_start_DMA();

    // 状态机
    JDY.state = JDY_listening;

    // 数据预处理   
    // strcpy有清零的效果
    strcpy(JDY_tx_AT_DATA, JDY.cmd.AT); // "AT"
    strcat(JDY_tx_AT_DATA, JDY.cmd.add);// "+"
    strcat(JDY_tx_AT_DATA, cmdin);      // "cmdin"
    strcat(JDY_tx_AT_DATA, JDY.cmd.over); // "\r\n"

    // 蓝牙串口发送 
    // dbg_send((uint8_t*) JDY_tx_AT_DATA);
    ble_send((uint8_t*) JDY_tx_AT_DATA);

    k = HAL_GetTick();

    // blocking 方式查询状态 or 超时
    while (JDY.state == JDY_listening)
    {
        // 没接收到数据
        if ((HAL_GetTick() - k) > JDY_Listening_timeout)
        {
            JDY.state = JDY_idle;
            return JDY_TMOUT;
        }
    }
    // 接收到数据了
    if (JDY.state == JDY_datahandling)
    {
        // strcpy有清零的效果
        strcpy(cmdBuff, JDY.cmd.add); //"+"
        strcat(cmdBuff, cmdin);// "cmdin"
        strcat(cmdBuff, JDY.cmd.equ);// "="
        k = strlen(cmdBuff);

        // strncpy 复制前k个字符
        strncpy(cmdBuff2, JDY_recvDATA, k); //""

        // 作比较
        if (strcmp((const char*) cmdBuff, (const char*) cmdBuff2) == 0)
        {
            // strncpy 复制第 k+1 个到第 paramlen-1 个字符 ，同时去掉了 "\r\n"
            strncpy(paramBuff, JDY_recvDATA + k, paramlen - 1);

            // paramBuff 的存在，确保了 param 不会出错
            strcpy(param, paramBuff);

            JDY.state = JDY_idle;
            return JDY_OK;
        }
    }
    JDY.state = JDY_idle;
    return JDY_ERR;
}


/**
 * @brief AT查询指令
 * @param cmdin 指令，例如传入 JDY.cmd.POWR
 * @param num 参数，被修改的值 例如 JDY.POWR
 * @return JDY_StateDef
 */
JDY_StateDef __jdy_AT_Query_with_enum(const char* cmdin, int* num)
{
    char    parabuf[JAY_MAX_Para_SIZE] = { 0U };
    int  numbuf = 0U;
    JDY_StateDef state = JDY_ERR;
    // if (__jdy_AT_OKcheck() == JDY_OK)
    // {
        // paramlen = sizeof() 包含了结尾的'\0'
    state = __jdy_AT_Query(cmdin, parabuf, (size_t) 2);
    numbuf = ((int) parabuf[0] - 0x30);   // ASCII   0~9   0x30~0x39
    *num = numbuf;
    // }
    return state;
}


/**
 * @brief AT查询指令
 * @param cmdin 指令，例如传入 JDY.cmd.NETID
 * @param c 参数，被修改的值 例如 JDY.NETID
 * @param paramlen paramlen = sizeof(char[]) 限制c的长度 包含了结尾的'\0'
 * @return JDY_StateDef
 */
JDY_StateDef __jdy_AT_Query_with_char(const char* cmdin, char* c, size_t paramlen)
{
    JDY_StateDef state = JDY_ERR;
    state = __jdy_AT_Query(cmdin, c, paramlen);
    return state;
}


/**
 * @brief 查询（更新JDY结构体）
 * @param cmdin 指令，例如传入 JDY.POWR
 * @return
 */
JDY_StateDef __jdy_AT_Query_Selectable(const char* cmdin)
{
    int  JDY_enumbuf = 0U;
    char JDY_charbuf[JAY_MAX_Para_SIZE] = { 0U };
    JDY_StateDef  state = JDY_ERR;

    // 结构体成员，只能使用 if-else 不能用 switch-case 
    /******* enum 类型 ********/
    // POWR
    if (cmdin == JDY.cmd.POWR)
    {
        state = __jdy_AT_Query_with_enum(JDY.cmd.POWR, &JDY_enumbuf);
        JDY.POWR = (_POWR) JDY_enumbuf;
    }
    // TYPE
    else if (cmdin == JDY.cmd.TYPE)
    {
        state = __jdy_AT_Query_with_enum(JDY.cmd.TYPE, &JDY_enumbuf);
        JDY.TYPE = (_TYPE) JDY_enumbuf;
    }
    // ENLOG
    else if ((cmdin == JDY.cmd.ENLOG))
    {
        state = __jdy_AT_Query_with_enum(JDY.cmd.ENLOG, &JDY_enumbuf);
        JDY.ENLOG = (_ENLOG) JDY_enumbuf;
    }
    // BAUD
    else if ((cmdin == JDY.cmd.BAUD))
    {
        state = __jdy_AT_Query_with_enum(JDY.cmd.BAUD, &JDY_enumbuf);
        JDY.BAUD = (_BAUD) JDY_enumbuf;
    }
    // STAT
    else if ((cmdin == JDY.cmd.STAT))
    {
        state = __jdy_AT_Query_with_enum(JDY.cmd.STAT, &JDY_enumbuf);
        JDY.STAT = (_STAT) JDY_enumbuf;
    }
    // MCLSS
    else if ((cmdin == JDY.cmd.MCLSS))
    {
        state = __jdy_AT_Query_with_enum(JDY.cmd.MCLSS, &JDY_enumbuf);
        JDY.MCLSS = (_MCLSS) JDY_enumbuf;
    }
    // ROLE
    else if ((cmdin == JDY.cmd.ROLE))
    {
        state = __jdy_AT_Query_with_enum(JDY.cmd.ROLE, &JDY_enumbuf);
        JDY.ROLE = (_ROLE) JDY_enumbuf;
    }

    /******* char* 类型 ********/
    // LADDR
    else if ((cmdin == JDY.cmd.LADDR))
    {
        state = __jdy_AT_Query_with_char(JDY.cmd.LADDR, JDY_charbuf, sizeof(JDY.LADDR));
        __jdy_StrCut(JDY_charbuf, sizeof(JDY.LADDR));
        strncpy(JDY.LADDR, JDY_charbuf, sizeof(JDY.LADDR));
    }
    // MADDR
    else if ((cmdin == JDY.cmd.MADDR))
    {
        state = __jdy_AT_Query_with_char(JDY.cmd.MADDR, JDY_charbuf, sizeof(JDY.MADDR));
        __jdy_StrCut(JDY_charbuf, sizeof(JDY.MADDR));
        strncpy(JDY.MADDR, JDY_charbuf, sizeof(JDY.MADDR));
    }
    // NETID
    else if ((cmdin == JDY.cmd.NETID))
    {
        state = __jdy_AT_Query_with_char(JDY.cmd.NETID, JDY_charbuf, sizeof(JDY.NETID));
        __jdy_StrCut(JDY_charbuf, sizeof(JDY.NETID));
        strncpy(JDY.NETID, JDY_charbuf, sizeof(JDY.NETID));
    }
    // NAME
    else if ((cmdin == JDY.cmd.NAME))
    {
        state = __jdy_AT_Query_with_char(JDY.cmd.NAME, JDY_charbuf, sizeof(JDY.NAME));
        __jdy_StrCut(JDY_charbuf, sizeof(JDY.NAME));
        strncpy(JDY.NAME, JDY_charbuf, sizeof(JDY.NAME));
    }
    // PIN
    else if ((cmdin == JDY.cmd.PIN))
    {
        state = __jdy_AT_Query_with_char(JDY.cmd.PIN, JDY_charbuf, sizeof(JDY.PIN));
        __jdy_StrCut(JDY_charbuf, sizeof(JDY.PIN));
        strncpy(JDY.PIN, JDY_charbuf, sizeof(JDY.PIN));
    }
    return state;
}

// len = sizeof(char[]) 包含结尾的'\0'
void __jdy_StrCut(char* str, size_t len)
{
    uint16_t i = 0U;
    if (strlen(str) > len - 1) { return; }
    for (i = 0; i < len;i++)
    {
        if (*(str + i) == '\r')
        {
            if (*(str + i + 1) == '\n')
            {
                memset(str + i, 0, len - i);
            }
        }
    }
}



/**
 * @brief 发送指令  并查询（更新JDY结构体）
 * @param cmdin
 * @param param 参数不能为 空 或者 "\0" ，必须有值
 * @return
 */
JDY_StateDef jdy_SNED_and_EQUERY(const char* cmdin, const char* param)
{
    JDY_StateDef state = __jdy_AT_CMD(cmdin, param);
    if (state == JDY_OK && cmdin == JDY.cmd.DEFAULT)
    {
        // 将结构体的内存清零
        memset(&JDY.isInited, 0, (size_t) (sizeof(JDY_Def) - sizeof(_JDY_CMD_TypeDef)));
        JDY.isInited = 0U;
        return state;
    }
    if (state == JDY_OK)
    {
        __jdy_AT_Query_Selectable(cmdin);
    }
    return state;
}


/**
 * @brief 仅发送 AT 设置指令
 */
JDY_StateDef jdy_SEND_CMD(const char* cmdin, const char* param)
{
    JDY_StateDef state = __jdy_AT_OKcheck();
    if (state == JDY_OK)
    {
        state = __jdy_AT_CMD(cmdin, param);
    }
    return state;
}


/**
 * @brief 发送 mesh 数据
 */
JDY_StateDef jdy_SEND_MESH(const char* data)
{
    return __jdy_AT_CMD(JDY.cmd.MESH, data);
}


/**
 * @brief  仅查询（更新JDY结构体）
 */
JDY_StateDef jdy_EQUERY(const char* cmdin)
{
    JDY_StateDef state = __jdy_AT_OKcheck();
    if (state == JDY_OK)
    {
        state = __jdy_AT_Query_Selectable(cmdin);
    }
    return state;
}


// JDY_StateDef jdy_SET_loopback(const char* data)
// {
//     JDY_StateDef  state = JDY_ERR;
//     state = __jdy_AT_CMD(JDY.cmd.MESH, data);
//     return state;
// }


void jdy_init(void)
{
    char charbuf[JAY_MAX_Para_SIZE] = {0U};

    if (__jdy_AT_OKcheck() == JDY_OK)
    {
        /* 发送指令  并查询（更新JDY结构体）*/
        jdy_SNED_and_EQUERY(JDY.cmd.ENLOG, "1");       // 先开启串口状态显示
        jdy_SNED_and_EQUERY(JDY.cmd.TYPE, "0");
        jdy_SNED_and_EQUERY(JDY.cmd.BAUD, "4");
        jdy_SNED_and_EQUERY(JDY.cmd.MCLSS, "0");
        jdy_SNED_and_EQUERY(JDY.cmd.ROLE, "5");
        jdy_SNED_and_EQUERY(JDY.cmd.POWR, "4");
        // jdy_SNED_and_EQUERY(JDY.cmd.MADDR, "");  // 注释掉，保持默认的 MADDR
        jdy_SNED_and_EQUERY(JDY.cmd.NETID, "1189");
        // jdy_SNED_and_EQUERY(JDY.cmd.NAME, "JDY-5M");
        // jdy_SNED_and_EQUERY(JDY.cmd.PIN, "1234");
        // jdy_SNED_and_EQUERY(JDY.cmd.DEFAULT, '\0'); // 出厂设置


        /* 仅发送指令 （JDY结构体存储之外的命令） */
        /*          很多指令都可以发            */
        __jdy_AT_CMD(JDY.cmd.CLRFRIEND, '\0');


        /* 仅查询（更新JDY结构体）*/
        // __jdy_AT_Query_Selectable(JDY.cmd.ENLOG);
        // __jdy_AT_Query_Selectable(JDY.cmd.TYPE);
        // __jdy_AT_Query_Selectable(JDY.cmd.BAUD);
        // __jdy_AT_Query_Selectable(JDY.cmd.MCLSS);
        // __jdy_AT_Query_Selectable(JDY.cmd.ROLE);
        // __jdy_AT_Query_Selectable(JDY.cmd.POWR);
        __jdy_AT_Query_Selectable(JDY.cmd.STAT);  // STAT 只能被查询，不能调用 __jdy_AT_CMD()
        __jdy_AT_Query_Selectable(JDY.cmd.LADDR);// LADDR 只能被查询，不能调用 __jdy_AT_CMD()
        __jdy_AT_Query_Selectable(JDY.cmd.MADDR);
        // __jdy_AT_Query_Selectable(JDY.cmd.NETID);
        __jdy_AT_Query_Selectable(JDY.cmd.NAME);
        __jdy_AT_Query_Selectable(JDY.cmd.PIN);


        /* 仅查询指令 （JDY结构体存储之外的命令） */
        /*          很多指令都可以发            */
        __jdy_AT_Query_with_char(JDY.cmd.VERSION, charbuf, sizeof(charbuf));


        // 初始化完成标志
        JDY.isInited = 1U;
    }
}

/**
 * @brief 调试时用的 dbg串口 发送到 ble串口
 */
void jdy_loop(void)
{
    static char c1[3] = "AT";
    char c2[3] = { 0U };

    if (JDY.dbgflag == 1U)
    {
        strncpy(c2, JDY_dbg_strbuf, sizeof(c2) - 1);
        if (strcmp(c1, c2) == 0)
        {
            ble_send((uint8_t*) JDY_dbg_strbuf);
        }

        // 处理完成
        JDY.dbgflag = 0U;
    }
}


