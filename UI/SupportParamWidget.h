#ifndef SUPPORTPARAMWIDGET_H
#define SUPPORTPARAMWIDGET_H

#include <QWidget>
#include "ui_SupportParamWidget.h"
#include "Data/ModelData.h"
class SupportGThread;
class SupportParamWidget : public QWidget
{
	Q_OBJECT

signals:
	void UIStatusChanged();

public:
	SupportParamWidget(QWidget *parent = 0);
	~SupportParamWidget();

	void Update();
	void TranslateUI();
private:
	Ui::SupportParamWidget ui;

	SupportGThread* supportGThread;

	void dsbSupportProcess(SupportParam a,double value);

private slots:
	void on_dsbSupport_wt_valueChanged(double value);
	void on_dsbSupport_br_valueChanged(double value);
	void on_dsbSupport_sd_valueChanged(double value);
	void on_dsbSupport_ct_valueChanged(double value);

	void on_dsbSupport_pr_valueChanged(double value);
	void on_dsbSupport_pse_valueChanged(double value);
	void on_dsbSupport_hh_valueChanged(double value);

	void on_btnSupport_generate_clicked();
	void on_btnSupport_delete_clicked();

};

#endif // SUPPORTPARAMWIDGET_H
