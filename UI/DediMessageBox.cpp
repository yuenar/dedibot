#include "DediMessageBox.h"

DediMessageBox::DediMessageBox(MessageBoxType mbt,QWidget *parent,const QString &info)
	: QMessageBox(parent)
{

	if(mbt<CriticalEnd)
		InitCriticalBox();
	else if(mbt<WarningEnd)
		InitWarningBox();
	else
		InitInformationBox();

	switch(mbt)
	{
		case OpenErrorCritical:
			this->setText(tr("Open file ")+info+tr(" failed!"));
			break;
		case MemoryLimitedCritical:
			this->setText(tr("The hard disk space is not enough. Please clean it!"));
			break;
		case ThickNot5sCritical:
			this->setText(tr("Slice Thickness must be a multiple of 5!"));
			break;
		case ExposurePeriodCritical:
			this->setText(tr("Exposure Period must be bigger than Exposure Time!"));
			break;
		case SliceErrorCritical:
			this->setText(tr("Slice Error!"));
			break;
		case PrintableErrorCritical:
			this->setText(tr("There are one or more models not inside the box!"));
			break;
		case SlicerNotFoundCritical:
			this->setText(tr("Slice Failed! Slicer is not found!"));
			break;

		case OpenFileWarning:
			this->setText(tr("Close current project and open file ")+info+tr(" ?"));
			break;
		case RemoveSupportWarning:
			this->setText(tr("This operation will remove the currrent support structure! Do you want to continue?"));
			break;
		case NoSupportWarning:
			this->setText(tr("There are one or more models without support structure! Do you want to continue?"));
			break;
		case SliceRunnningWarning:
			this->setText(tr("Slicing is not finished! Do you want to exit?"));
			break;
		case SaveFileWarning:
			this->addButton(QMessageBox::Cancel);
			this->setButtonText(QMessageBox::Cancel,tr("Cancel"));
			this->setText(tr("Do you want to save current project?"));
			break;

		case SliceFinishedInformation:
			this->addButton(QMessageBox::Cancel);
			this->setButtonText(QMessageBox::Cancel,tr("Cancel"));
			this->setText(tr("Slicing finished! Start to print?"));
			break;
		case SaveSuccessInformation:
			this->setText(tr("File ")+info+tr(" has been saved!"));
			break;
		default:
			break;
	}
	this->setStyleSheet("color:black;font:Times New Roman");
}

DediMessageBox::~DediMessageBox()
{

}


void DediMessageBox::InitCriticalBox()
{
	this->setIcon(QMessageBox::Critical);
	this->setWindowTitle(tr("Error"));
	this->setStandardButtons(QMessageBox::Ok);
	this->setButtonText(QMessageBox::Ok,tr("Ok"));
}
void DediMessageBox::InitWarningBox()
{
	this->setIcon(QMessageBox::Warning);
	this->setWindowTitle(tr("Warning"));
	this->setStandardButtons(QMessageBox::Yes|QMessageBox::No);
	this->setButtonText(QMessageBox::Yes,tr("Yes"));
	this->setButtonText(QMessageBox::No,tr("No"));
}
void DediMessageBox::InitInformationBox()
{
	this->setIcon(QMessageBox::Information);
	this->setWindowTitle(tr("Information"));
	this->setStandardButtons(QMessageBox::Ok);
	this->setButtonText(QMessageBox::Ok,tr("Ok"));
}