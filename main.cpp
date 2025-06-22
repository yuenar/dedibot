#include <QDir>
#include <QSettings>
#include <QApplication>
#include <QStyleFactory>
#include <QSurfaceFormat>
#include "UI/MainWindow.h"
#include "Data/ProjectData.h"
#include <SingleApplication.h>

void Init(int argc,char *argv[])
{
    // 添加空指针检查
    if(ProjectData::GetInstance() == nullptr) {
        qFatal("ProjectData instance creation failed!");
    }

	if(argc>1)
	{
		char exePath[128];
		strcpy(exePath,argv[0]);
		int i,len=strlen(exePath);
		for(i=len-1;i>=0&&exePath[i]!='\\'&&exePath[i]!='/';i--);
		exePath[i]='\0';
		QDir::setCurrent(QString::fromLocal8Bit(exePath));
	}
	QDir dir;
	QString dataPath;
	QString workPath=QDir::currentPath();
	bool success=true;
#ifdef WIN32
	char ladPath[128];
	sprintf(ladPath,"%s",getenv("localappdata"));
	dataPath=QString(QString::fromLocal8Bit(ladPath))+QString("/Dedibot");
#else
	dataPath=QDir::homePath()+QString("/Library/Application Support")+QString("/Dedibot");
#endif
	if(dir.exists(dataPath)==false)
		success=dir.mkdir(dataPath);
    if(success==false)
		dataPath=workPath;
	workPath.replace("\\","/");
	dataPath.replace("\\","/");

    // 添加路径有效性验证
    qDebug() << "Work path:" << workPath;
    qDebug() << "Data path:" << dataPath;
    if(!QDir(dataPath).exists()) {
        qWarning() << "Failed to create data directory!";
    }

    // QSettings setting(dataPath+"/setting.ini",QSettings::IniFormat);
    double l=1,w=500,h=800;
    // int langIndex=setting.value("Setting/Language").toInt();
    // l=setting.value("Box/Length").toDouble();
    // w=setting.value("Box/Width").toDouble();
    // h=setting.value("Box/Height").toDouble();
	if(l>eps&&w>eps&&h>eps)
		ProjectData::GetInstance()->SetBoxSize(l,w,h);
    // ProjectData::GetInstance()->SetLangIndex(langIndex);
	ProjectData::GetInstance()->SetWorkPath((const char*)workPath.toLocal8Bit());
	ProjectData::GetInstance()->SetDataPath((const char*)dataPath.toLocal8Bit());
}
int main(int argc, char *argv[])
{
    //Mesh mesh;
    //Chassis test;
    //test.GetTriMesh(mesh);
    //FileReaderWriter::SaveModel("test.stl",&mesh);
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    // QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts); // 共享OpenGL上下文
	Init(argc,argv);
    // QtSingleApplication::addLibraryPath(".");
    // QtSingleApplication::setStyle(QStyleFactory::create("fusion"));
    // SingleApplication a(argc,argv);
    QApplication a(argc,argv);



    qDebug()<<"windows=========0";
    // if(a.isSecondary())
    // {
    //     a.sendMessage("raise_window_noop");
    //     return EXIT_SUCCESS;
    // }
	
    // MainWindow w;
    // // a.setActivationWindow(&w);

    // w.show();
    // // if(argc>1)
    // //     w.Open(QString::fromLocal8Bit(argv[1]));

    // return a.exec();

    try {
        MainWindow w;
        w.show();
        if(argc>1)
            w.Open(QString::fromLocal8Bit(argv[1]));

        return a.exec();
    }
    catch (const std::exception& e) {
        qFatal("Exception in MainWindow: %s", e.what());
    }
    catch (...) {
        qFatal("Unknown exception occurred");
    }

}
