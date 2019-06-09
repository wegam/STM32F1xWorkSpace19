#include "WDH320S.H"

#include "stdio.h"
#include "string.h"

/*ͨ�������ö����*/
enum MESSAGE_CMD
{
    CMD_ONE_VS_N  = 0,                //�ɼ�������1��N�ȶ�
    CMD_ONE_VS_G,                     //�ɼ�������1��G�ȶ�
    CMD_ONE_VS_ONE,                   //�ɼ�������1��1�ȶ�
    CMD_REGISTER,                     //ע����ָ����
    CMD_REG_END,                      //����ע��
    CMD_DELETE_ONE,                   //ɾ��������ָ
    CMD_DELETE_ALL,                   //ɾ��������Ϣ
    CMD_UPLOAD_ALL_ID,                //�ϴ�������ָID
    CMD_UPLOAD_INFOR,                 //�ϴ�ָ����ָ��Ϣ
    CMD_UPLOAD_TEMPLATE,              //�ϴ�ָ����ָģ��
    CMD_UPLOAD_INFOR_TEMPLATES,       //�ϴ�ָ����ָ�Լ���Ӧģ��
    CMD_CREATE_TEMPLATE,              //�ɼ����ϴ�ģ��
    CMD_DOWNLOAD_INFOR_TEMPLATES,     //������ָ��Ϣͷ������ģ��
    CMD_UPLOAD_VERSION,               //��ȡ�̼��汾��
    CMD_UPLOAD_COUNT,                 //��ȡ��ָע������
    CMD_CHK_FINGER,                   //�����ָ״̬
    CMD_UPLOAD_SEQUENCE,              //��ȡ���к�
    CMD_SET_BAUD,                     //���ò�����
    CMD_SET_DEVID,                    //�����豸���
};

//ͨѶ�����ֽ�
#define ASCII_XON           (0x40)
#define ASCII_XON_DATA      (0x3e)
#define ASCII_XOFF          (0x0D)
#define BROADCASTADDR       (0xff)

//���������
typedef struct
{
	unsigned char Start;	//֡ͷ
	unsigned char Cmd;      //ָ����
	unsigned char Devid;	//�豸��
	unsigned char P1;		//���� 1 �� ���ݳ��ȵ��ֽ�
	unsigned char P2;		//���� 2 �� ���ݳ��ȸ��ֽ�
	unsigned char P3;		//���� 3
	unsigned char CHK;      //У���
	unsigned char End;      //֡β
}*P_CMD_PACK_SEND;

//���������
typedef struct
{
	unsigned char Start;	//֡ͷ
	unsigned char Cmd;      //ָ����
	unsigned char Devid;	//�豸��
	unsigned char Q1;       //���� 1 �� ���ݳ��ȵ��ֽ�
	unsigned char Q2;       //���� 2 �� ���ݳ��ȸ��ֽ�
	unsigned char Q3;       //�����ֵ
	unsigned char CHK;      //У���
	unsigned char End;      //֡β
}*P_CMD_PACK_RECV;

//���ݰ�ͷ��
typedef struct
{
	unsigned char Start;	//֡ͷ
	unsigned char Cmd;	//ָ����
	unsigned char Devid;	//�豸��
	unsigned char *Data; //ָ�򸽼����ݿ�ʼλ�õ�ָ��
}*P_DATA_PACK_HEAD;

//���ݰ�β��
typedef struct
{
	unsigned char CHK;	//У���
	unsigned char End;	//֡β
}*P_DATA_PACK_TAIL;


//����У���
unsigned char api_wdh320s_get_check_xor(unsigned char *data,unsigned int len)
{
	unsigned char chk = *data;
	unsigned long i	=	0;
	for(i=1;i<len;i++)
	{
		chk ^= *(data+i);
	}
	return chk;
}
/*
 * @brief api_wdh320s_get_pack_len���õ����Ͱ����ȡ�
 * @param cmd(I)            ������
 * @param Extra_DataLen(I)  ����Я������Ҫ���ݳ��ȣ��ο�ͨѶЭ��
 * @return ���Ͱ�����
 */
