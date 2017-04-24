#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "win_qextserialport.h"
#include "mythread.h"
namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
       bool IsIPaddress(QString ip, char *ipR);
    void GetComList_256();
private:
    Ui::Widget *ui;
    QUdpSocket *receiver;
    QUdpSocket *sender;
    MyThread thread;
    QHostAddress deviceAddr;

    QByteArray ComData;
    QByteArray EthernetData;
    int mode ;
    QString pathtemp;
    long int countEth;  //绝对的采集数量
    int countEth1;      //5000的
    int countCom;
    char wayToSend;
    BOOL comopened;
private slots:
void readData();
void sendData();

void on_pushButton_clicked();
void on_openMyComBtn_clicked();
void on_closeMyComBtn_clicked();
void on_sendMsgBtn_clicked();
void on_mode2Button_clicked();

void comDataShow(const QString& data);


void on_filterTypeComboBox_currentIndexChanged(const QString &arg1);


void on_filterOrderCombox_currentIndexChanged(const QString &arg1);

void on_wcComboBox_currentIndexChanged(int index);

void on_sendFrqCombox_currentIndexChanged(int index);

void on_sendFrqCombox_activated(const QString &arg1);

void on_pushButton_2_clicked();

void on_filterOrderCombox_activated(const QString &arg1);

void on_pushButton_3_clicked();

void on_pushButton_4_clicked();

void on_filterTypeComboBox_activated(const QString &arg1);

void on_sendMsgLineEdit_cursorPositionChanged(int arg1, int arg2);



void on_ehCheckBox_clicked();


void on_comCheckBox_clicked();

signals:
void myComOpen(const QString &myComName);
void myComClose();
void sendIP(char*,char*,char wayToSend);
void setFIR(int,int,int,char wayToSend);  //设置FIR 参数为阶数和截止频率。
void setIIR(int,int,char wayToSend);      //设置IIR 参数为截止频率。
void resetFilter(int,char wayToSend);    //取消数字滤波器。
void sendFreq(int,char wayToSend);    //设置发送频率。
void saveData(QString);
void startEnd(bool,char wayToSend);

};

#endif // WIDGET_H
