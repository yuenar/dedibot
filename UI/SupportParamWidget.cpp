#include "SupportParamWidget.h"
#include "IconHelper.h"
#include "Data/ModelData.h"
#include "Data/ProjectData.h"
#include "Thread/ThreadPool.h"
SupportParamWidget::SupportParamWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setDisabled(true);
	TranslateUI();
	IconHelper::Instance()->SetIcon(ui.btnSupport_delete,QChar(0xf014),15);
}

SupportParamWidget::~SupportParamWidget()
{

}

void SupportParamWidget::dsbSupportProcess(SupportParam a,double value)
{
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	if(md)
	{
		md->SetSupportParam(a,value);
	}
}
void SupportParamWidget::on_dsbSupport_wt_valueChanged(double value)
{
	dsbSupportProcess(SupportParam::SupportThickness,value);
}
void SupportParamWidget::on_dsbSupport_br_valueChanged(double value)
{
	dsbSupportProcess(SupportParam::ContactPointSize,value);
}
void SupportParamWidget::on_dsbSupport_sd_valueChanged(double value)
{
	dsbSupportProcess(SupportParam::PillarSpacing,value);
}
void SupportParamWidget::on_dsbSupport_ct_valueChanged(double value)
{
	dsbSupportProcess(SupportParam::ChassisThickness,value);
}
void SupportParamWidget::on_dsbSupport_pr_valueChanged(double value)
{
	dsbSupportProcess(SupportParam::PillarRadius,value);
}
void SupportParamWidget::on_dsbSupport_pse_valueChanged(double value)
{
	dsbSupportProcess(SupportParam::PillarSpacingE,value);
}
void SupportParamWidget::on_dsbSupport_hh_valueChanged(double value)
{
	dsbSupportProcess(SupportParam::HoistingHeight,value);
}
void SupportParamWidget::TranslateUI()
{
	ui.dsbSupport_br->setToolTip(tr("Span: ")+QString::number(ui.dsbSupport_br->minimum())+"~"+QString::number(ui.dsbSupport_br->maximum()));
	ui.dsbSupport_ct->setToolTip(tr("Span: ")+QString::number(ui.dsbSupport_ct->minimum())+"~"+QString::number(ui.dsbSupport_ct->maximum()));
	ui.dsbSupport_sd->setToolTip(tr("Span: ")+QString::number(ui.dsbSupport_sd->minimum())+"~"+QString::number(ui.dsbSupport_sd->maximum()));
	ui.dsbSupport_wt->setToolTip(tr("Span: ")+QString::number(ui.dsbSupport_wt->minimum())+"~"+QString::number(ui.dsbSupport_wt->maximum()));
	ui.dsbSupport_pr->setToolTip(tr("Span: ")+QString::number(ui.dsbSupport_pr->minimum())+"~"+QString::number(ui.dsbSupport_pr->maximum()));
	ui.dsbSupport_pse->setToolTip(tr("Span: ")+QString::number(ui.dsbSupport_pse->minimum())+"~"+QString::number(ui.dsbSupport_pse->maximum()));
	ui.retranslateUi(this);
}
void SupportParamWidget::on_btnSupport_generate_clicked()
{
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	if(md)
		ThreadPool::GetInstance()->GetSupportGThread()->GenerateSupporterClassic(md);
}
void SupportParamWidget::on_btnSupport_delete_clicked()
{
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	if(md)
		md->RemoveSupport();
	emit UIStatusChanged();
}
void SupportParamWidget::Update()
{
	ModelData* md=ProjectData::GetInstance()->GetSelectModel();
	double param[]={0,0,0,0,0,0,0};
	const double* sp=param;
	if(md)
		sp=md->GetSupportParam();
	ui.dsbSupport_wt->setValue(sp[SupportParam::SupportThickness]);
	ui.dsbSupport_br->setValue(sp[SupportParam::ContactPointSize]);
	ui.dsbSupport_sd->setValue(sp[SupportParam::PillarSpacing]);
	ui.dsbSupport_ct->setValue(sp[SupportParam::ChassisThickness]);
	ui.dsbSupport_pse->setValue(sp[SupportParam::PillarSpacingE]);
	ui.dsbSupport_pr->setValue(sp[SupportParam::PillarRadius]);
}
