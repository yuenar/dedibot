#ifndef DEDIMESSAGEBOX_H
#define DEDIMESSAGEBOX_H

#include <QMessageBox>
enum MessageBoxType
{
	OpenErrorCritical,MemoryLimitedCritical,ThickNot5sCritical,ExposurePeriodCritical,SliceErrorCritical,PrintableErrorCritical,SlicerNotFoundCritical,CriticalEnd,
	OpenFileWarning,RemoveSupportWarning,NoSupportWarning,SliceRunnningWarning,SaveFileWarning,WarningEnd,
	SliceFinishedInformation,SaveSuccessInformation,InformationEnd
};
class DediMessageBox : public QMessageBox
{
	Q_OBJECT

public:
	DediMessageBox(MessageBoxType mbt,QWidget *parent,const QString &info="");
	~DediMessageBox();

private:
	void InitCriticalBox();
	void InitWarningBox();
	void InitInformationBox();
};

#endif // DEDIMESSAGEBOX_H
