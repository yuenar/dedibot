#include "ScaleModelWidget.h"
#include "IconHelper.h"
#include "DediMessageBox.h"
#include "Data/ModelData.h"
#include "Data/ProjectData.h"
ScaleModelWidget::ScaleModelWidget(QWidget *parent)
	: QWidget(parent)
{
	dataChanged=true;
	changedPos=-1;
	ui.setupUi(this);
	setDisabled(true);
	IconHelper::Instance()->SetIcon(ui.btnScale_undo,QChar(0xf0e2),10);
}

ScaleModelWidget::~ScaleModelWidget()
{

}
void ScaleModelWidget::ScaleValueChanged(double value,bool reset,bool g)
{
	if(dataChanged==false)
		return;
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	if(md)
	{
		if(md->GetSupportId()>=0)
		{
			DediMessageBox rsw(RemoveSupportWarning,this->topLevelWidget());
			if(rsw.exec()==QMessageBox::No)
			{
				emit UIStatusChanged();
				return;
			}	
			md->RemoveSupport();
		}
		g?md->SetScaleValue(value/md->GetDefaultScale()):md->SetScaleValue(value);
		if(reset==false)
			changedPos=3;
		else
			changedPos=-1;
		emit UIStatusChanged();
		changedPos=-1;
	}
}
void ScaleModelWidget::dsbScaleProcess(Axis axis,const double value)
{
	if(dataChanged==false)
		return;
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	if(md)
	{
		double scale=md->GetScaleValue();
		double gs=md->GetGeometrySize(axis);
		if(md->GetSupportId()>=0)
		{
			DediMessageBox rsw(RemoveSupportWarning,this->topLevelWidget());
			if(rsw.exec()==QMessageBox::No)
			{
				emit UIStatusChanged();
				return;
			}	
			md->RemoveSupport();
		}
		md->SetScaleValue(value*scale/gs);
		changedPos=axis;
		emit UIStatusChanged();
		changedPos=-1;
	}
}
void ScaleModelWidget::on_dsbScale_x_valueChanged(double value)
{
	dsbScaleProcess(AxisX,value);
}
void ScaleModelWidget::on_dsbScale_y_valueChanged(double value)
{
	dsbScaleProcess(AxisY,value);
}
void ScaleModelWidget::on_dsbScale_z_valueChanged(double value)
{
	dsbScaleProcess(AxisZ,value);
}
void ScaleModelWidget::on_dsbScale_s_valueChanged(double value)
{
	ScaleValueChanged(value);
}

void ScaleModelWidget::on_btnScale_undo_clicked()
{
	ScaleValueChanged(1.0,true);
}

void ScaleModelWidget::Update()
{
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	double scale=1.0,tmp[3]={50,50,50},defaultScale=1.0;
	const double* gs=tmp;
	if(md)
	{
		scale=md->GetScaleValue();
		defaultScale=md->GetDefaultScale();
		gs=md->GetGeometrySize();
	}
	dataChanged=false;
	if(changedPos!=0)
	ui.dsbScale_x->setValue(gs[0]);
	if(changedPos!=1)
	ui.dsbScale_y->setValue(gs[1]);
	if(changedPos!=2)
	ui.dsbScale_z->setValue(gs[2]);
	if(changedPos!=3)
	ui.dsbScale_s->setValue(scale*defaultScale);
	dataChanged=true;
}

