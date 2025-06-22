#include "EditTreeWidget.h"
#include "ModelListWidget.h"
#include "MoveModelWidget.h"
#include "RotateModelWidget.h"
#include "ScaleModelWidget.h"
#include "SupportParamWidget.h"
#include<QMouseEvent>
#include<QScrollBar>
EditTreeWidget::EditTreeWidget(QWidget* parent)
:QTreeWidget(parent)
{
	sbvalue=-1;
	dataChanged=true;
	this->setHeaderHidden(true);
	this->setIndentation(0);
	this->setColumnCount(1);
	//this->setColumnWidth(0,225);
	//this->setColumnWidth(1,0);
	this->setExpandsOnDoubleClick(false);
	this->setSelectionMode(QAbstractItemView::NoSelection);
	this->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	this->setFixedWidth(250);
	this->setVerticalScrollMode(ScrollMode::ScrollPerPixel);
	this->setCursor(QCursor(Qt::ArrowCursor));

	//this->setAcceptDrops(true);
	height[0]=105,height[1]=105,height[2]=200,height[3]=135,height[4]=230;

	QScrollBar *sb=this->verticalScrollBar();
	connect(sb,&QScrollBar::valueChanged,this,&EditTreeWidget::OnScrollBarValueChanged);
    
}
EditTreeWidget::~EditTreeWidget()
{

}
void EditTreeWidget::TranslateUI()
{
	spw->TranslateUI();
}

void EditTreeWidget::Bind()
{
    mlw=new ModelListWidget(this);
    mmw=new MoveModelWidget(this);
    rmw=new RotateModelWidget(this);
    smw=new ScaleModelWidget(this);
    spw=new SupportParamWidget(this);
    QWidget* w[5]={mlw,mmw,rmw,smw,spw};
    for(int i=0;i<5;i++)
    {
		QTreeWidgetItem* child=new QTreeWidgetItem(this->topLevelItem(i));
		child->setSizeHint(0,QSize(240,height[i]));
        this->topLevelItem(i)->addChild(child);
        this->topLevelItem(i)->setSizeHint(0,QSize(240,30));
        this->setItemWidget(child,0,w[i]);
    }
	connect(mlw,&ModelListWidget::UIStatusChanged,this,&EditTreeWidget::OnChildUIStatusChanged);
	connect(mmw,&MoveModelWidget::UIStatusChanged,this,&EditTreeWidget::OnChildUIStatusChanged);
	connect(rmw,&RotateModelWidget::UIStatusChanged,this,&EditTreeWidget::OnChildUIStatusChanged);
	connect(smw,&ScaleModelWidget::UIStatusChanged,this,&EditTreeWidget::OnChildUIStatusChanged);
	connect(spw,&SupportParamWidget::UIStatusChanged,this,&EditTreeWidget::OnChildUIStatusChanged);
}

void EditTreeWidget::SetDisabled(bool disabled)
{
    mlw->setDisabled(disabled);
    mmw->setDisabled(disabled);
    rmw->setDisabled(disabled);
    smw->setDisabled(disabled);
    spw->setDisabled(disabled);
}
int EditTreeWidget::GetWidgetHeight()
{
    int n=this->topLevelItemCount();
    int h=0;
    for(int i=0;i<n;i++)
    {
        QTreeWidgetItem* head=this->topLevelItem(i);
        h+=head->sizeHint(0).height();
        if(head->isExpanded())
            h+=height[i];
    }
    return h;
}

int EditTreeWidget::GetNumofExpand()
{
    int n=this->topLevelItemCount(),nexpand=0;
    for(int i=0;i<n;i++)
        if(this->topLevelItem(i)->isExpanded())
            nexpand++;
    return nexpand;
}

void EditTreeWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()&Qt::RightButton)
    {
		
    }
    else if(event->button()&Qt::LeftButton)
    {
        QTreeWidgetItem* cur=itemFromIndex(indexAt(event->pos()));
        if(cur&&cur->childCount()>0)
        {
            if(cur->isExpanded())
                cur->setExpanded(false);
            else
                cur->setExpanded(true);
			emit ItemExpandStateChanged();
        }
    }
}
void EditTreeWidget::OnChildUIStatusChanged()
{
	emit UIStatusChanged();
}
void EditTreeWidget::OnScrollBarValueChanged(int value)
{
    if(dataChanged==false)
        return;
    dataChanged=false;
    QScrollBar *sb=this->verticalScrollBar();
    if(sbvalue==-1)
    {
        sbvalue=0;
    }
    else
    {
        if(value-sbvalue>200||value==sb->maximum())
            value=std::min(sbvalue+20,sb->maximum());
        if(value-sbvalue<-200||value==sb->minimum())
            value=std::max(sbvalue-20,sb->minimum());
        sbvalue=value;
    }
    sb->setValue(sbvalue);
    dataChanged=true;
}
void EditTreeWidget::Update()
{
    mlw->Update();
    mmw->Update();
    rmw->Update();
    smw->Update();
    spw->Update();
}

