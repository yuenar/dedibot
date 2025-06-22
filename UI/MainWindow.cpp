#include <QFileDialog>
#include <QMouseEvent>
#include <QTranslator>
#include <QDesktopWidget>
#include <QStyleFactory>
#include <QFileInfo>
#include <QMetaType>
#include <QMimeData>
#include <QSettings>
#include <fstream>
#include "MainWindow.h"
#include "IconHelper.h"
#include "SliceModelDialog.h"
#include "DediMessageBox.h"
#include "Data/ProjectData.h"
#include "Data/ModelData.h"
#include "Data/MeshList.h"
#include "Thread/ThreadPool.h"
const int MainWindow::margin=5;

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);
	this->setWindowFlags(Qt::FramelessWindowHint);

	this->setAcceptDrops(true);

	ThreadPool::InitInstance(this);
	/*主窗口拉伸缩放*/
	winMax=false;
	leftButtonDown=false;
	leftButtonDownPos=QPoint(0,0);
	preGeometry=this->geometry();
	UpdateCursorType(cursor().pos());

	/*重新布局网格*/
	ui.gridLayout->removeWidget(ui.glwMain_show);
	ui.gridLayout->addWidget(ui.glwMain_show,0,0,1,3);
	ui.wgtMain_layer->raise();
	ui.wgtMain_ctrl->raise();

	/*初始化赋值*/
	ui.vsdMain_layer->setValue(100);
	ui.btnMain_print->setEnabled(false);
	dedibot_zh=new QTranslator(this);
	dedibot_zh->load(":/dedibot_zh.qm");

	//安装事件监听器,让标题栏识别鼠标双击
	ui.wgtMain_upMenu->installEventFilter(this);

	//ui.pgbMain_progress->setStyle(QStyleFactory::create("windowsvista"));
	ui.cbbMain_language->setStyle(QStyleFactory::create("windowsvista"));
	ui.cbbMain_view->setStyle(QStyleFactory::create("windowsvista"));

	/*树形控制栏绑定子窗口*/
	ui.twtMain_edit->Bind();
	ui.cbbMain_language->setCurrentIndex(ProjectData::GetInstance()->GetLangIndex());

	IconHelper::Instance()->SetIcon(ui.btnMain_close,QChar(0xf00d),10);
	IconHelper::Instance()->SetIcon(ui.btnMain_max,QChar(0xf096),10);
	IconHelper::Instance()->SetIcon(ui.btnMain_min,QChar(0xf068),10);
	IconHelper::Instance()->SetIcon(ui.btnMain_print,QChar(0xf04b),10);

	connect(ThreadPool::GetInstance(),&ThreadPool::ThreadStatusChanged,this,&MainWindow::Update);
	connect(ui.twtMain_edit,&EditTreeWidget::UIStatusChanged,this,&MainWindow::Update);
	connect(ui.twtMain_edit,&EditTreeWidget::ItemExpandStateChanged,this,&MainWindow::ResizeControl);
}

MainWindow::~MainWindow()
{
	ProjectData::GetInstance()->Clear();
	MeshList::GetInstance()->Clear();
}

void MainWindow::paintEvent(QPaintEvent*)
{
	ResizeControl();
}

int MainWindow::GetMousePosFlag(QPoint p)
{
	int res=0;
	if(winMax)
		return 0;
	if(p.y()<=margin)
		res+=10;
	else if(p.y()<this->size().height()-margin)
		res+=20;
	else
		res+=30;
	if(p.x()<=margin)
		res+=1;
	else if(p.x()<this->size().width()-margin)
		res+=2;
	else
		res+=3;
	if(res==22)
	{
		res*=10;
		if(p.y()<32)
			res+=1;
		else
			res+=2;
	}
	return res;
}
void MainWindow::SetCursorType(int flag)
{
	Qt::CursorShape cursor;
	switch(flag)
	{
		case 11:
		case 33:
			cursor=Qt::SizeFDiagCursor;break;
		case 13:
		case 31:
			cursor=Qt::SizeBDiagCursor;break;
		case 21:
		case 23:
			cursor=Qt::SizeHorCursor;break;
		case 12:
		case 32:
			cursor=Qt::SizeVerCursor;break;
		case 220:
		case 221:
			cursor=Qt::ArrowCursor;break;
		default:
			// QApplication::restoreOverrideCursor();//恢复鼠标指针性状
			cursor=Qt::ArrowCursor;
			break;

	}
	setCursor(cursor);
}

