#ifndef SLICEMODELDIALOG_H
#define SLICEMODELDIALOG_H

#include <QDialog>
#include "ui_SliceModelDialog.h"

class SliceModelDialog : public QDialog
{
	Q_OBJECT

public:
	SliceModelDialog(QWidget *parent = 0);
	~SliceModelDialog();

	void SetConfigPath(const QString &path)
	{
		configPath=path;
	}
private:
	Ui::SliceModelDialog ui;
	QString configPath;

private slots:
	//void on_spbSlice_thick_valueChanged(int value);
	void on_btnSlice_ok_clicked();
};

#endif // SLICEMODELDIALOG_H
