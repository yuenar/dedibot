#ifndef SCALEMODELWIDGET_H
#define SCALEMODELWIDGET_H

#include <QWidget>
#include "ui_ScaleModelWidget.h"
#include "Data/ModelData.h"

class ScaleModelWidget : public QWidget
{
	Q_OBJECT

signals:
	void UIStatusChanged();

public:
	ScaleModelWidget(QWidget *parent = 0);
	~ScaleModelWidget();

	void Update();
private:
	Ui::ScaleModelWidget ui;
	bool dataChanged;
	int changedPos;
	void dsbScaleProcess(Axis axis,const double value);
	void ScaleValueChanged(double value,bool reset=false,bool g=true);

private slots:
	void on_dsbScale_x_valueChanged(double value);
	void on_dsbScale_y_valueChanged(double value);
	void on_dsbScale_z_valueChanged(double value);
	void on_dsbScale_s_valueChanged(double value);

	void on_btnScale_undo_clicked();
};

#endif // SCALEMODELWIDGET_H
