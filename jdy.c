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
 * @brief �жϴ�����������ble���ڽ�������ж���ִ��
 */
void jdy_it_ble_handle(void)
{
    if (JDY.state == JDY_listening)
    {
        memset((char*) JDY_recvDATA, 0, strlen(JDY_recvDATA));
        strcpy((char*) JDY_recvDATA, (const char*) ble_datarev_buff);

        // ��־λ
        JDY.state = JDY_datahandling;
    }
    return;
}
/**
 * @brief �жϴ�����������dbg���ڽ�������ж���ִ��
 */
void jdy_it_dbg_handle(void)
{
    if (JDY.dbgflag == 0U)
    {
        if (strlen((char*) dbg_datarev_buff) < sizeof(JDY_dbg_strbuf))
        {
            strcpy(JDY_dbg_strbuf, (char*) dbg_datarev_buff);

            // ��־λ
            JDY.dbgflag = 1U;
        }
    }
    return;
}


/**
 * @brief ������ AT ����ָ��
 *  ----- ���� AT+cmdin+param+������
 *  ----- һ�����յ� OK ����ɹ�
 * @param cmdin ָ��
 * @param Param ��������Ϊ �� ���� "\0" ��������ֵ
 * @return JDY_StateDef
 */
JDY_StateDef __jdy_AT_CMD(const char* cmdin, const char* param)
{
    uint32_t k = 0U;

    if (JDY.state != JDY_idle)
    {
        return JDY_Waitting;
    }

    // �����������ڽ���
    ble_recv_start_DMA();

    // ״̬��
    JDY.state = JDY_listening;

    // ����Ԥ����   
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

    // �������ڷ��� 
    // dbg_send((uint8_t*)JDY_tx_AT_DATA);
    ble_send((uint8_t*) JDY_tx_AT_DATA);

    k = HAL_GetTick();

    // blocking ��ʽ��ѯ״̬ or ��ʱ
    while (JDY.state == JDY_listening)
    {
        // û���յ�����
        if ((HAL_GetTick() - k) > JDY_Listening_timeout)
        {
            JDY.state = JDY_idle;
            return JDY_TMOUT;
        }
    }
    // ���յ�������
    if (JDY.state == JDY_datahandling)
    {
        // �� "OK" ���Ƚ�
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
 * @brief AT ��ѯָ��
 * @param cmdin ָ��
 * @param param ���صĲ���, char �ͣ��������Ҫ�����ͣ�����ת������ʹ�á�
 *              (��bug�����ܻ������ʹ��ʱȷ�Ϻ��ַ������ȣ�����ʹ��__jdy_AT_Query_return_char()����)
 * @param paramlen param �ĳ��ȣ� paramlen = sizeof() �����˽�β��'\0'
 * @return JDY_StateDef
 */
JDY_StateDef __jdy_AT_Query(const char* cmdin, char* param, size_t paramlen)
{
    uint32_t k = 0U;
    char  cmdBuff[JAY_MAX_AT_SIZE] = { 0U };
    char  cmdBuff2[JAY_MAX_AT_SIZE] = { 0U };
    char  paramBuff[JAY_MAX_Para_SIZE] = { 0U };    // paramlen = sizeof() �����˽�β��'\0'
    // char  recvBuff[JAY_MAX_Recv_SIZE] = { 0U };

    if (JDY.state != JDY_idle)
    {
        return JDY_Waitting;
    }

    // paramlen = sizeof() �����˽�β��'\0'
    if (paramlen > JAY_MAX_Para_SIZE)
    {
        return JDY_ERR;
    }

    // �����������ڽ���
    ble_recv_start_DMA();

    // ״̬��
    JDY.state = JDY_listening;

    // ����Ԥ����   
    // strcpy�������Ч��
    strcpy(JDY_tx_AT_DATA, JDY.cmd.AT); // "AT"
    strcat(JDY_tx_AT_DATA, JDY.cmd.add);// "+"
    strcat(JDY_tx_AT_DATA, cmdin);      // "cmdin"
    strcat(JDY_tx_AT_DATA, JDY.cmd.over); // "\r\n"

    // �������ڷ��� 
    // dbg_send((uint8_t*) JDY_tx_AT_DATA);
    ble_send((uint8_t*) JDY_tx_AT_DATA);

    k = HAL_GetTick();

    // blocking ��ʽ��ѯ״̬ or ��ʱ
    while (JDY.state == JDY_listening)
    {
        // û���յ�����
        if ((HAL_GetTick() - k) > JDY_Listening_timeout)
        {
            JDY.state = JDY_idle;
            return JDY_TMOUT;
        }
    }
    // ���յ�������
    if (JDY.state == JDY_datahandling)
    {
        // strcpy�������Ч��
        strcpy(cmdBuff, JDY.cmd.add); //"+"
        strcat(cmdBuff, cmdin);// "cmdin"
        strcat(cmdBuff, JDY.cmd.equ);// "="
        k = strlen(cmdBuff);

        // strncpy ����ǰk���ַ�
        strncpy(cmdBuff2, JDY_recvDATA, k); //""

        // ���Ƚ�
        if (strcmp((const char*) cmdBuff, (const char*) cmdBuff2) == 0)
        {
            // strncpy ���Ƶ� k+1 ������ paramlen-1 ���ַ� ��ͬʱȥ���� "\r\n"
            strncpy(paramBuff, JDY_recvDATA + k, paramlen - 1);

            // paramBuff �Ĵ��ڣ�ȷ���� param �������
            strcpy(param, paramBuff);

            JDY.state = JDY_idle;
            return JDY_OK;
        }
    }
    JDY.state = JDY_idle;
    return JDY_ERR;
}


/**
 * @brief AT��ѯָ��
 * @param cmdin ָ����紫�� JDY.cmd.POWR
 * @param num ���������޸ĵ�ֵ ���� JDY.POWR
 * @return JDY_StateDef
 */
JDY_StateDef __jdy_AT_Query_with_enum(const char* cmdin, int* num)
{
    char    parabuf[JAY_MAX_Para_SIZE] = { 0U };
    int  numbuf = 0U;
    JDY_StateDef state = JDY_ERR;
    // if (__jdy_AT_OKcheck() == JDY_OK)
    // {
        // paramlen = sizeof() �����˽�β��'\0'
    state = __jdy_AT_Query(cmdin, parabuf, (size_t) 2);
    numbuf = ((int) parabuf[0] - 0x30);   // ASCII   0~9   0x30~0x39
    *num = numbuf;
    // }
    return state;
}


/**
 * @brief AT��ѯָ��
 * @param cmdin ָ����紫�� JDY.cmd.NETID
 * @param c ���������޸ĵ�ֵ ���� JDY.NETID
 * @param paramlen paramlen = sizeof(char[]) ����c�ĳ��� �����˽�β��'\0'
 * @return JDY_StateDef
 */
JDY_StateDef __jdy_AT_Query_with_char(const char* cmdin, char* c, size_t paramlen)
{
    JDY_StateDef state = JDY_ERR;
    state = __jdy_AT_Query(cmdin, c, paramlen);
    return state;
}


/**
 * @brief ��ѯ������JDY�ṹ�壩
 * @param cmdin ָ����紫�� JDY.POWR
 * @return
 */
JDY_StateDef __jdy_AT_Query_Selectable(const char* cmdin)
{
    int  JDY_enumbuf = 0U;
    char JDY_charbuf[JAY_MAX_Para_SIZE] = { 0U };
    JDY_StateDef  state = JDY_ERR;

    // �ṹ���Ա��ֻ��ʹ�� if-else ������ switch-case 
    /******* enum ���� ********/
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

    /******* char* ���� ********/
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

// len = sizeof(char[]) ������β��'\0'
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
 * @brief ����ָ��  ����ѯ������JDY�ṹ�壩
 * @param cmdin
 * @param param ��������Ϊ �� ���� "\0" ��������ֵ
 * @return
 */
JDY_StateDef jdy_SNED_and_EQUERY(const char* cmdin, const char* param)
{
    JDY_StateDef state = __jdy_AT_CMD(cmdin, param);
    if (state == JDY_OK && cmdin == JDY.cmd.DEFAULT)
    {
        // ���ṹ����ڴ�����
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
 * @brief ������ AT ����ָ��
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
 * @brief ���� mesh ����
 */
JDY_StateDef jdy_SEND_MESH(const char* data)
{
    return __jdy_AT_CMD(JDY.cmd.MESH, data);
}


/**
 * @brief  ����ѯ������JDY�ṹ�壩
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
        /* ����ָ��  ����ѯ������JDY�ṹ�壩*/
        jdy_SNED_and_EQUERY(JDY.cmd.ENLOG, "1");       // �ȿ�������״̬��ʾ
        jdy_SNED_and_EQUERY(JDY.cmd.TYPE, "0");
        jdy_SNED_and_EQUERY(JDY.cmd.BAUD, "4");
        jdy_SNED_and_EQUERY(JDY.cmd.MCLSS, "0");
        jdy_SNED_and_EQUERY(JDY.cmd.ROLE, "5");
        jdy_SNED_and_EQUERY(JDY.cmd.POWR, "4");
        // jdy_SNED_and_EQUERY(JDY.cmd.MADDR, "");  // ע�͵�������Ĭ�ϵ� MADDR
        jdy_SNED_and_EQUERY(JDY.cmd.NETID, "1189");
        // jdy_SNED_and_EQUERY(JDY.cmd.NAME, "JDY-5M");
        // jdy_SNED_and_EQUERY(JDY.cmd.PIN, "1234");
        // jdy_SNED_and_EQUERY(JDY.cmd.DEFAULT, '\0'); // ��������


        /* ������ָ�� ��JDY�ṹ��洢֮������ */
        /*          �ܶ�ָ����Է�            */
        __jdy_AT_CMD(JDY.cmd.CLRFRIEND, '\0');


        /* ����ѯ������JDY�ṹ�壩*/
        // __jdy_AT_Query_Selectable(JDY.cmd.ENLOG);
        // __jdy_AT_Query_Selectable(JDY.cmd.TYPE);
        // __jdy_AT_Query_Selectable(JDY.cmd.BAUD);
        // __jdy_AT_Query_Selectable(JDY.cmd.MCLSS);
        // __jdy_AT_Query_Selectable(JDY.cmd.ROLE);
        // __jdy_AT_Query_Selectable(JDY.cmd.POWR);
        __jdy_AT_Query_Selectable(JDY.cmd.STAT);  // STAT ֻ�ܱ���ѯ�����ܵ��� __jdy_AT_CMD()
        __jdy_AT_Query_Selectable(JDY.cmd.LADDR);// LADDR ֻ�ܱ���ѯ�����ܵ��� __jdy_AT_CMD()
        __jdy_AT_Query_Selectable(JDY.cmd.MADDR);
        // __jdy_AT_Query_Selectable(JDY.cmd.NETID);
        __jdy_AT_Query_Selectable(JDY.cmd.NAME);
        __jdy_AT_Query_Selectable(JDY.cmd.PIN);


        /* ����ѯָ�� ��JDY�ṹ��洢֮������ */
        /*          �ܶ�ָ����Է�            */
        __jdy_AT_Query_with_char(JDY.cmd.VERSION, charbuf, sizeof(charbuf));


        // ��ʼ����ɱ�־
        JDY.isInited = 1U;
    }
}

/**
 * @brief ����ʱ�õ� dbg���� ���͵� ble����
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

        // �������
        JDY.dbgflag = 0U;
    }
}


