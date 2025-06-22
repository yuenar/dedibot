#include <QFileInfo>
#include <QTimer>
#include <QMutexLocker>
#include "FileRWThread.h"
#include "FileRW/FileReaderWriter.h"
using namespace std;
FileRWThread::FileRWThread(QObject *parent)
	: QThread(parent)
{
	timer=new QTimer(this);
	fileRW=new FileReaderWriter;
	connect(timer,&QTimer::timeout,this,&FileRWThread::on_timeout);
}

FileRWThread::~FileRWThread()
{
	delete fileRW;
}
void FileRWThread::Open(const QString &filePath)
{
	status.push(FileRWStatus(filePath));
	timer->start(100);
	this->start();
}
void FileRWThread::Open(const std::vector<QString>&filePaths)
{
	int size=filePaths.size();
	for(int i=0;i<size;i++)
		status.push(FileRWStatus(filePaths[i]));
	timer->start(100);
	this->start();
}
void FileRWThread::Save(const QString &filePath,Mesh* mesh)
{
	status.push(FileRWStatus(filePath,mesh,false));
	timer->start(100);
	this->start();
}

void FileRWThread::run()
{
	int size;
	while(!status.empty())
	{
		FileRWStatus &current=status.front();
		QFileInfo file(current.path);
		QString suffix=file.suffix().toLower();
		emit StatusChanged(current);

		if(current.read)
		{
			if(suffix=="dedi")
				current.success=fileRW->Load(current.path.toLocal8Bit());
			else
			{
				current.mesh=new Mesh;
				current.success=fileRW->LoadModel(current.path.toLocal8Bit(),current.mesh);
				if(current.success==false)
					delete current.mesh;
			}
		}
		else
		{
			if(suffix=="dedi")
				current.success=fileRW->Save(current.path.toLocal8Bit());
			else
				current.success=fileRW->SaveModel(current.path.toLocal8Bit(),current.mesh);
		}
		
		QMutexLocker lock(&mutex);
		current.finished=true;
		current.progress=100;
		emit StatusChanged(current);
		status.pop();
	}
}
void FileRWThread::Stop()
{
	if(this->isRunning())
	{
		this->terminate();
		timer->stop();
	}
}
void FileRWThread::on_timeout()
{
 	if(this->isFinished())
	{
		timer->stop();
		return;
	}	
	QMutexLocker lock(&mutex);
	if(!status.empty())
	{
		FileRWStatus &current=status.front();
		float progress=fileRW->GetProgress();
		if(!current.finished&&progress!=current.progress)
		{
			current.progress=progress;
			emit StatusChanged(current);
		}
	}
}
