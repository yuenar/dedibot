#ifndef READGCODE_H
#define READGCODE_H
#include <QObject>
#include <QFile>
#include <QTimer>
#include <QTextStream>
class readGcode : public QObject
{
    Q_OBJECT
public:
    explicit readGcode(QObject *parent = 0);
    ~readGcode();
    QString readOneLine();//读取单行
    QString readGraph();//读取段落
    QString readRest(unsigned long int pos,unsigned long int tal);//读取尾端
    void setTargetFile(QString file);
    void reset();
    unsigned long int  readNum;
    unsigned long int  getTotalLayer();
    unsigned long int  getTotalGcmd();
    unsigned long int  getTotalTime();
signals:

public slots:

private slots:


private:
    QFile* targetGcode;
    QString printFile;

    QTextStream* in;
    QString currentLine;
    QString currentGraph;
    unsigned long int layers;
    unsigned long int g_cmds;
    unsigned long int totaltimes;
};

#endif // READGCODE_H
