#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QDragEnterEvent>
#include "ui_MainWindow.h"
#include "Uart.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Update();
    void TranslateUI();
    void ResizeControl();
	void SetProgressBarValue(float value);
	void UpdateEnable();

	void Open(const QString &path);
    Uart com;
private:
    Ui::MainWindow ui;
	const static int margin;
	/*Window Geometry*/
	QRect preGeometry;
	QPoint leftButtonDownPos;
	QTranslator *dedibot_zh;
	int mousePosFlag;

	bool winMax;
	bool leftButtonDown;

	int GetMousePosFlag(QPoint p);
	void SetCursorType(int flag);
	void UpdateCursorType(QPoint p);

	/*User Setting*/
	void SaveUserSetting();

protected: 
	void paintEvent(QPaintEvent*);
	bool eventFilter(QObject *obj,QEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);

protected:

    bool IsUrlsLegal(const QList<QUrl>&urls);
	virtual void dragEnterEvent(QDragEnterEvent * event);
	virtual void dragMoveEvent(QDragMoveEvent * event);
	virtual void dropEvent(QDropEvent * event);

private slots:
	void closeEvent(QCloseEvent *event);

	void on_btnMain_close_clicked();
	void on_btnMain_max_clicked();
	void on_btnMain_min_clicked();

    void on_btnMain_import_clicked();
	void on_btnMain_open_clicked();
	void on_btnMain_save_clicked();
    void on_btnMain_ctrl_clicked();
    void on_vsdMain_layer_valueChanged(int value);
    void on_cbbMain_language_currentIndexChanged(int index);
	void on_cbbMain_view_activated(int index);

	void on_btnMain_print_clicked();
    void on_comBtn_clicked();
};

#endif // MAINWINDOW_H
