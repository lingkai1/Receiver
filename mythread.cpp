#include "mythread.h"
#include <QDebug>
#include <QPainter>
#include <iostream>
#include <fstream>
#include <QFileDialog>
using namespace std;
MyThread::MyThread(QObject *parent) :
    QThread(parent)
{
  received=0;
  sender = new QUdpSocket(this);
  QDir dir;
  pathtemp =  dir.currentPath();
  pathtemp+="/datatemfor.csv";
  DeleteFile(pathtemp.toStdWString().c_str());
  moveToThread(this);
  countCom=0;
  countCom1=0;
  waitsignal=0;
  notshow=1;
  destIP = 0xC0A8017B;//192.168.1.123;

}


void MyThread::run()
{
    long int i = 0;
    //while(1);
    exec();
         qDebug() << QString("in MyThread: %1").arg(i++);
  //  exit();
//    while (!stopped)
//    {
//        QString str = QString("in MyThread: %1").arg(i);
//        //emit stringChanged(str);
//        msleep(1000);
//        i++;
//    }
//    stopped = false;
}

void MyThread::myComOpen(const QString &portName)
{
    connect(myCom,SIGNAL(readyRead()),this,SLOT(readMyCom()));
}



void MyThread::stop()
{
    stopped = true;
}

bool MyThread::myComOpen(const QString &name, const PortSettings &settings, QextSerialBase::QueryMode mode)
{
    bool i;
    comMutex.lock();
    myCom = new Win_QextSerialPort(name,settings,mode);
    i = myCom->open(QIODevice::ReadWrite);
    comMutex.unlock();
    myCom->flush();
    return i;
}


void MyThread::readMyCom()
{
    QByteArray recv = myCom->readAll();
    myCom->flush();
    if(recv.size() >= 5)
    {
        if(recv.data()[0] == (char)0xAA && recv.data()[1] == (char)0xBB && recv.data()[6] == (char)0xFA && recv.data()[7] == (char)0xFA)
            received = 1;
        return;
    }
    uchar cal = (uchar)recv.data()[0] + (uchar)recv.data()[1] + (uchar)recv.data()[2];
    if(recv.size() != 4 || (uchar)recv.data()[0] != 0xAB || (uchar)recv.data()[3] != cal)
    {
        return;
    }
    //while(1);
    short data;
    memcpy(&data,&recv.data()[1],2); //截取有用数据
    if(data > 35000 || data < -35000)
        return;

    char p[20];
    itoa(data,p,10);
    QString temp(p);//转化为QSTRING字符串
    itoa(countCom,p,10);
    QString count(p);

    comData.append( count+ ","+temp+"\n"); // tab 空格空格？
    countCom++;
    countCom1++;
     if(countCom1>4999)
    {
     ofstream filewrite (pathtemp.toLatin1(), ios::app );
     filewrite << comData.data();

      comData.clear();
      countCom1 = 0;
     }
     //将串口的数据显示在窗口的文本浏览器中

    if(!notshow)
     emit comDataShow(count+ ","+temp);           //触发这个 消息
    myCom->flush();
}

void MyThread::myComClose()
{
    myCom->close();
}

void MyThread::sendIP(char* ip, char* ip1, char wayToSend)
{

     cout << "setIp   "  <<endl;
    char orderwrite[] = {0xAA, 0xBB, 0x10, (char)ip[0],(char)ip[1],(char)ip[2],(char)ip[3],
                       (char)ip1[0],(char)ip1[1],(char)ip1[2],(char)ip1[3],0xFA,0xFA
                       };
    ushort CRCtemp = CRC(orderwrite,11); // produce CRC
    orderwrite[11] = (char)(CRCtemp>>8);  //高位
    orderwrite[12] = (char)(CRCtemp);     //低位

if(wayToSend == Com)
    myCom->write(orderwrite,13);
else
    sender->writeDatagram(orderwrite,13,QHostAddress(destIP),45454);
}

