#ifndef FILERWTHREAD_H
#define FILERWTHREAD_H
#include <QThread>
#include <QMutex>
#include <queue>
struct Mesh;
class QTimer;
class FileReaderWriter;
struct FileRWStatus
{
	QString path;
	Mesh* mesh;
	bool read,finished,success;
	float progress;
	FileRWStatus(QString path_="",Mesh* mesh_=0,bool read_=true,bool finished_=false,bool success_=false,float progress_=0)
		:path(path_),mesh(mesh_),read(read_),finished(finished_),success(success_),progress(progress_)
	{

	}
};
class FileRWThread : public QThread
{
	Q_OBJECT
	
signals:
	void StatusChanged(FileRWStatus status);

private:
	QTimer *timer;
	QMutex mutex;
	FileReaderWriter* fileRW;
	std::queue<FileRWStatus>status;

private slots:
	void on_timeout();

public:
	FileRWThread(QObject *parent);
	~FileRWThread();

	void Open(const QString &filePath);
	void Open(const std::vector<QString>&filePaths);
	void Save(const QString &filePath,Mesh* mesh=0);

	void Stop();
protected:
	void run();

};

#endif // FILERWTHREAD_H
