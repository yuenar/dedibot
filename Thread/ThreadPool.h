#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QObject>
#include <QMutex>
#include "FileRWThread.h"
#include "SliceProcess.h"
#include "SupportGThread.h"
class MainWindow;
class ThreadPool : public QObject
{
	Q_OBJECT

private:
	float progress;
	bool isRunning;
	MainWindow *mainW;
	FileRWThread* fileRWThread;
	SupportGThread* supportGThread;
	SliceProcess* sliceProcess;
	
	static ThreadPool* instance;
	ThreadPool(MainWindow *mw);
signals:
	void ThreadStatusChanged();

private slots:
	void on_fileRWThread_StatusChanged(FileRWStatus status);
	void on_sliceProcess_StatusChanged(SliceStatus status);
	void on_supportGThread_StatusChanged(SupportGStatus status);

public:
	static void InitInstance(MainWindow* mw)
	{
		if(instance)
			delete instance;
		instance=NULL;
		instance=new ThreadPool(mw);
	}
	static ThreadPool* GetInstance()
	{	
		return instance;
	}

	FileRWThread* GetFileRWThread()
	{
		return fileRWThread;
	}
	SupportGThread* GetSupportGThread()
	{
		return supportGThread;
	}
	SliceProcess* GetSliceProcess()
	{
		return sliceProcess;
	}
	float GetProgress()
	{
		return progress;
	}
	bool IsRunning()
	{
		return isRunning;
	}
	void StopAll();
};

#endif // THREADPOOL_H