void MyThread::setFIR(int order, int wc, int sendFrq, char wayToSend)
{
  if(sendFrq < 2)
      notshow=1;
  else
      notshow=0;
  cout << "setFIR   "<< order << wc << sendFrq<<endl;
  char orderWrite[] = {(char)0xAA, (char)0xBB, (char)0x20, (char)sendFrq, (char)order, (char)wc, (char)0xFA, (char)0xFA};
  ushort CRCtemp = CRC(orderWrite,6); // produce CRC
  orderWrite[6] = (char)(CRCtemp>>8);  //高位
  orderWrite[7] = (char)(CRCtemp);     //低位
  if(wayToSend == Ethernet) //如果用以太网 发后直接返回
  {
      sender->writeDatagram(orderWrite,8,QHostAddress(destIP),45454);
     return;
  }
  int ct = 0;
  while(!received)
   {
    myCom->write(orderWrite,8);
    myCom->flush();
    myCom->waitForReadyRead(2);
    ct ++;
    if(ct > 5)
        break;
   }
  myCom->flush();

  received = 0;
}

void MyThread::setIIR(int wc, int sendFrq, char wayToSend)
{
    if(sendFrq < 2)
        notshow=1;
    else
        notshow=0;
  cout << "setIIR   " << wc << sendFrq <<endl;
  char orderWrite[] = {(char)0xAA, (char)0xBB, (char)0x30, (char)sendFrq, (char)0x00 , (char)wc, (char)0xFB, (char)0xFA};
  ushort CRCtemp = CRC(orderWrite,6); // produce CRC
  orderWrite[6] = (char)(CRCtemp>>8);  //高位
  orderWrite[7] = (char)(CRCtemp);     //低位

  if(wayToSend == Ethernet) //如果用以太网 发后直接返回
  {
      sender->writeDatagram(orderWrite,8,QHostAddress(destIP),45454);
     return;
  }
  int ct = 0;
  while(!received)
   {

    myCom->write(orderWrite,8);
    myCom->flush();
    myCom->waitForReadyRead(2);
    ct ++;
    if(ct > 5)
        break;
   }
    received = 0;
     myCom->flush();

}

void MyThread::resetFilter(int sendFrq, char wayToSend)
{
    if(sendFrq < 2)
        notshow=1;
    else
        notshow=0;
    cout << "REST FILTER   " <<endl;
    char orderWrite[] = {(char)0xAA, (char)0xBB, (char)0x40, (char)sendFrq, (char)0x00 , (char)0x00, (char)0xFB, (char)0xFA};
    ushort CRCtemp = CRC(orderWrite,6);
    orderWrite[6] = (char)(CRCtemp>>8);  //高位
    orderWrite[7] = (char)(CRCtemp);     //低位

    if(wayToSend == Ethernet) //如果用以太网 发后直接返回
    {
        sender->writeDatagram(orderWrite,8,QHostAddress(destIP),45454);
       return;
    }

    int ct = 0;
        while(!received)
         {
          myCom->write(orderWrite,8);
          myCom->flush();
           myCom->waitForReadyRead(2);
          ct ++;
          if(ct > 5)
              break;
         }
          received = 0;
         // msleep(1008611);
          myCom->flush();

}



void MyThread::sendFreq(int freq, char wayToSend)
{
    cout << freq;

}

void MyThread::saveData(QString fileName)
{
    ofstream filewrite (pathtemp.toLatin1(), ios::app ); //
    filewrite << comData.data();                             //先写入
    CopyFile(pathtemp.toStdWString().c_str(),fileName.toStdWString().c_str(),FALSE); //复制文件
    DeleteFile(pathtemp.toStdWString().c_str());                         //删除原文件
}

void MyThread::startEnd(bool t, char wayToSend)
{
   cout << "REST FILTER   " <<endl;
   char orderWrite[] = {(char)0xAA, (char)0xBB, (char)t, (char)0x00, (char)0x00 , (char)0x00, (char)0xFB, (char)0xFA};

   ushort CRCtemp = CRC(orderWrite,6);
   orderWrite[6] = (char)(CRCtemp>>8);  //高位
   orderWrite[7] = (char)(CRCtemp);     //低位

   if(wayToSend == Ethernet) //如果用以太网 发后直接返回
   {
       sender->writeDatagram(orderWrite,8,QHostAddress(destIP),45454);
      return;
   }

   int ct = 0;
   while(!received)
   {
      myCom->write(orderWrite,8);
      myCom->flush();
      ct ++;
      if(ct > 5)
            break;
       }
   received = 0;
   myCom->flush();

}

void MyThread::sendstopsignal()
{
    waitsignal=!waitsignal;
}


UINT16 CRC(char *SendBuf, int nEnd)
{
    UINT16 wCRC = 0xFFFF;
    int i = 0;
    for (i = 0; i<nEnd; i++)
    {
        UINT8 temp = SendBuf[i];
        wCRC ^= (UINT16)temp;
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