void MainWindow::UpdateCursorType(QPoint p)
{
	mousePosFlag=GetMousePosFlag(p);
	SetCursorType(mousePosFlag);
}

bool MainWindow::eventFilter(QObject *obj,QEvent *event)
{
	if(event->type()==QEvent::MouseButtonDblClick&&cursor().shape()==Qt::ArrowCursor)
	{
		this->on_btnMain_max_clicked();
		return true;
	}
	return QObject::eventFilter(obj,event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
	if(!winMax)
	{
		QPoint pos=e->globalPos();
		QRect rect=this->geometry();
		int h=rect.height(),w=rect.width();
		int x1=rect.x(),y1=rect.y(),x2=x1+w,y2=y1+h;
		if(leftButtonDown)
		{
			if(mousePosFlag>100)
			{
				if(mousePosFlag==221)
					this->move(pos-leftButtonDownPos);
			}
			else
			{
				if(mousePosFlag/10==1)y2-pos.y()<600?h=600:h=y2-pos.y(),y1=y2-h;
				if(mousePosFlag/10==3)pos.y()-y1<600?h=600:h=pos.y()-y1;
				if(mousePosFlag%10==1)x2-pos.x()<800?w=800:w=x2-pos.x(),x1=x2-w;
				if(mousePosFlag%10==3)pos.x()-x1<800?w=800:w=pos.x()-x1;
				this->move(x1,y1);
				this->resize(w,h);
			}
		}
		else
		{
			UpdateCursorType(e->pos());
		}
	}
	//ui.letMain_size->setText(QString::number(mousePosFlag));
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
	if(e->button()==Qt::LeftButton)
	{
		leftButtonDown=true;
		leftButtonDownPos=e->globalPos()-this->pos();
	}
}

void MainWindow::mouseReleaseEvent(QMouseEvent *)
{
	leftButtonDown=false;
}

void MainWindow::on_btnMain_close_clicked()
{
	/*while(sliceProcess->state()==QProcess::Running)
	{
		sliceProcess->terminate();
		QApplication::processEvents();
	}*/
	this->close();
}
void MainWindow::closeEvent(QCloseEvent *event)
{
	if(MeshList::GetInstance()->GetSize())
	{
		DediMessageBox sfw(SaveFileWarning,this);
		int ret=sfw.exec();
		if(ret==QMessageBox::Yes)
			on_btnMain_save_clicked();
		else if(ret==QMessageBox::Cancel)
		{
			event->ignore();
			return;
		}
	}
	SaveUserSetting();
	ProjectData::GetInstance()->Clear(1);
	MeshList::GetInstance()->Clear(1);
	ThreadPool::GetInstance()->StopAll();
	qApp->exit();
}
void MainWindow::on_btnMain_max_clicked()
{
	if(winMax)
	{
		this->move(preGeometry.x(),preGeometry.y());
		this->resize(preGeometry.width(),preGeometry.height());
		//this->setGeometry(preGeometry);
		IconHelper::Instance()->SetIcon(ui.btnMain_max,QChar(0xf096),10);
		//ui.btnMain_max->setFont(IconHelper::Instance()->GetIconFont());
	}
	else
	{
		preGeometry=this->geometry();
		QRect avaGeometry=qApp->desktop()->availableGeometry();
		this->move(avaGeometry.x(),avaGeometry.y());
		this->resize(avaGeometry.width(),avaGeometry.height());
		IconHelper::Instance()->SetIcon(ui.btnMain_max,QChar(0xf079),10);
	}
	winMax=!winMax;
}

void MainWindow::on_btnMain_min_clicked()
{
	this->showMinimized();
}

/*Drag and drop*/
bool MainWindow::IsUrlsLegal(const QList<QUrl>&urls)
{
	int size=urls.size();
	if(urls.size()==0)
		return false;
	int cnt[10]={0},i,j;
	QString suffixes[]={"dedi","obj","stl"};
	for(i=0;i<size;i++)
	{
		QString url=urls[i].toString();
		QFileInfo info=QFileInfo(url);
		QString suffix=info.suffix().toLower();
		for(j=0;j<4;j++)
			if(suffixes[j]==suffix)
			{
				cnt[j]++;
				break;
			}
		if(j==4)
			return false;
	}
	if(cnt[0]>1||cnt[0]==1&&size>1)
		return false;
	return true;
}

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
	if(IsUrlsLegal(event->mimeData()->urls())&&!ThreadPool::GetInstance()->IsRunning())
		event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent * event)
{
	if(IsUrlsLegal(event->mimeData()->urls())&&!ThreadPool::GetInstance()->IsRunning())
		event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent * event)
{
	if(ThreadPool::GetInstance()->IsRunning())
		return;
	QList<QUrl>urls=event->mimeData()->urls();
	std::vector<QString>filePaths;
	int size=urls.size();
	for(int i=0;i<size;i++)
	{
		filePaths.push_back(urls[i].toLocalFile());
	}
	if(filePaths.size()==1)
		Open(filePaths[0]);
	else
		ThreadPool::GetInstance()->GetFileRWThread()->Open(filePaths);
}

