#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QThread>
#include "win_qextSerialPort.h"
#include <QtNetwork>
#include <QMutex>
#define Com      0
#define Ethernet 1
class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = 0);
    void stop();
    bool myComOpen(const QString & name, const PortSettings& settings, QextSerialBase::QueryMode mode = QextSerialBase::Polling);
protected:
    void run();

private:
    volatile bool stopped;
    bool received;
    QByteArray comData;
    Win_QextSerialPort *myCom;
    long int countCom;
    int countCom1;
    QMutex comMutex;
    QString pathtemp;
    char notshow;
    char waitsignal;
    QUdpSocket *sender;
    UINT32 destIP;
    //char destIP[4];
signals:
  //  void stringChanged(const QString &);
    void comDataShow(const QString &);
public slots:

    void myComOpen(const QString &portName);
    void readMyCom();                           // 读串口
    void myComClose();                          // 关闭
    void sendIP(char*, char*, char wayToSend);
    void setFIR(int,int,int,char wayToSend);  //设置FIR 参数为阶数和截止频率。
    void setIIR(int,int,char wayToSend);      //设置IIR 参数为截止频率。
    void resetFilter(int sendFreq,char wayToSend);    //取消数字滤波器。
    void sendFreq(int freq,char wayToSend);    //设置发送频率。
    void saveData(QString fileName);
    void startEnd(bool t,char wayToSend);
    void sendstopsignal();

};
UINT16 CRC(char *SendBuf, int nEnd);
#endif // MYTHREAD_H
