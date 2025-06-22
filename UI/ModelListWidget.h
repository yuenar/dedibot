#ifndef MODELLISTWIDGET_H
#define MODELLISTWIDGET_H

#include <QWidget>
#include <QDragEnterEvent>
#include "ui_ModelListWidget.h"
class ModelData;
class ModelListWidget : public QWidget
{
	Q_OBJECT

signals:
	void UIStatusChanged();

public:
	ModelListWidget(QWidget *parent = 0);
	~ModelListWidget();

	void Update();
private:
	Ui::ModelListWidget ui;
	int columnCount;
	bool dataChanged;
	void InsertRow(int row);
//protected:
//	virtual void dragEnterEvent(QDragEnterEvent * event);
//	virtual void dragMoveEvent(QDragMoveEvent * event);
//	virtual void dropEvent(QDropEvent * event);

private slots:
	void OnButtonClicked();
	void OnItemSelectionChanged();
	void OnItemChanged(QTableWidgetItem * item);
};

#endif // MODELLISTWIDGET_H
