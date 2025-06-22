#include "SliceModelDialog.h"
#include "Data/ProjectData.h"
#include<QSettings>
#include"DediMessageBox.h"
SliceModelDialog::SliceModelDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	Qt::WindowFlags flags=Qt::Dialog;
	flags|=Qt::WindowCloseButtonHint;
	configPath="config.ini";
	this->setWindowFlags(flags);
	this->setWindowModality(Qt::ApplicationModal);
	ui.spbSlice_thick->setToolTip(tr("Span: ")+QString::number(ui.spbSlice_thick->minimum())+"~"+QString::number(ui.spbSlice_thick->maximum()));
	ui.spbSlice_current->setToolTip(tr("Span: ")+QString::number(ui.spbSlice_current->minimum())+"~"+QString::number(ui.spbSlice_current->maximum()));
	ui.spbSlice_ET->setToolTip(tr("Span: ")+QString::number(ui.spbSlice_ET->minimum())+"~"+QString::number(ui.spbSlice_ET->maximum()));
	ui.spbSlice_EP->setToolTip(tr("Span: ")+QString::number(ui.spbSlice_EP->minimum())+"~"+QString::number(ui.spbSlice_EP->maximum()));
}

SliceModelDialog::~SliceModelDialog()
{

}
//void SliceModelDialog::on_spbSlice_thick_valueChanged(int value)
//{
//
//}
void SliceModelDialog::on_btnSlice_ok_clicked()
{
	if(ui.spbSlice_thick->value()%5!=0)
	{
		DediMessageBox tnc(ThickNot5sCritical,this);
		tnc.exec();
		return;
	}
	if(ui.spbSlice_ET->value()>=ui.spbSlice_EP->value())
	{
		DediMessageBox epc(ExposurePeriodCritical,this);
		epc.exec();
		return;
	}
	QString color;
	int index=ui.cbbSlice_color->currentIndex();
	index==0?color="black":  color="white";
	QSettings config(configPath,QSettings::IniFormat);
	config.setValue("Slice/ImageColor",color);
	config.setValue("Slice/ImageResolution",ui.cbbSlice_res->currentText());
	config.setValue("Slice/SliceThickness",ui.spbSlice_thick->text());
	config.setValue("Slice/LEDCurrent",ui.spbSlice_current->text());
	config.setValue("Slice/ExposureTime",ui.spbSlice_ET->text());
	config.setValue("Slice/ExposurePeriod",ui.spbSlice_EP->text());
	config.setValue("Slice/ShowPrint",0);
	this->accept();
}
