#include "readgcode.h"
#include <QDebug>
#include <QRegExp>
const unsigned long int linum=256;
readGcode::readGcode(QObject *parent) : QObject(parent)
{
    layers=0;
    g_cmds=0;
    totaltimes=0;
    readNum=0;
    printFile = "test.gcode";
    targetGcode = new QFile("test.gcode");
    in =new  QTextStream(targetGcode);
}

readGcode::~readGcode()
{
    targetGcode->close();
}
void readGcode::setTargetFile(QString file)
{
    layers=0;
    g_cmds=0;
    totaltimes=0;
    printFile = file;
    targetGcode->close();
    targetGcode->setFileName(printFile);
    if(!targetGcode->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"Can't open the g-code file!"<<printFile;
    }
    in->setDevice(targetGcode);
    quint64 pos = in->device()->pos();
    QRegExp regLt(QString(";Layer count: *([0-9]*)"));
    QRegExp regL(QString(";LAYER: *([0-9]*)"));
    QRegExp regT(QString(";TIME: *([0-9]*)"));
    while(!(in->atEnd()))
    {
        QString yi = in->readLine();
        if(yi.startsWith("G"))
        {
            g_cmds++;
        }
        if(yi.contains(regLt))
        {
            if((yi.indexOf(regLt))>=0)
            {
                qDebug()<<"total layers is :"<<regLt.cap(1);
            }
        }
        if(yi.contains(regT))
        {
            if((yi.indexOf(regT)))
            {
                qDebug()<<"total time is :"<<regT.cap(1);
            }
        }
    }
    in->device()->seek(pos);
    qDebug()<<tr("total comands : %1").arg(g_cmds);
}
unsigned long int readGcode::getTotalLayer()
{
    return layers;
}
unsigned long int readGcode::getTotalGcmd()
{
    return g_cmds;
}
unsigned long int readGcode::getTotalTime()
{
    return totaltimes;
}
QString readGcode::readOneLine()
{
    QString pi = in->readLine()+"\n";
    bool goon = true;
    QRegExp reg(QString("(.*) *;"));
    if(in->atEnd())
    {
        currentLine = "B29";
        return currentLine;
    }
    while(goon&&(!in->atEnd()))
    {
        if((pi[0]==';')||(pi[0]==' ')||(pi[0]=='\n'))
        {
            pi = in->readLine()+"\n";
        }
        else
        {
            if(pi.contains(reg))
            {
                int pos = pi.indexOf(reg);
                if(pos>=0)
                    currentLine = reg.cap(1)+"\n";
            }
            else
            {
                currentLine = pi;
            }
            goon = false;
        }
    }
    readNum++;//读行计数器
    //    qDebug()<<"read gcode:"<<currentLine;
    currentGraph.append(currentLine);
    return currentLine;
}
QString readGcode::readGraph()
{
    currentGraph.clear();
    while((readNum%linum)==0)
    {
        readOneLine();
        qDebug()<<"Read next 256 lines cmd...";
    }
        do{
            readOneLine();
        }while((readNum%linum)!=0);
    return currentGraph;
}
QString readGcode::readRest(unsigned long int pos,unsigned long int tal)
{
    currentGraph.clear();
    unsigned long int rst;
    rst=pos-(tal-1)*linum;//从分割点继续读行
    while(rst>0)
    {
        readOneLine();
        if(currentLine=="M84     \n")
        {
            rst=1;
            qDebug()<<"Write rest graph cmd...";
            return currentGraph;
        }
        else
        {
        rst-=1;
        }
    }
    return currentGraph;
}
void readGcode::reset()
{
    currentLine.clear();
    currentGraph.clear();
    layers = 0;
    totaltimes = 0;
    g_cmds = 0;
    targetGcode->close();
}