int api_wdh320s_get_pack_len(int cmd, unsigned int Extra_DataLen)
{
    int retlen = 0;

    //���з��Ͱ�ֻ�� CMD_DOWNLOAD_INFOR_TEMPLATES �и�������
    if(CMD_DOWNLOAD_INFOR_TEMPLATES == cmd)
    {
        //У�鸽�ӳ����Ƿ���ȷ
        if((Extra_DataLen - 18) % 512 != 0)
        {
            //dbgInfoPrint("�������ݳ��ȴ���.");
            return -1;
        }
        else
        {
            retlen = CMD_PACK_SIZE + DATA_PACK_HEAD_SIZE + Extra_DataLen + DATA_PACK_TAIL_SIZE;
        }
    }
    else
    {
        retlen = CMD_PACK_SIZE;
    }
    //dbgInfoPrint("���Ͱ�����ӦΪ %d �ֽ�.", retlen);
    return retlen;
}


/**
 * @brief api_wdh320s_get_send_pack�����һ�����Ͱ���
 * @param p(O)             ���Ͱ�������
 * @param _cmd(I)          ������
 * @param _device(I)       �豸ID
 * @param _p1(I)           ����1 �ɾ����������
 * @param _p2(I)           ����2 �ɾ����������
 * @param _p3(I)           ����3 �ɾ����������
 * @param Extra_Data(I)    ��������
 * @param Extra_DataLen(I) �������ݵĳ���
 * @return ���Ͱ�����
 */
int api_wdh320s_get_send_pack(unsigned char * p,
                    unsigned char _cmd,
                    unsigned char _device,
                    unsigned char _p1,
                    unsigned char _p2,
                    unsigned char _p3,
                    unsigned char *Extra_Data,
                    unsigned int  Extra_DataLen)
{
    //����ֵ-���Ͱ���С
    int retlen = 0;
	unsigned char *databuf	= NULL;
	P_DATA_PACK_TAIL datatail;
    //���-�����
    P_CMD_PACK_SEND head = (P_CMD_PACK_SEND)p;
    head->Start = ASCII_XON;	//֡ͷ
    head->Cmd = _cmd;           //ָ����
    head->Devid = _device;      //�豸��
    head->P1 = _p1;             //���� 1
    head->P2 = _p2;             //���� 2
    head->P3 = _p3;             //���� 3
    head->CHK = api_wdh320s_get_check_xor((unsigned char *)head, 6);	//У���
    head->End = ASCII_XOFF;     //֡β

    //dbgInfoPrint("���÷��Ͱ���������ɹ�.");
    retlen += CMD_PACK_SIZE;
    //���-���ݰ�
    if(NULL != Extra_Data && 0 != Extra_DataLen)
    {
        //��������ͷ
        P_DATA_PACK_HEAD datahead = (P_DATA_PACK_HEAD)(p + CMD_PACK_SIZE);
        datahead->Start = ASCII_XON_DATA;
        datahead->Cmd = _cmd;
        datahead->Devid = _device;

        //��������
		databuf = p + CMD_PACK_SIZE +DATA_PACK_HEAD_SIZE;
        memcpy(databuf,Extra_Data,Extra_DataLen);

        //��������β
		datatail = (P_DATA_PACK_TAIL)(p + CMD_PACK_SIZE + DATA_PACK_HEAD_SIZE + Extra_DataLen);
        datatail->CHK = api_wdh320s_get_check_xor((unsigned char *)datahead, DATA_PACK_HEAD_SIZE + Extra_DataLen);
        datatail->End = ASCII_XOFF;

        //dbgInfoPrint("���÷��Ͱ������ݰ��ɹ�,�������ݹ� %d �ֽ�.", Extra_DataLen);
        retlen += DATA_PACK_HEAD_SIZE + Extra_DataLen + DATA_PACK_TAIL_SIZE;
    }
    //dbgInfoPrint("������Ͱ����� %d �ֽ�.", retlen);
    return retlen;
}


