#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <string>
#include <iostream>
#include <QDebug>
#include <QFileDialog>
#include <fstream>
#include <sstream>
using namespace  std;
Widget::Widget(QWidget *parent):
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = 0;
    flags |= Qt::WindowMinimizeButtonHint;
    setWindowFlags(Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint);
    receiver = new QUdpSocket(this);
    sender = new QUdpSocket(this);
   // QByteArray ADRESS("192.168.1.104");
    receiver->bind(45454,QUdpSocket::ShareAddress); //useful for udp server.
    thread.start();
    //thread.exit();
    connect(receiver,SIGNAL(readyRead()),
            this,SLOT(readData()));
    comopened = 0;
    ui->closeMyComBtn->setEnabled(false); //开始“关闭串口”按钮不可用
    ui->sendMsgBtn->setEnabled(false); //开始“发送数据”按钮不可用

    ui->filterTypeComboBox->setEnabled(false);
    ui->wcComboBox->setEnabled(false);
    ui->sendFrqCombox->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);


    GetComList_256();
    countEth = 0;
    mode = 0;
    countEth1=0;
    countCom=0;
    QDir dir;
    pathtemp =  dir.currentPath();
    pathtemp+="/datatemforEHT.csv";
    DeleteFile(pathtemp.toStdWString().c_str());

    wayToSend = Com;

/********************************thread*********************************************/


connect(this, SIGNAL(myComOpen(QString)), &thread, SLOT(myComOpen(QString))); // autoconnection which means when the
connect(this, SIGNAL(myComClose()), &thread, SLOT(myComClose()));              //receiver and emmiter is in the same  thread. use directconnection~ excecute in the signal's thread
                                                                              // otherwise, the slot is excute in the receiver's thread(after its control loop) .see in qt help.
connect(&thread,SIGNAL(comDataShow(QString)), this, SLOT(comDataShow(QString)));  // connect the datashow slot function.hfffffttttttttttttttttttttttttttttttttttttttttthh
connect(this, SIGNAL(setFIR(int,int,int,char)), &thread, SLOT(setFIR(int,int,int,char)));
connect(this, SIGNAL(setIIR(int,int,char)), &thread, SLOT(setIIR(int,int,char)));
connect(this, SIGNAL(resetFilter(int,char)), &thread, SLOT(resetFilter(int,char)));
connect(this, SIGNAL(sendFreq(int,char)), &thread, SLOT(sendFreq(int,char)));
connect(this, SIGNAL(sendIP(char*,char*,char)), &thread, SLOT(sendIP(char*,char*,char)));
connect(this, SIGNAL(saveData(QString)), &thread, SLOT(saveData(QString)));
connect(this, SIGNAL(startEnd(bool,char)), &thread, SLOT(startEnd(bool,char)));

}

Widget::~Widget()
{
    delete ui;
}



void Widget::readData()
{
    while(receiver->hasPendingDatagrams())  //拥有等待的数据报
       {
          float f[8];
          unsigned char packege[16]; //接受到的包， packege[0]=0xAB packege[1~2] data packege[3]=0xFB
          //char *data;
          QByteArray datagram; //拥于存放接收的数据报
          //让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
          datagram.resize(receiver->pendingDatagramSize());
          //接收数据报，将其存放到datagram中
          receiver->readDatagram(datagram.data(),datagram.size());
          //将数据报内容显示出来
          //datagram.data();
         // if(datagram.data()[0] != 0xbb)return;
          QString out;
          if(mode == 0)
          {
            memcpy(packege,datagram.data(),sizeof(datagram.size()));  //datagram.data()
             uchar cal = (uchar)packege[0] + (uchar)packege[1] + (uchar)packege[2];
            if(datagram.size() != 4 || (uchar)packege[0] != 0xAB || (uchar)packege[3] != cal)
                return;   //不满足校验
            short temp;
            memcpy(&temp,&packege[1],2);             //截取有用数据
            out = QString("%1").arg(temp);      //
          }
          else if(mode == 1)
          {
              memcpy(f,datagram.data(),sizeof(datagram.size()));  //datagram.data()
              out = QString("%1 %2").arg(f[0]).arg(countEth);
          }
          ui->label->setText(out);

          char p[20];
          itoa(countEth,p,10);
          QString count(p);
           ui->textBrowser->append(count+ ","+out);
          EthernetData.append( count+ ","+out+"\n"); // tab 空格空格？
          countEth++;        // realcount
          countEth1++;       // observecount
           if(countEth1>5000)
           {

                    ofstream filewrite (pathtemp.toLatin1(), ios::app );
                    filewrite << EthernetData.data();

                     EthernetData.clear();
                     countEth1 = 0;
                     ui->textBrowser->clear();
           }
    }
}

