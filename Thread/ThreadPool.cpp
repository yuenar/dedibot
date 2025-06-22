#include "ThreadPool.h"
#include "UI/MainWindow.h"
#include "UI/DediMessageBox.h"
#include "Data/MeshList.h"
#include "Data/ModelData.h"
#include "Data/ProjectData.h"
ThreadPool* ThreadPool::instance=0;
ThreadPool::ThreadPool(MainWindow *mw)
{
	mainW=mw;
	isRunning=false;
	fileRWThread=new FileRWThread(mw);
	supportGThread=new SupportGThread(mw);
	sliceProcess=new SliceProcess(mw);
	
	qRegisterMetaType<FileRWStatus>("FileRWStatus");
	qRegisterMetaType<SliceStatus>("SliceStatus");
	qRegisterMetaType<SupportGStatus>("SupportGStatus");
	connect(fileRWThread,&FileRWThread::StatusChanged,this,&ThreadPool::on_fileRWThread_StatusChanged);
	connect(sliceProcess,&SliceProcess::StatusChanged,this,&ThreadPool::on_sliceProcess_StatusChanged);
	connect(supportGThread,&SupportGThread::StatusChanged,this,&ThreadPool::on_supportGThread_StatusChanged);
}
void ThreadPool::StopAll()
{
	fileRWThread->Stop();
	supportGThread->Stop();
	sliceProcess->Stop();
}

void ThreadPool::on_fileRWThread_StatusChanged(FileRWStatus status)
{
	isRunning=!status.finished;
	QFileInfo info(status.path);
	QString suffix=info.suffix();
	progress=status.progress;
	if(status.finished)
	{
		if(status.read)
		{
			if(status.success==false)
			{
				DediMessageBox oec(OpenErrorCritical,mainW,status.path);
				oec.exec();
				//UpdateEnable();
			}
			else
			{
				if(suffix!="dedi")
				{
					int id=MeshList::GetInstance()->InsertMesh(status.mesh);
					ModelData *md=new ModelData;
					md->SetModelId(id);

					QString fileName=info.fileName();
					int idx=fileName.lastIndexOf(".");
					if(idx>0)
						fileName.truncate(idx);
					md->SetModelName((const char*)fileName.toLocal8Bit());
					ProjectData::GetInstance()->InsertModelData(md);
				}
			}
			emit ThreadStatusChanged();
		}
		else
		{
			emit ThreadStatusChanged();
			DediMessageBox ssi(SaveSuccessInformation,mainW,status.path);
			ssi.exec();
			//UpdateEnable();
		}
	}
	else
	{
		emit ThreadStatusChanged();
	}
}
void ThreadPool::on_sliceProcess_StatusChanged(SliceStatus status)
{
	isRunning=!status.finished;
	progress=status.progress;
	emit ThreadStatusChanged();
	if(status.finished)
	{
		if(status.result==SliceStatus::SliceSuccess)
		{
			DediMessageBox sfi(SliceFinishedInformation,mainW);
			if(sfi.exec()!=QMessageBox::Cancel)
			{
				/*´òÓ¡*/
			}
			
		}
		else if(status.result==SliceStatus::MemoryLimited)
		{
			DediMessageBox mlc(MemoryLimitedCritical,mainW);
			mlc.exec();
		}
		else if(status.result==SliceStatus::SlicerNotFound)
		{
			DediMessageBox snf(SlicerNotFoundCritical,mainW);
			snf.exec();
		}
		else
		{
			DediMessageBox sec(SliceErrorCritical,mainW);
			sec.exec();
		}
		emit ThreadStatusChanged();
		return;
	}
}
void ThreadPool::on_supportGThread_StatusChanged(SupportGStatus status)
{
	isRunning=!status.finished;
	if(status.finished)
		status.md->SetSupportFromSC(status.sc);
	progress=status.progress;
	emit ThreadStatusChanged();
}