/**
 * @brief api_wdh320s_get_pack_data����ý��հ��е���Ҫ���ݡ�
 * @param pack(I)      ���յ������ݰ�
 * @param ebuf(O)      ��Ҫ��������
 * @param ebuflen(IO)  ����Ϊ��������С�����Ϊ��Ҫ�������ݳ���
 * @param special(O)   Я�����ݣ����ͨ����������ص����ݣ�specialͨ��Ϊ���α����ĵ�ַ
 *                     �����¼������
 *                       CMD_ONE_VS_N
 *                       CMD_ONE_VS_ONE
 *                       CMD_ONE_VS_G
 *                       CMD_UPLOAD_COUNT
 *                     ���ͨѶЭ��
 * @return ����ִ�еĽ�� �� ��������
 */
int api_wdh320s_get_pack_data(void *pack, void *ebuf, unsigned int *ebuflen, int *special)
{
    unsigned char *p = (unsigned char *)pack;                           //�����ݰ��ֽ����鴦��

    P_CMD_PACK_RECV cmdpack = (P_CMD_PACK_RECV)p;                       //ָ��-�����
    unsigned char *datapack =  p + CMD_PACK_SIZE;                       //ָ��-���ݰ�
    unsigned char *databuf = p + CMD_PACK_SIZE + DATA_PACK_HEAD_SIZE;   //ָ��-���ݰ��е�����

    //1. ������еĸ������ݳ���
    unsigned int extarlen = 0;
    //��������Q1��Q2����ʾ�������ݳ��ȣ������ǵĸ�������Ϊ0
    if(	cmdpack->Cmd == CMD_ONE_VS_N 		||	//��ʾFID
		cmdpack->Cmd == CMD_ONE_VS_ONE 		|| 	//��ʾFID
		cmdpack->Cmd == CMD_ONE_VS_G 		||	//��ʾFID
		cmdpack->Cmd == CMD_UPLOAD_COUNT 	||	//��ʾע�����
		cmdpack->Cmd == CMD_CHK_FINGER 		||	//��ʾ��ָ״̬
		cmdpack->Cmd == CMD_REGISTER)   		//320Ϊ��ʾ�������ݳ���0��330��ʾ��ָ��б�ĽǶ�
    {
        extarlen = 0;
    }
    //������������Q1��Q2��ʾ�������ݳ���
    else
    {
        extarlen = cmdpack->Q1 + cmdpack->Q2*256;
    }

    //2.У���û�����������
    //dbgInfoPrint("0X%02X ����������Ҫ���ݳ��ȣ�%d, ���뻺����Ϊ��%d",\
                 cmdpack->Cmd, extarlen, ebuflen?*ebuflen:0);
    if((ebuflen?*ebuflen:0) < extarlen)
    {
        //dbgInfoPrint("���뻺����̫С.");
        return BUF_SMALL;
    }
    else
    {
        memset(ebuf, 0x00, ebuflen?*ebuflen:0);
    }

    //3. У��-������Ƿ���ȷ
    if(api_wdh320s_get_check_xor((unsigned char *)cmdpack, 6) != cmdpack->CHK)
    {
        //dbgInfoPrint("���ݰ�ͷ����.");
        return  PACK_ISBAD;
    }
    //dbgInfoPrint("���ݰ�ͷУ��ͨ��.");

    //4. У��-���ݰ��Ƿ���ȷ
    if(	cmdpack->Cmd == CMD_ONE_VS_N 	||	//Я��FID
		cmdpack->Cmd == CMD_ONE_VS_ONE	||	//Я��FID
		cmdpack->Cmd == CMD_ONE_VS_G 	||	//Я��FID
		cmdpack->Cmd == CMD_UPLOAD_COUNT||	//Я��ע�����
		cmdpack->Cmd == CMD_CHK_FINGER	||	//��ʾ��ָ״̬
		cmdpack->Cmd == CMD_REGISTER)      	//320Ϊ��ʾ�������ݳ���0��330��ʾ��ָ��б�ĽǶ�
    {
        //dbgInfoPrint("���ݰ�ͷЯ������.");
    }
    else if(extarlen != 0)
    {
        if(api_wdh320s_get_check_xor((unsigned char *)datapack, extarlen + 3) != *(datapack +extarlen + 3))
        {
            //dbgInfoPrint("���ݰ���������.");
            return PACK_ISBAD;
        }
        //dbgInfoPrint("���ݰ�����У��ͨ��.");
    }
    else
    {
        //dbgInfoPrint("�޸�������.");
    }
    //5. �����������ݲ����ظ������ݵĳ���
    //��������������ͨ������������û�ID
    if(	cmdpack->Cmd == CMD_ONE_VS_N 		||	//Я��FID
		cmdpack->Cmd == CMD_ONE_VS_ONE 		||	//Я��FID
		cmdpack->Cmd == CMD_ONE_VS_G 		||	//Я��FID
		cmdpack->Cmd == CMD_UPLOAD_COUNT 	|| 	//Я��ע�����
		cmdpack->Cmd == CMD_REGISTER)         	//320Ϊ��ʾ�������ݳ���0��330��ʾ��ָ��б�ĽǶ�
    {
        if(special != NULL)
        {
            *special = cmdpack->Q1 + cmdpack->Q2*256;
            //dbgInfoPrint("��ȡ������������ݳɹ���ֵ=%d.", *special);
        }
    }
    //���渽�����ݼ�����
    else
    {
        if(ebuf != NULL)
        {
			memcpy(ebuf ,databuf, extarlen);
            //dbgInfoPrint("�������ݰ��������ݳɹ�.");
        }
        if(ebuflen != NULL)
        {
            *ebuflen = extarlen;
        }
    }
    //����ֵ-����ִ�н��
    //dbgInfoPrint("����ֵ����ִ�н��: %d", cmdpack->Q3);
    return cmdpack->Q3;
}
/////////////////////////////////////////////////////////////////////////