void Widget::sendData()
{
}


void Widget::on_pushButton_clicked()
{
//    emit resetFilter(0);
//    char orderwrite[4] = {(char)0xAA,(char)0xBB,(char)0x00,(char)0x65};
//    sender->writeDatagram(orderwrite,4,
//                          QHostAddress("192.168.1.123"),45454);
//    mode = 0;
//    cout <<"main" <<endl;
    //myCom->write(orderwrite);

}


void Widget::on_openMyComBtn_clicked()
{
//    struct PortSettings myComSetting = {BAUD115200,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,500};
//    setComm(myComSetting);
    //
//emit on_firCheckBox_clicked();
    //

         QString portName = ui->portNameComboBox->currentText();  // 获得串口名~
         struct PortSettings myComSetting = {BAUD115200,DATA_8,PAR_NONE,STOP_1,FLOW_OFF,500};
         //thread.myCom = new Win_QextSerialPort(portName,myComSetting,QextSerialBase::EventDriven);
         if(thread.myComOpen(portName,myComSetting,QextSerialBase::EventDriven))
         {
             ui->openMyComBtn->setEnabled(false); //打开串口后“打开串口”按钮不可用
             ui->closeMyComBtn->setEnabled(true); //打开串口后“关闭串口”按钮可用
             ui->sendMsgBtn->setEnabled(true); //打开串口后“发送数据”按钮可用
             ui->filterTypeComboBox->setEnabled(true);
             ui->wcComboBox->setEnabled(true);
             ui->sendFrqCombox->setEnabled(true);
             ui->pushButton_3->setEnabled(true);
             ui->pushButton_4->setEnabled(true);
             comopened = 1;

             emit myComOpen(portName);
         }

         // 产生这个 信号 给线程 叫线程打开串口
}

void Widget::on_closeMyComBtn_clicked()
{

    //thread
    //myCom->close(); //关闭串口，该函数在win_qextserialport.cpp文件中定义
    emit myComClose();
    ui->openMyComBtn->setEnabled(true); //关闭串口后“打开串口”按钮可用
    ui->closeMyComBtn->setEnabled(false); //关闭串口后“关闭串口”按钮不可用
    ui->sendMsgBtn->setEnabled(false); //关闭串口后“发送数据”按钮不可用
    ui->filterTypeComboBox->setEnabled(false);
    ui->wcComboBox->setEnabled(false);
    ui->sendFrqCombox->setEnabled(false);
    ui->pushButton_3->setEnabled(false);
    ui->pushButton_4->setEnabled(false);
    comopened = 0;

}

void Widget::on_sendMsgBtn_clicked()
{
    QString localHostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localHostName);
    QString ipLocal;
    foreach(QHostAddress address,info.addresses())
    {
         if(address.protocol() == QAbstractSocket::IPv4Protocol)
             ipLocal = address.toString();
            //qDebug()<< ipLocal;
    }
    //以ASCII码形式将行编辑框中的数据写入串口
   static char ip[4];//myCom->write(ui->sendMsgLineEdit->text().toLatin1());
   static char ip1[4];//myCom->write(ui->sendMsgLineEdit->text().toLatin1());
    if(IsIPaddress(ui->sendMsgLineEdit->text(),ip) &&
                    IsIPaddress(ipLocal,ip1))
                 emit sendIP(ip,ip1,wayToSend);
    return;
}