void MainWindow::on_btnMain_open_clicked()
{
	QString path=QFileDialog::getOpenFileName(this,tr("Load File"),"",tr("Dedibot File (*.dedi)"));
	if(path=="")
		return;
	Open(path);
	//UpdateEnable();
}

void MainWindow::Open(const QString &path)
{
	QFileInfo info(path);
	if(info.suffix().toLower()=="dedi")
	{
		if(MeshList::GetInstance()->GetSize()!=0)
		{
			DediMessageBox ofw(OpenFileWarning,this,path);
			if(ofw.exec()==QMessageBox::Cancel)
				return;
		}
		ProjectData::GetInstance()->Clear();
		MeshList::GetInstance()->Clear();
		this->repaint();
	}
	ThreadPool::GetInstance()->GetFileRWThread()->Open(path);
}

void MainWindow::on_btnMain_save_clicked()
{
	QString path=QFileDialog::getSaveFileName(this,tr("Save File"),"",tr("Dedibot File (*.dedi)"));
	if(path=="")
		return;
	QFileInfo info(path);
	if(info.suffix().toLower()!="dedi")
		path+=".dedi";
	ThreadPool::GetInstance()->GetFileRWThread()->Save(path);
	//UpdateEnable();
}

void MainWindow::on_btnMain_import_clicked()
{
    QString path=QFileDialog::getOpenFileName(this,tr("Import Model"),"",tr("3D Model File(*.stl *.obj)"));
	if(path=="")
		return;
	Open(path);
	//UpdateEnable();
}

void MainWindow::SaveUserSetting()
{
	const double *boxSize=ProjectData::GetInstance()->GetBoxSize();
	QString dataPath=QString::fromLocal8Bit(ProjectData::GetInstance()->GetDataPath().c_str());
	QSettings setting(dataPath+"/setting.ini",QSettings::IniFormat);
	setting.setValue("Setting/Language",ui.cbbMain_language->currentIndex());
	setting.setValue("Box/Length",boxSize[0]);
	setting.setValue("Box/Width",boxSize[1]);
	setting.setValue("Box/Height",boxSize[2]);
}

void MainWindow::ResizeControl()
{
	int h=ui.twtMain_edit->GetWidgetHeight()+40;
	h=std::min(h,ui.glwMain_show->size().height());
	ui.wgtMain_ctrl->resize(QSize(250,h));
	if(ui.twtMain_edit->GetNumofExpand()==5)
		IconHelper::Instance()->SetIcon(ui.btnMain_ctrl,QChar(0xf102),20);
	else if(ui.twtMain_edit->GetNumofExpand()==0)
		IconHelper::Instance()->SetIcon(ui.btnMain_ctrl,QChar(0xf103),20);
}

void MainWindow::TranslateUI()
{
	ui.retranslateUi(this);
	ui.twtMain_edit->TranslateUI();
}

void MainWindow::on_btnMain_ctrl_clicked()
{
    QString text=ui.btnMain_ctrl->text();
    if(text==QChar(0xf102))
    {
        ui.twtMain_edit->collapseAll();
        ui.btnMain_ctrl->setText(QChar(0xf103));
    }
    else
    {
        ui.twtMain_edit->expandAll();
        ui.btnMain_ctrl->setText(QChar(0xf102)); 
		ResizeControl();
    }
}

