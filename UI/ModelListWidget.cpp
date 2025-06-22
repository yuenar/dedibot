#include "ModelListWidget.h"
#include "IconHelper.h"
#include <QPushButton>
#include <QLineEdit>
#include <QAbstractTableModel>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QFileInfo>
#include <QMimeData>

#include "Data/ProjectData.h"
#include "Data/ModelData.h"
ModelListWidget::ModelListWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	dataChanged=true;
	columnCount=5;
	ui.tbwModel_list->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tbwModel_list->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	ui.tbwModel_list->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	ui.tbwModel_list->setEditTriggers(QAbstractItemView::EditTrigger::DoubleClicked);
	ui.tbwModel_list->setColumnCount(columnCount);
	ui.tbwModel_list->setShowGrid(true);
	ui.tbwModel_list->setColumnWidth(0,25);
	ui.tbwModel_list->setColumnWidth(1,25);
	ui.tbwModel_list->setColumnWidth(2,100);
	ui.tbwModel_list->setColumnWidth(3,25);
	ui.tbwModel_list->setColumnWidth(4,25);
	QStringList header;
    //header<<tr("M")<<tr("S")<<tr("V")<<tr("F")<<tr("C")<<tr("D");
	ui.tbwModel_list->setHorizontalHeaderLabels(header);
	//ui.tbwModel_list->verticalHeader()->setVisible(false);
	ui.tbwModel_list->horizontalHeader()->setVisible(false);
	//ui.tbwModel_list->setHorizontalHeaderLabels(header);

	this->setAcceptDrops(true);

	connect(ui.tbwModel_list,&QTableWidget::itemSelectionChanged,this,&ModelListWidget::OnItemSelectionChanged);
	connect(ui.tbwModel_list,&QTableWidget::itemChanged,this,&ModelListWidget::OnItemChanged);
}

ModelListWidget::~ModelListWidget()
{

}

void ModelListWidget::InsertRow(int row)
{
	ui.tbwModel_list->insertRow(row);
	ui.tbwModel_list->setRowHeight(row,20);
	QPushButton* button0=new QPushButton(ui.tbwModel_list);
	IconHelper::Instance()->SetIcon(button0,QChar(0xf06e),10);
	button0->setStyleSheet("QPushButton:enabled{background:white;color:black}QPushButton:disabled{background:grey;color:black}");
	ui.tbwModel_list->setCellWidget(row,0,button0);
	QPushButton* button1=new QPushButton(ui.tbwModel_list);
	IconHelper::Instance()->SetIcon(button1,QChar(0xf06e),10);
	button1->setStyleSheet("QPushButton:enabled{background:white;color:black}QPushButton:disabled{background:grey;color:black}");
	ui.tbwModel_list->setCellWidget(row,1,button1);
	QTableWidgetItem* modelName=new QTableWidgetItem;
	modelName->setTextAlignment(Qt::AlignCenter);
	ui.tbwModel_list->setItem(row,2,modelName);
	QPushButton* button3=new QPushButton(ui.tbwModel_list);
	IconHelper::Instance()->SetIcon(button3,QChar(0xf0c5),10);
	button3->setStyleSheet("QPushButton:enabled{background:#008ccc}QPushButton:disabled{background:grey}");
	ui.tbwModel_list->setCellWidget(row,3,button3);
	QPushButton* button4=new QPushButton(ui.tbwModel_list);
	IconHelper::Instance()->SetIcon(button4,QChar(0xf014),10);
	button4->setStyleSheet("QPushButton:enabled{background:red}QPushButton:disabled{background:grey}");
	ui.tbwModel_list->setCellWidget(row,4,button4);
	connect(button0,&QPushButton::clicked,this,&ModelListWidget::OnButtonClicked);
	connect(button1,&QPushButton::clicked,this,&ModelListWidget::OnButtonClicked);
	connect(button3,&QPushButton::clicked,this,&ModelListWidget::OnButtonClicked);
	connect(button4,&QPushButton::clicked,this,&ModelListWidget::OnButtonClicked);
}