void Widget::GetComList_256()//获取可用com口支持到256个
{
    QString  strCom, strComOpen;
    int  nCom = 0;
    int  count = 0;
    HANDLE    hCom;

    ui->portNameComboBox->clear();
    do
    {
        nCom++;
        strCom = QString("COM%1").arg(nCom);
        strComOpen = QString("\\\\.\\COM%1").arg(nCom);
        //QString strVariable1;
        LPCWSTR strVariable2 = (const wchar_t*) strComOpen.utf16();
        hCom = CreateFile( strVariable2, GENERIC_READ | GENERIC_WRITE, 0, NULL,
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
        if (INVALID_HANDLE_VALUE == hCom)
        {
            //DWORD error = ::GetLastError();//取得错误信息
        }
        else
        {
            ui->portNameComboBox->addItem(strCom);
            count++;
        }
        CloseHandle(hCom);
    } while (nCom<256);


    //CCombox->SetCurSel(0);
    //m_CommNum.GetWindowTextA(CFlowMeterAppPtr->CommNum);
    count = 0;
}


void Widget::on_mode2Button_clicked()
{
    char orderwrite[4] = {(char)0xAA,(char)0xBB,(char)0x01,(char)0x66};
    sender->writeDatagram(orderwrite,4,
                          QHostAddress("192.168.1.123"),45454);
    mode = 1;
    //myCom->write(orderwrite,4);
}

void Widget::comDataShow(const QString &data)
{
        ui->textBrowserCom->append(data);
//        char p[6];

//        itoa(countCom,p,10);
//        ComData.append((QString)p +","+ data +"\n");
        countCom++;
   if(countCom > 4999)
    {
        countCom = 0;
        ComData.clear();
        ui->textBrowserCom->clear();
    }
}



bool Widget::IsIPaddress(QString ip, char *ipR)
{
    string str = ip.toStdString();
    uint found =  str.find_first_of('.');
    int before = 0;
    int temp;
    string s = str.substr(before,found - before);
    before = found;
    if(found == string::npos) {QMessageBox::warning(this, tr("warning"),
                                            tr("不是一个有效的IPv4地址，请输入如192.168.1.1格式"),
                                            QMessageBox::Yes);return 0;}
     temp = atoi(s.c_str());
     if (temp < 255 && temp > 0)
         ipR[0] =  (char)temp;
     else
     {
       QMessageBox::warning(this, tr("warning"),
                    tr("不是一个有效的IPv4地址，请输入如192.168.1.1格式"),
                    QMessageBox::Yes);
       return 0;
     }
    for(int i = 1; i <= 2; i++)
    {
      found =  str.find_first_of('.',found+1);
      if(found == string::npos) {QMessageBox::warning(this, tr("warning"),
                                                     tr("不是一个有效的IPv4地址，请输入如192.168.1.1格式"),
                                                     QMessageBox::Yes);return 0;}
      s = str.substr(before+1,found - before-1);
      before = found;
      temp = atoi(s.c_str());
      if (temp < 255 && temp > 0)
          ipR[i] =  (char)temp;
      else
      {
       QMessageBox::warning(this, tr("warning"),
                            tr("不是一个有效的IPv4地址，请输入如192.168.1.1格式"),
                             QMessageBox::Yes);
            return 0;
        }
    }
    found =  str.length();
    if(found == string::npos) {QMessageBox::warning(this, tr("warning"),
                                            tr("不是一个有效的IPv4地址，请输入如192.168.1.1格式"),
                                            QMessageBox::Yes);return 0;}
    s = str.substr(before+1 ,found - before-1);
    before = found;
    temp = atoi(s.c_str());
    if (temp < 255 && temp > 0)
        ipR[3] =  (char)temp;
    else
    {
      QMessageBox::warning(this, tr("warning"),
                   tr("不是一个有效的IPv4地址，请输入如192.168.1.1格式"),
                   QMessageBox::Yes);
        return 0;
    }
    return 1;
}




void Widget::on_filterTypeComboBox_currentIndexChanged(const QString &arg1)
{
    int order = 0;
    int wc = 0;
    int sendFrq = ui->sendFrqCombox->currentIndex();

    if(arg1 == "FIR")
    {
        order = ui->filterOrderCombox->currentIndex();
        wc = ui->wcComboBox->currentIndex();
        emit setFIR(order,wc,sendFrq,wayToSend);
    }
    else if(arg1 == "IIR")
    {
        wc = ui->wcComboBox->currentIndex();

        emit setIIR(wc,sendFrq,wayToSend);
    }
    else
      emit resetFilter(sendFrq,wayToSend);


}



void Widget::on_filterOrderCombox_currentIndexChanged(const QString &arg1)
{

    int order = 0;
    int wc = 0;
    int sendFrq = ui->sendFrqCombox->currentIndex();

     if(ui->filterTypeComboBox->currentText() == "FIR")
     {
         order = ui->filterOrderCombox->currentIndex();
         wc = ui->wcComboBox->currentIndex();
         emit setFIR(order,wc,sendFrq,wayToSend);
     }
     else if(ui->filterTypeComboBox->currentText() == "IIR")
     {
         wc = ui->wcComboBox->currentIndex();

         emit setIIR(wc,sendFrq,wayToSend);
     }
     else
          emit resetFilter(sendFrq,wayToSend);
}

void Widget::on_wcComboBox_currentIndexChanged(int index)
{
    int order = 0;
    int wc = 0;
    int sendFrq = ui->sendFrqCombox->currentIndex();

    if(ui->filterTypeComboBox->currentText() == "FIR")
    {
        order = ui->filterOrderCombox->currentIndex();
        wc = ui->wcComboBox->currentIndex();
        emit setFIR(order,wc,sendFrq,wayToSend);
    }
    else if(ui->filterTypeComboBox->currentText() == "IIR")
    {
        wc = ui->wcComboBox->currentIndex();

        emit setIIR(wc,sendFrq,wayToSend);
    }
    else
        emit resetFilter(sendFrq,wayToSend);
}

void Widget::on_sendFrqCombox_currentIndexChanged(int index)
{

}

void Widget::on_sendFrqCombox_activated(const QString &arg1)
{
    int order = 0;
    int wc = 0;
    int sendFrq = ui->sendFrqCombox->currentIndex();

    if(ui->filterTypeComboBox->currentText() == "FIR")
    {
        order = ui->filterOrderCombox->currentIndex();
        wc = ui->wcComboBox->currentIndex();
        emit setFIR(order, wc, sendFrq , wayToSend);
    }
    else if(ui->filterTypeComboBox->currentText() == "IIR")
    {
        wc = ui->wcComboBox->currentIndex();

        emit setIIR(wc,sendFrq,wayToSend);
    }
    else
        emit resetFilter(sendFrq,wayToSend);
}

void Widget::on_pushButton_2_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("open file"), "com",  tr("csv(*.csv);;Allfile(*.*)"));
    emit saveData(fileName);
    fileName = QFileDialog::getSaveFileName(this, tr("open file"), "eth",  tr("csv(*.csv);;Allfile(*.*)"));

    ofstream filewrite (pathtemp.toLatin1(), ios::app ); //
    filewrite << EthernetData.data();                            //先写入
    CopyFile(pathtemp.toStdWString().c_str(),fileName.toStdWString().c_str(),FALSE); //复制文件
    DeleteFile(pathtemp.toStdWString().c_str());                         //删除原文件
