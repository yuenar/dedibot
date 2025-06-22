#include "Uart.h"
#include "ui_Uart.h"
#include <QTimer>
#include <QDebug>
#pragma execution_character_set("utf-8")
const unsigned long int linum=256;
Uart::Uart(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Uart)
{
    ui->setupUi(this);
    ui->sendMsgBtn->setDisabled(true);
    myCom = NULL;//初始化为空
    status=false;//默认关闭
    isPrinting = false;
    ok_mount = 0;
    checkTimer = new QTimer();
    checkTemp = new QTimer();
#ifdef Q_OS_LINUX
    ui->portNameComboBox->addItem( "ttyUSB0");
    ui->portNameComboBox->addItem( "ttyUSB1");
    ui->portNameComboBox->addItem( "ttyUSB2");
    ui->portNameComboBox->addItem( "ttyUSB3");
    ui->portNameComboBox->addItem( "ttyS0");
    ui->portNameComboBox->addItem( "ttyS1");
    ui->portNameComboBox->addItem( "ttyS2");
    ui->portNameComboBox->addItem( "ttyS3");
    ui->portNameComboBox->addItem( "ttyS4");
    ui->portNameComboBox->addItem( "ttyS5");
    ui->portNameComboBox->addItem( "ttyS6");
#elif defined (Q_OS_WIN)
    ui->portNameComboBox->addItem("COM0");
    ui->portNameComboBox->addItem("COM1");
    ui->portNameComboBox->addItem("COM2");
    ui->portNameComboBox->addItem("COM3");
    ui->portNameComboBox->addItem("COM4");
    ui->portNameComboBox->addItem("COM5");
    ui->portNameComboBox->addItem("COM6");
    ui->portNameComboBox->addItem("COM7");
    ui->portNameComboBox->addItem("COM8");
    ui->portNameComboBox->addItem("COM9");
    ui->portNameComboBox->addItem("COM10");
    ui->portNameComboBox->addItem("COM11");
    ui->portNameComboBox->addItem("COM12");
    ui->portNameComboBox->addItem("COM13");
    ui->portNameComboBox->addItem("COM14");
    ui->portNameComboBox->addItem("COM15");

#endif
}

Uart::~Uart()
{
    if(myCom != NULL){
        if(myCom->isOpen()){
            myCom->close();
        }
        delete myCom;
    }
    delete ui;
}

void Uart::on_openCloseBtn_clicked()
{
    if(false==status)
    {//关闭时则打开
        QString portName = ui->portNameComboBox->currentText();   //获取串口名
#ifdef Q_OS_LINUX
        myCom = new QextSerialPort("/dev/" + portName);
#elif defined (Q_OS_WIN)
        myCom = new QextSerialPort(portName);
#endif
        //        connect(myCom, SIGNAL(readyRead()), this, SLOT(readMyCom()));

        //设置波特率
        myCom->setBaudRate((BaudRateType)ui->baudRateComboBox->currentText().toInt());

        //设置数据位
        myCom->setDataBits((DataBitsType)ui->dataBitsComboBox->currentText().toInt());

        //设置校验
        switch(ui->parityComboBox->currentIndex()){
        case 0:
            myCom->setParity(PAR_NONE);
            break;
        case 1:
            myCom->setParity(PAR_ODD);
            break;
        case 2:
            myCom->setParity(PAR_EVEN);
            break;
        default:
            myCom->setParity(PAR_NONE);
            qDebug("set to default : PAR_NONE");
            break;
        }

        //设置停止位
        switch(ui->stopBitsComboBox->currentIndex()){
        case 0:
            myCom->setStopBits(STOP_1);
            break;
        case 1:
#ifdef Q_OS_WIN
            myCom->setStopBits(STOP_1_5);
#endif
            break;
        case 2:
            myCom->setStopBits(STOP_2);
            break;
        default:
            myCom->setStopBits(STOP_1);
            qDebug("set to default : STOP_1");
            break;
        }
        //设置数据流控制
        myCom->setFlowControl(FLOW_OFF);//无数据流控制
        //设置延时
        myCom->setTimeout(200);//设置延时

        if(myCom->open(QIODevice::ReadWrite)){
            QMessageBox::information(this, tr("打开成功"), tr("已成功打开串口") + portName, QMessageBox::Ok);
            ui->openCloseBtn->setText("关闭");
            ui->sendMsgBtn->setEnabled(true);
            status=true;//标志位
            qDebug()<<"helloworld!!";
            checkTemp->start(5000);
            checkTimer->stop();
        }else{
            QMessageBox::critical(this, tr("打开失败"), tr("未能打开串口 ") + portName + tr("\n该串口设备不存在或已被占用"), QMessageBox::Ok);
            ui->sendMsgBtn->setDisabled(true);
            return;
        }
    }
    else
    {
        myCom->close();
        delete myCom;
        myCom = NULL;
        isPrinting=false;
        ok_mount = 0;
        gcode.reset();
        ui->openCloseBtn->setText("打开");
        ui->sendMsgBtn->setDisabled(true);
        QMessageBox::information(this, tr("关闭成功"), tr("已成功关闭串口"), QMessageBox::Ok);
        status=false;
    }
}
void Uart::readMyCom()
{
    //读取数据
    QByteArray ba = myCom->readAll();
    if(!ba.isEmpty())
    {
        QString le = ba;
        qDebug()<<"SerialCom receive:"<<le;
        if(true==isPrinting)
        {
            ok_mount++;
            emit Sig_okmount(ok_mount);
        }
    }
}