void ModelListWidget::Update()
{
	dataChanged=false;
	int size=ProjectData::GetInstance()->GetSize();
	int rowCount=ui.tbwModel_list->rowCount();
	//更新显示列表行数
	for(int i=rowCount;i<size;i++)
		InsertRow(i);
	for(int i=rowCount-1;i>=size;i--)
		ui.tbwModel_list->removeRow(i);
	//更新列表每行数据
	for(int i=0;i<size;i++)
	{
		ModelData* md=ProjectData::GetInstance()->GetModelData(i);
		for(int j=0;j<columnCount;j++)
		{
			QWidget* w=ui.tbwModel_list->cellWidget(i,j);
			if(w)
				w->setObjectName(QString::number(i*columnCount+j));//设置标记
		}
		QPushButton* button0=(QPushButton*)ui.tbwModel_list->cellWidget(i,0);
		QPushButton* button1=(QPushButton*)ui.tbwModel_list->cellWidget(i,1);
		md->IsModelVisible()?(button0->setText(QChar(0xf06e)),button0->setToolTip(tr("Hide Model"))):(button0->setText(QChar(0xf070)),button0->setToolTip(tr("Show Model")));
		md->IsSupportVisible()?(button1->setText(QChar(0xf06e)),button1->setToolTip(tr("Hide Support"))):(button1->setText(QChar(0xf070)),button1->setToolTip(tr("Show Support")));
		md->GetSupportMesh()==NULL?button1->setDisabled(true):button1->setDisabled(false);
		/*QLineEdit* modelName=(QLineEdit*)ui.tbwModel_list->cellWidget(i,2);
		modelName->setText("Model 0");*/
		/*QLineEdit* defalutScale=(QLineEdit*)ui.tbwModel_list->cellWidget(i,3);*/
		ui.tbwModel_list->item(i,2)->setText(QString::fromLocal8Bit(md->GetModelName().c_str()));
		//ui.tbwModel_list->item(i,3)->setText(QString::number(md->GetDefaultScale()));

		//ui.tbwModel_list->item(i,2)->setText(QString::number(md->GetModelId()));
		//ui.tbwModel_list->item(i,3)->setText(QString::number(md->GetModelId())+","+QString::number(md->GetSupportId()));
	}
	ui.tbwModel_list->selectRow(ProjectData::GetInstance()->GetSelectModelIndex());
	dataChanged=true;
}

void ModelListWidget::OnButtonClicked()
{
	QString flag=sender()->objectName();
	int id=flag.toInt();
	int row=id/columnCount,column=id%columnCount;
	ModelData* md=ProjectData::GetInstance()->GetModelData(row);
	if(column==0)
	{
		bool visible=md->IsModelVisible();
		md->SetModelVisible(!visible);
	}
	if(column==1)
	{
		bool visible=md->IsSupportVisible();
		md->SetSupportVisible(!visible);
	}
	if(column==3)
	{
		ModelData *nmd=new ModelData(*md);
		nmd->SetMoveValue(Axis::AxisX,nmd->GetMoveValue(Axis::AxisX)+(rand()%1000)/100.0);
		nmd->SetMoveValue(Axis::AxisY,nmd->GetMoveValue(Axis::AxisY)+(rand()%1000)/100.0);
		ProjectData::GetInstance()->InsertModelData(nmd);
	}
	else if(column==4)
	{
		ProjectData::GetInstance()->RemoveModelData(row);
	}
	emit UIStatusChanged();
}

void ModelListWidget::OnItemSelectionChanged()
{
	if(dataChanged==false)
		return;
	int idx=ui.tbwModel_list->selectedItems()[0]->row();
	ProjectData::GetInstance()->SetSelectModelIndex(idx);
	emit UIStatusChanged();
}
void ModelListWidget::OnItemChanged(QTableWidgetItem * item)
{
	if(dataChanged==false)
		return;
	int row=item->row(),column=item->column();
	ModelData* md=ProjectData::GetInstance()->GetModelData(row);
	if(column==2)
	{
		md->SetModelName(item->text().toLocal8Bit().data());
	}
	else if(column==3)
	{
		//double value=item->text().toDouble();
		//md->SetDefaultScale(value);
	}
	emit UIStatusChanged();
}
