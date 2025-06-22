#include<algorithm>
#include<QSettings>
#include<QMessageBox>
#include<QStorageInfo>
#include "SliceProcess.h"
#include "FileRW/FileReaderWriter.h"

SliceProcess::SliceProcess(QObject *parent)
	: QProcess(parent),thickness(0.045),color("black"),w(912),h(1140),configPath("")
{
	timer=new QTimer(this);

	connect(timer,&QTimer::timeout,this,&SliceProcess::on_timeout);
	connect(this,static_cast<void (SliceProcess::*)(int)>(&SliceProcess::finished),this,static_cast<void (SliceProcess::*)(int)>(&SliceProcess::on_finished));
}
SliceProcess::~SliceProcess()
{

}
void SliceProcess::SetConfigPath(const QString& path)
{
	configPath=path;
}
void SliceProcess::UpdateConfig()
{
	/*read*/
	QString resolution;
	QSettings config(configPath,QSettings::IniFormat);
	thickness=config.value("Slice/SliceThickness").toDouble();
	color=config.value("Slice/ImageColor").toString();
	resolution=config.value("Slice/ImageResolution").toString();
	QStringList sl=resolution.split("x");
	if(sl.size()==2)
		w=sl[0].toInt(),h=sl[1].toInt();
	thickness/=1000.0;

	/*write*/
	config.setIniCodec("UTF8");
	config.setValue("Slice/Zmin",zmin);
	config.setValue("Slice/ZMax",zmax);
	config.setValue("Slice/InputPath",inPath);
	config.setValue("Slice/OutputPath",outPath);
}
void SliceProcess::SetWorkPath(const QString &path)
{
	workPath=path;
	exePath=path+"/slice.exe";
}
void SliceProcess::SetDataPath(const QString &path)
{
	dataPath=path;
	QString stl=path+"/stl",bmp=path+"/bmp";

	QDir dir;
	dir.mkdir(stl);
	dir.mkdir(bmp);

	dir.setCurrent(bmp);
	dir.removeRecursively();

	inPath=stl+"/1.stl";
	outPath=bmp+"/1.bmp";
}
bool SliceProcess::CheckStorage()
{
	QStorageInfo storageInfo(dataPath);
	qint64 need=5*1024*1024*nlayer;
	qint64 available=storageInfo.bytesAvailable();
	if(need>available)
		return false;
	return true;
}
void SliceProcess::Slice(const Mesh &mesh)
{
	QDir dir;
	current.result=SliceStatus::SliceSuccess;
	if(CheckStorage()==false)
		current.result=SliceStatus::MemoryLimited;
	else if(dir.exists(exePath)==false)
		current.result=SliceStatus::SlicerNotFound;
	if(current.result!=SliceStatus::SliceSuccess)
	{
		current.finished=true;
		current.progress=100;
		emit StatusChanged(current);
		return;
	}

	current.finished=false;
	current.progress=0;
	emit StatusChanged(current);

	zmin=0,zmax=0;
	int psize=mesh.points.size();
	if(psize>0)
		zmin=zmax=mesh.points[0][2];
	for(int i=0;i<psize;i++)
	{
		zmin=std::min(zmin,mesh.points[i][2]);
		zmax=std::max(zmax,mesh.points[i][2]);
	}

	UpdateConfig();
	nlayer=ceil((zmax-zmin)/thickness);

	dir.setPath(outPath);
	dir.removeRecursively();

	FileReaderWriter frw;
	frw.SaveModel(inPath.toLocal8Bit().data(),&mesh);

	QStringList paramlist=QStringList()<<"-z"<<QString::number(zmin)+","+QString::number(zmax)+","+QString::number(thickness)
		<<"--background"<<color<<"--core"<<(color=="black"?"white":"black")<<"--cavity"<<color<<"-l0.01"
		<<inPath<<"-w"+QString::number(w)<<"-h"+QString::number(h)<<"-o"<<outPath;

	this->start(exePath,paramlist);
	timer->start(100);
}
void SliceProcess::on_timeout()
{
	if(this->state()==QProcess::NotRunning)
	{
		timer->stop();
		return;
	}
	QString filePath;
	QDir d=QDir(dataPath+"/bmp");
	d.setFilter(QDir::Files);
	float progress=d.count()*100.0/nlayer;
	if(progress-current.progress>0.5)
	{
		current.progress=progress;
		emit StatusChanged(current);
	}
}
void SliceProcess::Stop()
{
	if(this->state()==QProcess::Running)
	{
		this->terminate();
		timer->stop();
	}
}
void SliceProcess::on_finished(int exit)
{
	int count=0;
	current.finished=true;
	current.progress=100;
	timer->stop();
	if(this->exitStatus()==QProcess::NormalExit)
	{
		QDir d=QDir(dataPath+"/bmp");
		d.setFilter(QDir::Files);
		count=d.count();
	}
	else
		return;
	if(count+3>=nlayer)
		current.result=SliceStatus::SliceSuccess;
	else
		current.result=SliceStatus::SliceError;
	emit StatusChanged(current);
}