void Uart::on_sendMsgBtn_clicked()
{       
    isPrinting=false;
    myCom->flush();//先清空缓存
    ok_mount = 0;
    sendToCom("B29 \n");
    gcode.reset();
    ui->openCloseBtn->setDisabled(true);
    ui->sendMsgBtn->setDisabled(true);
    QString path=QFileDialog::getOpenFileName(this, tr("读取文件"), 0, tr("切片文件 (*.gco *.gcode)"));
    if(false==path.isEmpty())
    {
        beginPrint(path);//开始打印
        ui->openCloseBtn->setEnabled(true);
        isPrinting=true;
    }
    else
    {
        ui->openCloseBtn->setEnabled(true);
        ui->sendMsgBtn->setEnabled(true);
    }
}
void Uart::sendToCom(QString str)
{
    char *pt ;
    int write_nu=0;
    QByteArray ba = str.toLatin1();
    pt = ba.data();
    write_nu = myCom->write(ba);
    qDebug()<<QObject::tr("写入命令 :%1 ").arg(str);
}
void Uart::beginPrint(const QString &path)
{   
    gcode.setTargetFile(path);
    unsigned long int po = gcode.getTotalGcmd();
    unsigned long int absCount=po/linum;
    unsigned long int tal=absCount;
    qDebug()<<"大概要分成"<<absCount<<"个段落需发送至打印机";
    emit Sig_oktalmount(po);
    sendToCom("M28 \n");
    QByteArray tst = myCom->readAll();
    if(tst.isEmpty())
    {
        qDebug()<<"读取串口第一段回执失败";
        return;
    }
    else
    {
        qDebug()<<"读取串口第一段回执成功，内容为："<<tst;
        QTimer::singleShot(220, this, SLOT(readMyCom()));
        do{
            readMyCom();
            addToSender();
            absCount-=1;
            qDebug()<<"还剩"<<absCount<<"个段落需发送至打印机";
            sendToCom(tosend);
        }
        while(absCount!=0);
    }
    if(absCount==0)
    {
        tosend = gcode.readRest(po,tal)+"@";
        sendToCom(tosend);
    }
    qDebug()<<"开始读取串口最后一段回执";
    readMyCom();
    sendToCom("B29 \n");
    qDebug()<<"读取完成，请自行比对内容";
}
void Uart::addToSender()
{
    tosend = gcode.readGraph()+"@";
    if(tosend=="")
    {
        tosend = "B29 \n";
        ok_mount = 0;
        emit Sig_okmount(ok_mount);
    }
    else
    {
        if(!tosend.startsWith(";"))
        {
            isPrinting = true;
        }
        else
        {
            isPrinting = false;
        }
    }
}
