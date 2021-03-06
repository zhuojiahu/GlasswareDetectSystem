#include "Widget_CarveSetting.h"
#include "glasswaredetectsystem.h"
#include <QScrollArea>
extern GlasswareDetectSystem *pMainFrm;

myPushButton::myPushButton(QWidget *parent)
	: QPushButton(parent)
{

}
myPushButton::~myPushButton()
{

}

WidgetCarveSetting::WidgetCarveSetting(QWidget *parent)
	: QWidget(parent)
{
	pStackedCarve = new QStackedWidget(this);
	image_widget = new ImageWidget(this);
	errorList_widget = new CErrorImageList(this);
	//pStackedCarve->setCurrentIndex(0);
	for (int i = 0;i<pMainFrm->m_sSystemInfo.iCamCount;i++)
	{
		widgetCarveImage = new Widget_CarveImage;
		widgetCarveImage->init(i);
		listWidgetCarveImage.append(widgetCarveImage);
		pStackedCarve->addWidget(widgetCarveImage);
		connect(widgetCarveImage,SIGNAL(signals_showPic(int)),image_widget,SLOT(slots_imageItemDoubleClick(int )));
		connect(widgetCarveImage,SIGNAL(signals_showPic(int)),image_widget,SLOT(slots_showOnlyCamera(int )));
		connect(pMainFrm->pdetthread[i],SIGNAL(signals_updateActiveImg(int,int,double,int)),widgetCarveImage,SLOT(slots_updateActiveImg(int,int,double,int)));
	}
	buttonGroupCamera  = new QButtonGroup(this);
	QGridLayout *GridLayout = new QGridLayout;
	for (int i = 0;i<pMainFrm->m_sSystemInfo.iCamCount;i++)
	{
		myPushButton *pushButton_carvePage = new myPushButton(this);
		pushButton_carvePage->setObjectName("toolButtonCamera");
		pushButton_carvePage->setText(QString::number(i+1));
		pushButton_carvePage->setStyleSheet("background-color:gray");
		buttonGroupCamera->addButton(pushButton_carvePage,i);
		if(pMainFrm->m_sSystemInfo.iCamCount ==36)
		{
			GridLayout->addWidget(pushButton_carvePage,i%3,i/3);
		}else
		{
			GridLayout->addWidget(pushButton_carvePage,i%2,i/2);
		}
	}
	setWidget = new QWidget(this);
	setWidget->setMaximumWidth(400);
	QVBoxLayout* vLayout = new QVBoxLayout(setWidget);
	vLayout->addWidget(pStackedCarve);
	vLayout->addLayout(GridLayout);
	vLayout->setSpacing(0);
	vLayout->setContentsMargins(0, 0, 0, 0);
	
	listWidget = new QWidget(this);
	QScrollArea *scrollArea = new QScrollArea(listWidget);
	QVBoxLayout* vLayout1 = new QVBoxLayout(listWidget);
	vLayout1->addWidget(scrollArea);
	vLayout1->setSpacing(0);
	vLayout1->setContentsMargins(0,0,0,0  );
	QVBoxLayout* vLayout2 = new QVBoxLayout(scrollArea);
	vLayout2->addWidget(errorList_widget);
	vLayout2->setSpacing(0);
	vLayout2->setContentsMargins(5,5,5,5);	
	
	splitter = new QSplitter;
	splitter->addWidget(image_widget);
	splitter->addWidget(listWidget);
	main_layout = new QHBoxLayout();
	main_layout->addWidget(setWidget);
	main_layout->addWidget(splitter);
	main_layout->setSpacing(0);
	main_layout->setContentsMargins(0, 0, 0, 0);
	setLayout(main_layout); 
	connect(buttonGroupCamera,SIGNAL(buttonClicked(int)),this,SLOT(slots_turnCameraPage(int)));
	iButtonMode = 0;

	QDesktopWidget* desktopWidget = QApplication::desktop();
	QRect screenRect = desktopWidget->screenGeometry();
	int width = screenRect.width();

 	listSplitter.append((width-300));
 	listSplitter.append(300);
  	splitter->setSizes(listSplitter);

	setWidget->setVisible(false);
	//slots_turnCameraPage(0);
	iCameraNo = 0;
	buttonGroupCamera->button(0)->setText(QString("*%1").arg(1));
	buttonGroupCamera->button(0)->setStyleSheet("background-color:green");

	connect(image_widget,SIGNAL(signals_showCarve()),this,SLOT(slots_showCarve()));
	connect(image_widget,SIGNAL(signals_hideCarve()),this,SLOT(slots_hideCarve()));

	connect(errorList_widget,SIGNAL(signals_showErrorImage(QImage*, int, int, double, int, int, QList<QRect>, int )),image_widget, SLOT(slots_showErrorImage(QImage*, int, int, double, int, int, QList<QRect>, int)));
}

