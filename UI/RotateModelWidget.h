#ifndef ROTATEMODELWIDGET_H
#define ROTATEMODELWIDGET_H

#include <QWidget>
#include "ui_RotateModelWidget.h"
#include "Data/ModelData.h"

class RotateModelWidget : public QWidget
{
	Q_OBJECT

signals:
	void UIStatusChanged();

public:
	RotateModelWidget(QWidget *parent = 0);
	~RotateModelWidget();

	void Update();
private:
	Ui::RotateModelWidget ui;
	bool dataChanged;
	void hsdRotateProcess(Axis axis,const double value);
	void btnRotateProcess(double rx,double ry,double rz);
private slots:
	void on_hsdRotate_x_valueChanged(int value);
	void on_hsdRotate_y_valueChanged(int value);
	void on_hsdRotate_z_valueChanged(int value);

	void on_letRotate_x_editingFinished();
	void on_letRotate_y_editingFinished();
	void on_letRotate_z_editingFinished();

    void BtnRotateClicked();
};

#endif // ROTATEMODELWIDGET_H
