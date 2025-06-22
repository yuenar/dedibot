#ifndef EDITTREEWIDGET_H
#define EDITTREEWIDGET_H

#include <QWidget>
#include <QTreeWidget>

class ModelListWidget;
class MoveModelWidget;
class RotateModelWidget;
class ScaleModelWidget;
class SupportParamWidget;

class EditTreeWidget:public QTreeWidget
{
	Q_OBJECT

signals:
	void UIStatusChanged();

public:
	EditTreeWidget(QWidget *parent=0);
	~EditTreeWidget();

	void Bind();
	void Update();
	void TranslateUI();
	void SetDisabled(bool disabled);
    int GetWidgetHeight();
    int GetNumofExpand();

signals:
	void ItemExpandStateChanged();

protected:
	void mouseReleaseEvent(QMouseEvent *event);

private:
    int sbvalue;
	bool dataChanged;
    int height[5],expand[5];

	ModelListWidget* mlw;
	MoveModelWidget* mmw;
	RotateModelWidget* rmw;
	ScaleModelWidget* smw;
	SupportParamWidget* spw;

	void OnScrollBarValueChanged(int value);
	void OnChildUIStatusChanged();
};

#endif // EDITTREEWIDGET_H