WidgetCarveSetting::~WidgetCarveSetting()
{
	delete widgetCarveImage;
}
void WidgetCarveSetting::slots_changeButtonMode()
{
	if (0 == iButtonMode)
	{
		QPixmap pixmap(":/sysButton/hide");
		expandButton->setIcon(pixmap);

		pStackedCarve->setVisible(true);
		setLayout(main_layout);
		iButtonMode = 1;
	}
	else
	{
		QPixmap pixmap(":/sysButton/expand");
		expandButton->setIcon(pixmap);

		pStackedCarve->setVisible(false);
		setLayout(main_layout);
		iButtonMode = 0;
	}
}
void WidgetCarveSetting::slots_turnCameraPage(int index)
{
	Widget_CarveImage *tempCarveiamge = dynamic_cast<Widget_CarveImage *>(pStackedCarve->widget(pStackedCarve->currentIndex()));
	if (tempCarveiamge->bIsTestMode)
	{
		if (QMessageBox::Yes == QMessageBox::question(this,tr("tip"),
			tr("Testing,do you want to stop test?"),QMessageBox::Yes | QMessageBox::No))
		{
			tempCarveiamge->slots_startTest();
		}
		else{
			return;
		}
	}
	buttonGroupCamera->button(pStackedCarve->currentIndex())->setStyleSheet("background-color:gray");
	buttonGroupCamera->button(pStackedCarve->currentIndex())->setText(QString("%1").arg(pStackedCarve->currentIndex()+1));
	
	pStackedCarve->setCurrentIndex(index);
	buttonGroupCamera->button(index)->setText(QString("*%1").arg(index+1));
	buttonGroupCamera->button(index)->setStyleSheet("background-color:green");

	//????????????
 	tempCarveiamge = dynamic_cast<Widget_CarveImage *>(pStackedCarve->widget(index));
	tempCarveiamge->slots_readDelayPara();
	Widget_CarveInfo *tempCarveInfo = tempCarveiamge->pWidgetCarveInfo;
	tempCarveInfo->ui.spinBox_realNo->setValue(pMainFrm->m_sCarvedCamInfo[index].m_iToRealCamera+1);
	tempCarveInfo->ui.spinBox_Ro->setValue(pMainFrm->m_sCarvedCamInfo[index].m_iImageAngle);

	//????????????
	if (1 == pMainFrm->m_sCarvedCamInfo[index].m_iTrigger)
	{
		tempCarveInfo->ui.comboBox_triggerType->setCurrentIndex(1);
	}
	else
	{
		tempCarveInfo->ui.comboBox_triggerType->setCurrentIndex(0);
	}
	iCameraNo = index;
	//image_widget->showMaxImage(iCameraNo);
	tempCarveInfo->ui.spinBox_exposureTime->setValue(pMainFrm->m_sRealCamInfo[pMainFrm->m_sCarvedCamInfo[index].m_iToRealCamera].m_iShuter);
	//tempCarveiamge->updateActiveImg(index);
	tempCarveiamge->intoWidget();
	tempCarveiamge->slots_cancel();
}

void WidgetCarveSetting::slots_showCarve()
{
	setWidget->setVisible(true);
}
void WidgetCarveSetting::slots_hideCarve()
{
	setWidget->setVisible(false);
}
