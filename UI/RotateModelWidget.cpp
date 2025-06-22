#include "RotateModelWidget.h"
#include "DediMessageBox.h"
#include "Data/ProjectData.h"
#include "Data/ModelData.h"
RotateModelWidget::RotateModelWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	dataChanged=true;
	setDisabled(true);

    connect(ui.btnRotate_front,&QPushButton::clicked,this,&RotateModelWidget::BtnRotateClicked);
    connect(ui.btnRotate_back,&QPushButton::clicked,this,&RotateModelWidget::BtnRotateClicked);
    connect(ui.btnRotate_left,&QPushButton::clicked,this,&RotateModelWidget::BtnRotateClicked);
    connect(ui.btnRotate_right,&QPushButton::clicked,this,&RotateModelWidget::BtnRotateClicked);
    connect(ui.btnRotate_top,&QPushButton::clicked,this,&RotateModelWidget::BtnRotateClicked);
    connect(ui.btnRotate_down,&QPushButton::clicked,this,&RotateModelWidget::BtnRotateClicked);
}

RotateModelWidget::~RotateModelWidget()
{

}
void RotateModelWidget::hsdRotateProcess(Axis axis,const double value)
{
	if(dataChanged==false)
		return;
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	if(md)
	{
		double v=value;
		if(value>360||value<0)
			v=value-floor(value/360)*360; 
		if(md->GetSupportId()>=0&&axis!=Axis::AxisZ)
		{
			DediMessageBox rsw(RemoveSupportWarning,this->topLevelWidget());
			if(rsw.exec()==QMessageBox::No)
				return;
			md->RemoveSupport();
		}
		md->SetRotateValue(axis,int(v+0.5+eps));
		emit UIStatusChanged();
	}
}
void RotateModelWidget::btnRotateProcess(double rx,double ry,double rz)
{
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	if(md)
	{
		if(md->GetSupportId()>=0)
		{
			DediMessageBox rsw(RemoveSupportWarning,this->topLevelWidget());
			if(rsw.exec()==QMessageBox::No)
			{
				this->setFocus();
				return;
			}
			md->RemoveSupport();
		}	
		md->SetRotateValue(Point3D(rx,ry,rz));
		emit UIStatusChanged();
	}
}
void RotateModelWidget::on_hsdRotate_x_valueChanged(int value)
{
	hsdRotateProcess(AxisX,value);
}
void RotateModelWidget::on_hsdRotate_y_valueChanged(int value)
{
	hsdRotateProcess(AxisY,value);
}
void RotateModelWidget::on_hsdRotate_z_valueChanged(int value)
{
	hsdRotateProcess(AxisZ,value);
}
void RotateModelWidget::on_letRotate_x_editingFinished()
{
	double value=ui.letRotate_x->text().toDouble();
	hsdRotateProcess(AxisX,value);
} 
void RotateModelWidget::on_letRotate_y_editingFinished()
{
	double value=ui.letRotate_y->text().toDouble();
	hsdRotateProcess(AxisY,value);
}
void RotateModelWidget::on_letRotate_z_editingFinished()
{
	double value=ui.letRotate_z->text().toDouble();
	hsdRotateProcess(AxisZ,value);
}
void RotateModelWidget::BtnRotateClicked()
{
	QString s=sender()->objectName();
	if(s=="btnRotate_front")
		btnRotateProcess(90,0,0);
	else if(s=="btnRotate_back")
		btnRotateProcess(270,0,0);
	else if(s=="btnRotate_left")
		btnRotateProcess(0,270,0);
	else if(s=="btnRotate_right")
		btnRotateProcess(0,90,0);
	else if(s=="btnRotate_top")
		btnRotateProcess(0,180,0);
	else if(s=="btnRotate_down")
		btnRotateProcess(0,0,0);
}
void RotateModelWidget::Update()
{
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	Point3D value=Point3D(0,0,0);
	if(md)
		value=md->GetRotateValue();
	dataChanged=false;
	ui.hsdRotate_x->setValue(value[0]);
	ui.hsdRotate_y->setValue(value[1]);
	ui.hsdRotate_z->setValue(value[2]);
	dataChanged=true;
	ui.letRotate_x->setText(QString::number(int(value[0]+eps)));
	ui.letRotate_x->setCursorPosition(0);
	ui.letRotate_y->setText(QString::number(int(value[1]+eps)));
	ui.letRotate_y->setCursorPosition(0);
	ui.letRotate_z->setText(QString::number(int(value[2]+eps)));
	ui.letRotate_z->setCursorPosition(0);
}
