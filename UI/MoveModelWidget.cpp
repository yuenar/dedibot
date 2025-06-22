#include "MoveModelWidget.h"
#include "Data/ProjectData.h"
#include "Data/ModelData.h"
MoveModelWidget::MoveModelWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	dataChanged=true;
	setDisabled(true);
	const double* boxSize=ProjectData::GetInstance()->GetBoxSize();
	ui.hsdMove_x->setMaximum(boxSize[0]/2+eps);
	ui.hsdMove_x->setMinimum(-boxSize[0]/2-eps);
	ui.hsdMove_y->setMaximum(boxSize[1]/2+eps);
	ui.hsdMove_y->setMinimum(-boxSize[1]/2-eps);
}

MoveModelWidget::~MoveModelWidget()
{

}
void MoveModelWidget::hsdMoveProcess(Axis axis,const double value)
{
	if(dataChanged==false)
		return;
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	if(md)
	{
		md->SetMoveValue(axis,value);
		emit UIStatusChanged();
	}
}
void MoveModelWidget::on_hsdMove_x_valueChanged(int value)
{
	hsdMoveProcess(AxisX,value);
}
void MoveModelWidget::on_hsdMove_y_valueChanged(int value)
{
	hsdMoveProcess(AxisY,value);
}
//void MoveModelWidget::on_hsdMove_z_valueChanged(int value)
//{
//	hsdMoveProcess(AxisZ,value);
//}
void MoveModelWidget::on_letMove_x_editingFinished()
{
	double value=ui.letMove_x->text().toDouble();
	value=std::min(value,1.0*ui.hsdMove_x->maximum());
	value=std::max(value,1.0*ui.hsdMove_x->minimum());
	hsdMoveProcess(AxisX,value);
}
void MoveModelWidget::on_letMove_y_editingFinished()
{
	double value=ui.letMove_y->text().toDouble();
	value=std::min(value,1.0*ui.hsdMove_y->maximum());
	value=std::max(value,1.0*ui.hsdMove_y->minimum());
	hsdMoveProcess(AxisY,value);
}
//void MoveModelWidget::on_letMove_z_editingFinished()
//{
//	double value=ui.letMove_z->text().toDouble();
//	value=std::min(value,1.0*ui.hsdMove_z->maximum());
//	value=std::max(value,1.0*ui.hsdMove_z->minimum());
//	hsdMoveProcess(AxisZ,value);
//}
void MoveModelWidget::Update()
{
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	Point3D value=Point3D(0,0,0);
	if(md)
		value=md->GetMoveValue();
	dataChanged=false;
	ui.hsdMove_x->setValue(value[0]);
	ui.hsdMove_y->setValue(value[1]);
	//ui.hsdMove_z->setValue(value[2]);
	dataChanged=true;
	ui.letMove_x->setText(QString::number(value[0],'f',2));
	ui.letMove_x->setCursorPosition(0);
	ui.letMove_y->setText(QString::number(value[1],'f',2));
	ui.letMove_y->setCursorPosition(0);
	ui.letMove_z->setText(QString::number(value[2],'f',2));
	//ui.letMove_z->setCursorPosition(0);
}
