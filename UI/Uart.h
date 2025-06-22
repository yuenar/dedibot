#ifndef UART_H
#define UART_H

#include <QDialog>
#include <QMessageBox>
#include <QFile>
#include <QInputDialog>
#include <QFileDialog>
#include <QTextStream>
#include "FileRW/readgcode.h"
#include "qextserial/qextserialport.h"

namespace Ui {
class Uart;
}

class Uart : public QDialog
{
    Q_OBJECT

public:
    explicit Uart(QWidget *parent = 0);
    ~Uart();
    void readMyCom();
    void beginPrint(const QString &path);
    void addToSender();
    void sendToCom(QString str);
    bool status;
signals:
    void Sig_okmount(unsigned long int);
    void Sig_recvCom(QString);
    void Sig_oktalmount(unsigned long int);
private:
    Ui::Uart *ui;
    /*Com*/
    QextSerialPort *myCom;
    QTimer* checkTimer;
    QTimer* checkTemp;
    readGcode gcode;
    QString tosend;
    bool isPrinting;
    unsigned long int ok_mount;  //  统计ok的数量
private slots:
    void on_openCloseBtn_clicked();
    void on_sendMsgBtn_clicked();
};

#endif // UART_H
