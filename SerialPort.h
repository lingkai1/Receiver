// SerialPort.h: interface for the SerialPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALPORT_H__5ECD2FCF_946A_41AA_9B60_A054A48D4AEB__INCLUDED_)
#define AFX_SERIALPORT_H__5ECD2FCF_946A_41AA_9B60_A054A48D4AEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WAIT_PREAMBLE  0
#define WAIT_DELIMITER 1
#define READING_DATA  2

long bcdtrans(unsigned char,unsigned char);
void shijinzhiasciitrans(long,unsigned char*);
unsigned int CRC(unsigned char *SendBuf, int nEnd);
class SerialPort  
{
public:
	volatile BOOL m_bConnected;
	volatile HANDLE m_hCom;
	unsigned char m_Countff;
	unsigned char m_ucRcvLen;
	unsigned char m_ucRxBuffer[255];
	unsigned char m_ucTxBuffer[255];
	DWORD             dwWritten;
	OVERLAPPED        osWrite;
	OVERLAPPED        osRead;
	COMSTAT           strComStat;
	//DWORD             dwErrorFlags;
	CWinThread* m_pThread;
	CFlowMeterApp* app;
	BOOL flag=1;
public:


	DWORD SendCmd(unsigned char*);       //发送数据 ，输入指定字符串；
	DWORD receive(unsigned char*, int,int,int);  //接受数据 ，返回接受的数据长度；
	DWORD testreceive(char*, int);
	BOOL close();
	BOOL open();
	SerialPort();
	virtual ~SerialPort();

};

#endif // !defined(AFX_SERIALPORT_H__5ECD2FCF_946A_41AA_9B60_A054A48D4AEB__INCLUDED_)
