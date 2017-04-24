#pragma once
#include "afxwin.h"
//class MyThread;
class serials :
	public CWinThread
{
public:
	//DECLARE_DYNCREATE(serials)
	serials();
	~serials();

	//MyThread* ptrflow;
	//void setOwner(MyThread* ptrDialog)
	//{
	//	ptrflow = ptrDialog;
	//}          // Assign the pointer to pointerDlg is as agent 
	// of Class CSerialAppDlg.
};


