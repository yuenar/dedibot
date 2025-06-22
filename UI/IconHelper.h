#ifndef ICONHELPER_H
#define ICONHELPER_H

#include <QObject>
#include <QFont>
#include <QFontDatabase>
#include <QMutex>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QTableWidgetItem>
class IconHelper : public QObject
{
private:
	QFont iconFont;
    static IconHelper* _instance;
	explicit IconHelper(QObject *parent=0);

public:
    static IconHelper* Instance()
    {
        static QMutex mutex;
        if (!_instance) {
            QMutexLocker locker(&mutex);
            if (!_instance) {
                _instance = new IconHelper;
            }
        }
        return _instance;
    }
	QFont GetIconFont(int size=10);
	template<typename T>
	void SetIcon(T* obj,QChar c,int size=10)
	{
		iconFont.setPointSize(size);
		obj->setFont(iconFont);
		obj->setText(c);
	}
	template<typename T>
	void SetIcon(T* obj,int column,QChar c,int size=10)
	{
		iconFont.setPointSize(size);
		obj->setFont(column,iconFont);
		obj->setText(column,c);
	}
};
#endif // ICONHELPER_H