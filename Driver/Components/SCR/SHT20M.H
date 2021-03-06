#ifndef __SHT20M_H
#define __SHT20M_H

#include "STM32_SPI.H"
typedef enum	_sht20m_cmd
{
	cmd1	=	0x03,		//�����ּĴ���
	cmd2	=	0x04,		//������Ĵ���
	cmd3	=	0x06,		//д�������ּĴ���
	cmd4	=	0x10,		//д������ּĴ���
}sht20m_cmd_def;
typedef struct	_sht20m_pro_write
{
	unsigned char device_address;		//�ӻ���ַ
	sht20m_cmd_def cmd;							//������
	unsigned char	reg_address_h;		//�Ĵ�����ַ���ֽ�
	unsigned char reg_address_l;		//�Ĵ�����ַ���ֽ�
	unsigned char reg_data_h;				//�Ĵ���ֵ���ֽ�
	unsigned char reg_data_l;				//�Ĵ���ֵ���ֽ�
}sht20m_pro_write_def;
typedef struct	_sht20m_pro_read
{
	unsigned char device_address;		//�ӻ���ַ
	sht20m_cmd_def cmd;							//������
	unsigned char	len;							//�ֽ���
	unsigned char data[256];				//�Ĵ�����ַ���ֽ�
}sht20m_pro_read_def;



void api_SHT20M_configuration(void);
	

#endif