/* ʹ�÷��� */
/*
void func()
{
    //1.����һ�����Ͱ��������÷�������
    //  ����һ������û�и������ݵķ��Ͱ���
    unsigned char sendpack[CMD_PACK_SIZE] = {0};
    GetSendPack(sendpack, CMD_CHK_FINGER, ui->comboBox_Devid->currentText().toInt(), 0, 0, 0);

    //  ���������������з��Ͱ���
    int lenx = api_wdh320s_get_pack_len(CMD_CHK_FINGER, 0);                   //�����������Я���ĸ������ݻ�÷��Ͱ��Ĵ�С
    if(lenx < 0)
    {
        return -1;
    }
    unsigned char * sendpack = (unsigned char *)calloc(1, lenx);    //���䷢�Ͱ��ڴ�

    int len = api_wdh320s_get_send_pack(sendpack,                                 //���÷��Ͱ�
                    CMD_CHK_FINGER,
                    ui->comboBox_Devid->currentText().toInt(),
                    0,
                    0,
                    0,
                    NULL,                                           //���Ͱ���Я��������
                    0);                                             //Я�����ݳ���


    //2.����һ�����ջ�����
    unsigned char *recvdata = ...;


    //3.ʹ����ķ������շ��ص����ݡ�
    you_recv(recvdata)


    //4.�ѽ��յ����ݶ����˺�������������ȡ���͵����������������ߴ��صĸ����������ݡ�
    int rel = api_wdh320s_get_pack_data(recvdata, NULL, NULL, NULL);
    if(rel < 0)
    {
        cout << "���ݰ���";
    }

    //do what you want to do ...
}

*/

