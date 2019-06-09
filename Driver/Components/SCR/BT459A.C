#include "BT459A.H"


/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void api_bt459a_configuration(void)
{

}
/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void api_bt459a_server(void)
{

}
/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
unsigned char api_bt459a_get_frame_ReadQuantity(unsigned char addr,unsigned char* pbuffer)
{
	unsigned char frame[128]={0};
	unsigned short crc16	=	0;
	frame[0]=addr;
	frame[1]=ReadHoldingRegisters;	//�����ּĴ������ֽ�ָ��������ɶ��������߶��
	//---------------�Ĵ�����ַ
	frame[2]=reg_quantity>>8;				//�Ĵ�����ʼ��ַ��8λ
	frame[3]=reg_quantity&0xFF;			//�Ĵ�����ʼ��ַ��8λ
	//---------------
	frame[4]=0x00;									//�Ĵ���������8λ
	frame[5]=0x01;
	
	crc16	=	CRC16_MODBUS(frame,6);
	
	frame[6]=crc16&0xFF;
	frame[7]=crc16>>8;	
	
	pbuffer	=	frame;
	
	return 8;
}
/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
unsigned char api_bt459a_get_frame_ReadWeight(unsigned char addr,unsigned char* pbuffer)
{
	unsigned char frame[128]={0};
	unsigned short crc16	=	0;
	frame[0]=addr;
	frame[1]=ReadHoldingRegisters;	//�����ּĴ������ֽ�ָ��������ɶ��������߶��
	//---------------�Ĵ�����ַ
	frame[2]=reg_weight>>8;				//�Ĵ�����ʼ��ַ��8λ
	frame[3]=reg_weight&0xFF;			//�Ĵ�����ʼ��ַ��8λ
	//---------------
	frame[4]=0x00;									//�Ĵ���������8λ
	frame[5]=0x01;
	
	crc16	=	CRC16_MODBUS(frame,6);
	
	frame[6]=crc16&0xFF;
	frame[7]=crc16>>8;	
	
	pbuffer	=	frame;
	
	return 8;
}
//------------------------------------------------------------------------------

/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
unsigned char api_bt459a_get_frame_SetQuantity(unsigned char addr,unsigned short Quantity,unsigned char* pbuffer)
{
	unsigned char frame[128]={0};
	unsigned short crc16	=	0;
	frame[0]=addr;
	frame[1]=WriteSingleRegister;		//д�������ּĴ������ֽ�ָ�������ֻ��дһ��
	//---------------�Ĵ�����ַ
	frame[2]=reg_quantity>>8;				//�Ĵ�����ʼ��ַ��8λ
	frame[3]=reg_quantity&0xFF;			//�Ĵ�����ʼ��ַ��8λ
	//---------------
	frame[4]=Quantity>>8;									//�Ĵ���������8λ
	frame[5]=Quantity&0xFF;
	
	crc16	=	CRC16_MODBUS(frame,6);
	
	frame[6]=crc16&0xFF;
	frame[7]=crc16>>8;	
	
	frame[0]=0x01;
	frame[1]=0x06;
	frame[2]=0x00;
	frame[3]=0x0a;
	frame[4]=0x00;
	frame[5]=0x00;
	
	crc16	=	CRC16_MODBUS(frame,6);
	
	frame[6]=crc16&0xFF;
	frame[7]=crc16>>8;
	
	pbuffer	=	frame;
	
//	//-----------------------------
//	frame[0]=0x01;
//	frame[1]=0x06;
//	frame[2]=0x00;
//	frame[3]=0x0a;
//	frame[4]=0x00;
//	frame[5]=0x00;	
//	crc16	=	CRC16_MODBUS(frame,6);	
//	frame[6]=crc16&0xFF;
//	frame[7]=crc16>>8;	
//	pbuffer	=	frame;
//	
//	//-----------------------------
//	frame[0]=0x02;
//	frame[1]=0x06;
//	frame[2]=0x00;
//	frame[3]=0x0a;
//	frame[4]=0x00;
//	frame[5]=0x00;	
//	crc16	=	CRC16_MODBUS(frame,6);	
//	frame[6]=crc16&0xFF;
//	frame[7]=crc16>>8;	
//	pbuffer	=	frame;
//	
//	//-----------------------------
//	frame[0]=0x03;
//	frame[1]=0x06;
//	frame[2]=0x00;
//	frame[3]=0x0a;
//	frame[4]=0x00;
//	frame[5]=0x00;	
//	crc16	=	CRC16_MODBUS(frame,6);	
//	frame[6]=crc16&0xFF;
//	frame[7]=crc16>>8;	
//	pbuffer	=	frame;
//	
//	//-----------------------------
//	frame[0]=0x04;
//	frame[1]=0x06;
//	frame[2]=0x00;
//	frame[3]=0x0a;
//	frame[4]=0x00;
//	frame[5]=0x00;	
//	crc16	=	CRC16_MODBUS(frame,6);	
//	frame[6]=crc16&0xFF;
//	frame[7]=crc16>>8;	
//	pbuffer	=	frame;
	
	
	return 8;
}
//------------------------------------------------------------------------------


