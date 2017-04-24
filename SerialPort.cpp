/ SerialPort.cpp: implementation of the SerialPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerialPort.h"
#include "string.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SerialPort::SerialPort()
{
	this->m_bConnected=FALSE;
	this->m_Countff=0;
	app=(CFlowMeterApp *)AfxGetApp();
	if (!open())
	{
		AfxMessageBox("�޷�������������!");
		flag = 0;
	}

}

SerialPort::~SerialPort()
{
	if(m_bConnected)
		close();
	if(osRead.hEvent)
		CloseHandle(osRead.hEvent);
	if(osWrite.hEvent)
		CloseHandle(osWrite.hEvent);

}

BOOL SerialPort::open()
{
     if(app->CommNum==1)
         m_hCom=CreateFile( "COM2", GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, NULL ); 
	 else if (app->CommNum == 0)
		 m_hCom = CreateFile("COM1", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, NULL);
	 else if (app->CommNum == 2)
		 m_hCom = CreateFile("COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	 else if (app->CommNum == 3)
		 m_hCom = CreateFile("COM4", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	 else if (app->CommNum == 5)
		 m_hCom = CreateFile("COM5", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
// �򿪴��ڣ��첽����
    if( m_hCom == NULL ) 
         return( FALSE );

    DCB dcb;
	if(!GetCommState(m_hCom,&dcb))
		 return FALSE;

	dcb.BaudRate = CBR_9600;         //������9600
	dcb.ByteSize = DATABITS_8;       //һ��8λ
	dcb.Parity = EVENPARITY;         //żУ��
	dcb.StopBits = ONESTOPBIT;       //һ��ֹͣλ
	dcb.fBinary = TRUE;
	dcb.fParity = TRUE;
	dcb.fErrorChar = FALSE;
	dcb.ErrorChar = (char) 0;
	dcb.fAbortOnError = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
if (!SetCommState(m_hCom, &dcb))
{
  AfxMessageBox("�������ó���!");
  CloseHandle(m_hCom);
  return FALSE;
} 
SetupComm(m_hCom, 1024, 1024); //���պͷ��ͻ�������Ϊ1024 byte
PurgeComm(m_hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR); //��ս��պͷ��ͻ�����
SetCommMask(m_hCom, EV_RXCHAR | EV_TXEMPTY); //�յ�һ���ֽڻ����ַ�������������¼�
	memset(&osWrite,0,sizeof(osWrite));
	memset(&osRead,0, sizeof(osRead));
	memset(&m_ucTxBuffer[0],0,sizeof(m_ucTxBuffer));
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
m_bConnected =TRUE;
return TRUE;
}

BOOL SerialPort::close()
{
	if(!m_bConnected)
		return FALSE;
	m_bConnected = FALSE;
	SetCommMask(m_hCom,0);
	m_pThread = NULL;
	CloseHandle(m_hCom);
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD SerialPort::SendCmd(unsigned char* str)
{
	DWORD t = 8;          //get the length want to send
	char outbuffer[100];            //1024  buff
	memset(outbuffer, '\0', 100);
	memcpy(outbuffer, str,t);                  // tΪ�������ֽ���
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	BOOL bWriteStat;
	ClearCommError(m_hCom, &dwErrorFlags, &ComStat);  
	//�ú������ͨ�Ŵ��󲢱��洮�ڵĵ�ǰ״̬ComStat,ͬʱ,�ú���������ڵĴ����־�Ա�������롢�������.
	bWriteStat = WriteFile(m_hCom, outbuffer,
		t, &t, &osWrite);             
	if (!bWriteStat)   //   if the result is false
	{
		if (GetLastError() == ERROR_IO_PENDING)  
		{
			WaitForSingleObject(osWrite.hEvent, 1000);  //wait till 
			return t;
		}
		return 0;  //дʧ�ܷ���ʧ��

	}

	PurgeComm(m_hCom, PURGE_TXABORT |
		PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);   //��ջ�����
	return t;
	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD SerialPort::receive(unsigned char* strin,int len,int time,int inter)   //��������  ���wait 2s
{
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 20;  //����������ʱ������	MAXDWORD ���Ҷ�ʱ��ϵ���Ͷ�ʱ�䳣����Ϊ 0, ��ô�ڶ�һ�����뻺���������ݺ�		����������������, �������Ƿ������Ҫ����ַ�.
	TimeOuts.ReadTotalTimeoutMultiplier = 6;   //���ܳ�ʱ��ReadTotalTimeoutMultiplier��10��ReadTotalTimeoutConstant
	TimeOuts.ReadTotalTimeoutConstant = 100;

	TimeOuts.WriteTotalTimeoutMultiplier = 30;//дʱ��ϵ��
	TimeOuts.WriteTotalTimeoutConstant = 1000;//дʱ�䳣��
	SetCommTimeouts(m_hCom, &TimeOuts);

	memset(&osRead, 0, sizeof(OVERLAPPED));
	osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	unsigned char str[50];
	memset(str, '\0', sizeof(str));
	DWORD dwBytesRead = len;//��ȡ���ֽ���
	BOOL bReadStat=FALSE;
	ClearCommError(m_hCom, &dwErrorFlags, &ComStat);
	Sleep(time);
	bReadStat = ReadFile(m_hCom, str,
		dwBytesRead, &dwBytesRead, &osRead);
	
	if (bReadStat == FALSE)
	{


		if (GetLastError() == ERROR_IO_PENDING)
		{
			GetOverlappedResult(m_hCom, &osRead, &dwBytesRead, TRUE);
			memcpy(strin, str, dwBytesRead);
			return dwBytesRead;
		}
    }
	//	return 0;
	PurgeComm(m_hCom, PURGE_TXABORT |
		PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	//m_disp = str;

	memcpy(strin, str, dwBytesRead);
	return dwBytesRead;
}
long bcdtrans(unsigned char a, unsigned char b)    //bcd to 10jinzhi
{ 
	int i, j,x,y;
	long temp;
	i = int(a) / 16;
	j = int(a) % 16;
	x = int(b) / 16;
    y = int(b) % 16;
	temp = i * 1000 + j * 100 + x * 10 + y;
	return temp;
	
}


void shijinzhiasciitrans(long a,unsigned char* str)   // 10����ת��Ϊ ascii��
{
	unsigned char i, j, x, y;
	unsigned char b[4];
	i = a / 1000;
	j = (a - i * 1000) / 100;
	x = (a - i * 1000 - j * 100) / 10;
	y = a - i * 1000 - j * 100 - x * 10;
	b[0] = i+48;
	b[1] = j+48;
	b[2] = x+48;
	b[3] = y+48;
	memcpy(str, b,sizeof(b));
}


unsigned int CRC(unsigned char *SendBuf, int nEnd)//crcУ��
{
	UINT16 wCRC = 0xFFFF;
	int i = 0;
	for (i = 0; i<nEnd; i++)
	{
		wCRC ^= (UINT16)SendBuf[i];
		for (int j = 0; j<8; j++)
		{
			if (wCRC & 1)
			{
				wCRC >>= 1; wCRC ^= 0xA001;
			}
			else
			{
				wCRC >>= 1;
			}
		}
	}
	return wCRC;
}
