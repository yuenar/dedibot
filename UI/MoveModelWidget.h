#ifndef MOVEMODELWIDGET_H
#define MOVEMODELWIDGET_H

#include <QWidget>
#include "ui_MoveModelWidget.h"
#include "Data/ModelData.h"

class MoveModelWidget : public QWidget
{
	Q_OBJECT

signals:
	void UIStatusChanged();

public:
	MoveModelWidget(QWidget *parent = 0);
	~MoveModelWidget();

	void Update();
private:
	Ui::MoveModelWidget ui;
	bool dataChanged;
	void hsdMoveProcess(Axis axis,const double value);

private slots:;
	void on_hsdMove_x_valueChanged(int value);
	void on_hsdMove_y_valueChanged(int value);
	//void on_hsdMove_z_valueChanged(int value);

	void on_letMove_x_editingFinished();
	void on_letMove_y_editingFinished();
	//void on_letMove_z_editingFinished();
};

#endif // MOVEMODELWIDGET_H