void MainWindow::on_vsdMain_layer_valueChanged(int value)
{
    ui.glwMain_show->setClipLayerNumber(value);
    ui.glwMain_show->update();
}
void MainWindow::on_cbbMain_language_currentIndexChanged(int index)
{
	if(index==1)
		qApp->installTranslator(dedibot_zh);
	else
		qApp->removeTranslator(dedibot_zh);
    ui.glwMain_show->setLanguage(index);
    TranslateUI();
	Update();
}
void MainWindow::on_cbbMain_view_activated(int index)
{
	ui.glwMain_show->setViewDirection(GLWidget::ViewDir(index));
	ui.glwMain_show->update();
}
void MainWindow::on_btnMain_print_clicked()
{
	if(ui.btnMain_print->text()==QChar(0xf04b))
	{
		if(ProjectData::GetInstance()->IsAllVisibleModelPrintable()==false)
		{
			DediMessageBox pec(PrintableErrorCritical,this);
			pec.exec();
			return;
		}
		if(ProjectData::GetInstance()->IsAllVisibleModelHasSupport()==false)
		{
			DediMessageBox nsw(NoSupportWarning,this);
			if(nsw.exec()==QMessageBox::No)
				return;
		}
		QString dataPath=QString::fromLocal8Bit(ProjectData::GetInstance()->GetDataPath().c_str());
		QString workPath=QString::fromLocal8Bit(ProjectData::GetInstance()->GetWorkPath().c_str());
		QString configPath=dataPath+"/config.ini";
		SliceModelDialog smd(this);
		smd.SetConfigPath(configPath);
		if(smd.exec()==QDialog::Accepted)
		{
			Mesh print;
			ProjectData::GetInstance()->GetPrintedMesh(&print);
			SliceProcess* sliceP=ThreadPool::GetInstance()->GetSliceProcess();
			sliceP->SetDataPath(dataPath);
			sliceP->SetWorkPath(workPath+"/FreesteelSlicer");
			sliceP->SetConfigPath(configPath);
			sliceP->Slice(print);
		}
	}
}
void MainWindow::SetProgressBarValue(float value)
{
	int maximum=ui.pgbMain_progress->maximum();
	ui.pgbMain_progress->setValue(value/100.0*maximum+eps);
}

void MainWindow::UpdateEnable()
{
	float progress;
	int nvisible=ProjectData::GetInstance()->GetNumofVisibleModel();
	int size=ProjectData::GetInstance()->GetSize();
	if(ThreadPool::GetInstance()->IsRunning())
	{
		ui.twtMain_edit->SetDisabled(true);
		ui.btnMain_import->setDisabled(true);
		ui.btnMain_open->setDisabled(true);
		ui.btnMain_save->setDisabled(true);
		ui.btnMain_print->setDisabled(true);
		progress=ThreadPool::GetInstance()->GetProgress();
	}
	else
	{
		if(size>0)
			ui.twtMain_edit->SetDisabled(false);
		else
			ui.twtMain_edit->SetDisabled(true);
		ui.btnMain_import->setDisabled(false);
		ui.btnMain_open->setDisabled(false);
		ui.btnMain_save->setDisabled(false);
		if(nvisible>0)
			ui.btnMain_print->setDisabled(false);
		else
			ui.btnMain_print->setDisabled(true);
		progress=100;
	}
	SetProgressBarValue(progress);
}
void MainWindow::Update()
{
    QString size;
    ModelData* model=ProjectData::GetInstance()->GetSelectModel();
    if(model)
    {
        const double* gs=model->GetGeometrySize();
		size=QString::number(gs[0],'f',2)+"x"+QString::number(gs[1],'f',2)+"x"+QString::number(gs[2],'f',2);
    }
    else
        size="-";
    ui.letMain_size->setText(size);

	this->UpdateEnable();
    ProjectData::GetInstance()->Update();//更新模型颜色及名字信息
    ui.twtMain_edit->Update();
    ui.glwMain_show->update();
}

void MainWindow::on_comBtn_clicked()
{
    com.show();
}
