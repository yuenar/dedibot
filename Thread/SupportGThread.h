#ifndef SUPPORTGTHREAD_H
#define SUPPORTGTHREAD_H

#include <QThread>
#include <QMutex>
struct Mesh;
class QTimer;
class ModelData;
class SupportGenerator;
struct Supporter_Classic;
struct SupportGStatus
{
	bool finished;
	float progress;
	ModelData* md;
	Supporter_Classic* sc;
};
class SupportGThread : public QThread
{
	Q_OBJECT
signals:
	void StatusChanged(SupportGStatus);

private slots:
	void on_timeout();

public:
	SupportGThread(QObject *parent);
	~SupportGThread();

	void GenerateSupporterClassic(ModelData* md);

	void Stop();
protected:
	void run();

private:
	QTimer *timer;
	QMutex mutex;
	SupportGenerator* sg;

	SupportGStatus current;
};

#endif // SUPPORTGTHREAD_H