//  ui->textBrowser->data;

}

void Widget::on_filterOrderCombox_activated(const QString &arg1)
{

}

void Widget::on_pushButton_3_clicked()
{
    emit startEnd(1,wayToSend);

}

void Widget::on_pushButton_4_clicked()
{
    emit startEnd(0,wayToSend);
}

void Widget::on_filterTypeComboBox_activated(const QString &arg1)
{

}

void Widget::on_sendMsgLineEdit_cursorPositionChanged(int arg1, int arg2)
{

}




void Widget::on_ehCheckBox_clicked()
{
    ui->comCheckBox->setCheckState(Qt::Unchecked);
    ui->ehCheckBox->setCheckState(Qt::Checked);
    wayToSend = Ethernet;
    ui->sendMsgBtn->setEnabled(true); //打开串口后“发送数据”按钮可用
    ui->filterTypeComboBox->setEnabled(true);
    ui->wcComboBox->setEnabled(true);
    ui->sendFrqCombox->setEnabled(true);
    ui->pushButton_3->setEnabled(true);
    ui->pushButton_4->setEnabled(true);

    ui->closeMyComBtn->setEnabled(false);
    ui->openMyComBtn->setEnabled(false);
}



void Widget::on_comCheckBox_clicked()
{
    ui->comCheckBox->setCheckState(Qt::Checked);
    ui->ehCheckBox->setCheckState(Qt::Unchecked);
    wayToSend = Com;

    if(!comopened)
    {
        ui->sendMsgBtn->setEnabled(false); //开始“发送数据”按钮不可用
        ui->filterTypeComboBox->setEnabled(false);
        ui->wcComboBox->setEnabled(false);
        ui->sendFrqCombox->setEnabled(false);
        ui->pushButton_3->setEnabled(false);
        ui->pushButton_4->setEnabled(false);
         ui->openMyComBtn->setEnabled(true);
         ui->closeMyComBtn->setEnabled(false);
    }
    else
    {
        ui->openMyComBtn->setEnabled(false);
        ui->closeMyComBtn->setEnabled(true);
    }

}
