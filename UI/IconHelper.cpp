#include"IconHelper.h"
IconHelper* IconHelper::_instance=0;
IconHelper::IconHelper(QObject *parent):QObject(qApp)
{
	int fontId=QFontDatabase::addApplicationFont(":/Resources/fontawesome-webfont.ttf");
	QString fontName=QFontDatabase::applicationFontFamilies(fontId).at(0);
	iconFont=QFont(fontName);
}
QFont IconHelper::GetIconFont(int size)
{
	iconFont.setPointSize(size);
	return iconFont;
}