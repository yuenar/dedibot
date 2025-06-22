#ifndef SLICEPROCESS_H
#define SLICEPROCESS_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QDir>
#include "common.h"
struct SliceStatus
{
	enum SliceResult
	{
		SliceSuccess,SliceError,MemoryLimited,SlicerNotFound
	};
	SliceResult result;
	bool finished;
	float progress;
};
class SliceProcess : public QProcess
{
	Q_OBJECT
signals:
	void StatusChanged(SliceStatus);
public:
	SliceProcess(QObject *parent=NULL);
	~SliceProcess();

	void SetWorkPath(const QString &path);
	void SetDataPath(const QString &path);
	void SetConfigPath(const QString &path);

	void Slice(const Mesh &mesh);

	void Stop();
private:
	QTimer* timer;
	QString dataPath,workPath,configPath;
	QString exePath,inPath,outPath;

	double zmin,zmax;

	int w,h;
	QString color;
	double thickness;
	int nlayer;
	
	SliceStatus current;
	void UpdateConfig();
	bool CheckStorage();

private slots:

	void on_timeout();
	void on_finished(int exit);
};

#endif // SLICEPROCESS_H
