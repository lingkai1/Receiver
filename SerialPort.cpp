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
		AfxMessageBox("无法建立串口链接!");
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
// 打开串口，异步操作
    if( m_hCom == NULL ) 
         return( FALSE );

    DCB dcb;
	if(!GetCommState(m_hCom,&dcb))
		 return FALSE;

	dcb.BaudRate = CBR_9600;         //波特率9600
	dcb.ByteSize = DATABITS_8;       //一次8位
	dcb.Parity = EVENPARITY;         //偶校验
	dcb.StopBits = ONESTOPBIT;       //一个停止位
	dcb.fBinary = TRUE;
	dcb.fParity = TRUE;
	dcb.fErrorChar = FALSE;
	dcb.ErrorChar = (char) 0;
	dcb.fAbortOnError = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
if (!SetCommState(m_hCom, &dcb))
{
  AfxMessageBox("串口设置出错!");
  CloseHandle(m_hCom);
  return FALSE;
} 
SetupComm(m_hCom, 1024, 1024); //接收和发送缓冲区均为1024 byte
PurgeComm(m_hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR); //清空接收和发送缓冲区
SetCommMask(m_hCom, EV_RXCHAR | EV_TXEMPTY); //收到一个字节或者字符发送完就引发事件
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
	memcpy(outbuffer, str,t);                  // t为拷贝的字节数
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	BOOL bWriteStat;
	ClearCommError(m_hCom, &dwErrorFlags, &ComStat);  
	//该函数获得通信错误并报告串口的当前状态ComStat,同时,该函数清除串口的错误标志以便继续输入、输出操作.
	bWriteStat = WriteFile(m_hCom, outbuffer,
		t, &t, &osWrite);             
	if (!bWriteStat)   //   if the result is false
	{
		if (GetLastError() == ERROR_IO_PENDING)  
		{
			WaitForSingleObject(osWrite.hEvent, 1000);  //wait till 
			return t;
		}
		return 0;  //写失败返回失败

	}

	PurgeComm(m_hCom, PURGE_TXABORT |
		PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);   //清空缓冲区
	return t;
	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD SerialPort::receive(unsigned char* strin,int len,int time,int inter)   //接受数据  最多wait 2s
{
	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = 20;  //如果读间隔超时被设置	MAXDWORD 并且读时间系数和读时间常量都为 0, 那么在读一次输入缓冲区的内容后		读操作就立即返回, 而不管是否读入了要求的字符.
	TimeOuts.ReadTotalTimeoutMultiplier = 6;   //读总超时＝ReadTotalTimeoutMultiplier×10＋ReadTotalTimeoutConstant
	TimeOuts.ReadTotalTimeoutConstant = 100;

	TimeOuts.WriteTotalTimeoutMultiplier = 30;//写时间系数
	TimeOuts.WriteTotalTimeoutConstant = 1000;//写时间常量
	SetCommTimeouts(m_hCom, &TimeOuts);

	memset(&osRead, 0, sizeof(OVERLAPPED));
	osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	unsigned char str[50];
	memset(str, '\0', sizeof(str));
	DWORD dwBytesRead = len;//读取的字节数
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


void shijinzhiasciitrans(long a,unsigned char* str)   // 10进制转化为 ascii码
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


unsigned int CRC(unsigned char *SendBuf, int nEnd)//crc校验
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
