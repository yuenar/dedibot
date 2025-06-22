#include<QTimer>
#include<QMutexLocker>
#include "SupportGThread.h"
#include "Data/ModelData.h"
#include "Data/MeshList.h"
#include "MeshP/MeshProcess.h"
#include "SupportG/SupportGenerator.h"
#include <fstream>
SupportGThread::SupportGThread(QObject *parent)
	: QThread(parent)
{
	sg=new SupportGenerator;
	timer=new QTimer(this);
	current.sc=new Supporter_Classic;
	connect(timer,&QTimer::timeout,this,&SupportGThread::on_timeout);
}

SupportGThread::~SupportGThread()
{
	delete sg;
	delete current.sc;
}

void SupportGThread::GenerateSupporterClassic(ModelData* md)
{
	current.md=md;
	current.finished=false;
	current.progress=0;
	sg->Reset();
	timer->start(100);
	this->start();
}

void SupportGThread::run()
{
	emit StatusChanged(current);
	sg->SetWallThickness(current.md->GetSupportParam(SupportParam::SupportThickness));
	sg->SetBallRadius(current.md->GetSupportParam(SupportParam::ContactPointSize));
	sg->SetSamplingDistance(current.md->GetSupportParam(SupportParam::PillarSpacing));
	sg->SetChassisThickness(current.md->GetSupportParam(SupportParam::ChassisThickness));
	sg->SetModelHeight(current.md->GetSupportParam(SupportParam::HoistingHeight));//提升高度设置
	sg->SetConeRadius(current.md->GetSupportParam(SupportParam::PillarRadius));
	sg->SetEdgeSamplingDistance(current.md->GetSupportParam(SupportParam::PillarSpacingE));

	Mesh model;
	auto M=current.md->GetTransformMatrix();
	const Mesh* input=MeshList::GetInstance()->GetMesh(current.md->GetModelId());
	MeshProcess::MeshTransform(input,M,&model);
	sg->SetMesh(&model);

	*current.sc=sg->GenerateClassicSupporter();

	QMutexLocker locker(&mutex);//控制this和UI线程读写current
	current.finished=true;
	current.progress=100;
	emit StatusChanged(current);
}

void SupportGThread::Stop()
{
	if(this->isRunning())
	{
		this->terminate();
		timer->stop();
	}
}

void SupportGThread::on_timeout()
{
	if(this->isFinished())
	{
		timer->stop();
		return;
	}
	float progress=sg->GetProgress()*100;
	//static std::ofstream ofs("E:/1.txt");
	//ofs<<progress<<"	"<<current.progress<<std::endl;
	QMutexLocker locker(&mutex);//控制this和UI线程读写current
	if(fabs(progress-current.progress)>eps)
	{
		current.progress=progress;
		if(!current.finished)
			emit StatusChanged(current);
	}
}
