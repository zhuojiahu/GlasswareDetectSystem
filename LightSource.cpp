#include "LightSource.h"
#include <QLibrary>
#include <qDebug>
#include "glasswaredetectsystem.h"
extern GlasswareDetectSystem *pMainFrm;
LightSource::LightSource(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initLight();
	initConfig();
}

LightSource::~LightSource()
{

}
void LightSource::closeEvent(QCloseEvent *event)
{
	//�ر�����
	slot_cancel();
}
void LightSource::initLight()
{
	signal_mapper = new QSignalMapper(this);
	for(int i=0;i<60;i++)
	{
		QPushButton *test = new QPushButton();
		connect(test,SIGNAL(clicked()),signal_mapper,SLOT(map()));
		signal_mapper->setMapping(test,i);
		test->setMinimumWidth(20);
		test->setMinimumHeight(150);
		test->setText(QString("%1").arg(i+1));
		test->setStyleSheet("background-color:gray;border:1px solid rgb(96,96,96)");
		ui.horizontalLayout->addWidget(test);
		ui.horizontalLayout->setSpacing(0);
		nBrightness[i] = 50;
		nListButton.push_back(test);
	}
	ui.checkBox->setStyleSheet("QCheckBox::indicator {width:15px;height:15px;border-radius:7px}""QCheckBox::indicator:checked {background-color:green;}""QCheckBox::indicator:unchecked {background-color:red;}");
	connect(signal_mapper, SIGNAL(mapped(int)), this, SLOT(slots_clickAccont(int)));
	connect(ui.horizontalSlider,SIGNAL(valueChanged(int)), ui.spinBox, SLOT(setValue(int)));
	connect(ui.spinBox, SIGNAL(valueChanged(int)), ui.horizontalSlider, SLOT(setValue(int)));
	connect(ui.pushButton_5,SIGNAL(clicked()),this,SLOT(slot_login()));
	connect(ui.pushButton_6,SIGNAL(clicked()),this,SLOT(slot_cancel()));
	connect(ui.radioButton,SIGNAL(toggled(bool)),this,SLOT(slot_radioButton(bool)));
	signal_mapper2 = new QSignalMapper(this);
	connect(ui.pushButton,SIGNAL(clicked()),signal_mapper2,SLOT(map()));
	connect(ui.pushButton_2,SIGNAL(clicked()),signal_mapper2,SLOT(map()));
	connect(ui.pushButton_3,SIGNAL(clicked()),signal_mapper2,SLOT(map()));
	connect(ui.pushButton_4,SIGNAL(clicked()),signal_mapper2,SLOT(map()));
	connect(ui.pushButton_7,SIGNAL(clicked()),signal_mapper2,SLOT(map()));
	signal_mapper2->setMapping(ui.pushButton,1);
	signal_mapper2->setMapping(ui.pushButton_2,2);
	signal_mapper2->setMapping(ui.pushButton_3,3);
	signal_mapper2->setMapping(ui.pushButton_4,4);
	signal_mapper2->setMapping(ui.pushButton_7,5);
	connect(signal_mapper2, SIGNAL(mapped(int)), this, SLOT(slot_setPushbotton(int)));
	ui.radioButton->setChecked(false);
	ui.pushButton_6->setVisible(false);
	ui.pushButton_3->setVisible(false);
}

void LightSource::initConfig()
{
	nPositon =0;
	nNetID = 0;
	nStyle = 0;
	nState = 0;
	setWindowIcon(QIcon("./Resources/LOGO.png"));
	setWindowTitle(tr("Light set"));
	char* szStr = "LED_DIST.dll";
	WCHAR szTest[256];
	memset(szTest,0,sizeof(szTest));
	MultiByteToWideChar(CP_ACP,0,szStr,strlen(szStr)+1,szTest,sizeof(szTest)/sizeof(szTest[0]));
	HMODULE hdll = LoadLibrary(szTest);
	nLed_Connect = (LED_Connect)GetProcAddress(hdll,"LED_DIST_Connect");
	nLed_readInfo = (LED_DIST_New_ReadInfo)GetProcAddress(hdll,"LED_DIST_New_ReadInfo");
	nled_disconnect = (LED_DIST_Disconnect)GetProcAddress(hdll,"LED_DIST_Disconnect");
	nled_readAll = (LED_DIST_New_ReadAll)GetProcAddress(hdll,"LED_DIST_New_ReadAll");
	//nled_setArea = (LED_DIST_New_SetArea)GetProcAddress(hdll,"LED_DIST_New_SetArea");
	nled_setAll = (LED_DIST_New_SetAll)GetProcAddress(hdll,"LED_DIST_New_SetAll");
	nled_SelectStyle = (LED_DIST_New_SelectStyle)GetProcAddress(hdll,"LED_DIST_New_SelectStyle");
	nled_SetLightTime = (LED_DIST_New_SetLightTime)GetProcAddress(hdll,"LED_DIST_New_SetLightTime");
	nled_SaveAll = (LED_DIST_New_SaveAll)GetProcAddress(hdll,"LED_DIST_New_SaveAll");
}
void LightSource::slot_login()
{
	QString str=ui.lineEdit_2->text();
	QByteArray ba = str.toLatin1();
	char* ipAddress=ba.data();
	nNetID = nLed_Connect(ipAddress, 8234);
	ui.pushButton_6->setVisible(true);
	ui.pushButton_5->setVisible(false);
	ui.checkBox->setChecked(true);
	nState = 1;
	getLight();
	getStyle();
	QMessageBox::information(this,tr("Infomation"),tr("login success"));
}
void LightSource::slot_cancel()
{
	nled_disconnect(nNetID);
	ui.pushButton_6->setVisible(false);
	ui.pushButton_5->setVisible(true);
	ui.checkBox->setChecked(false);
	nListButton[nPositon]->setStyleSheet("background-color:gray;border:1px solid rgb(96,96,96)");//background-color:green
	nState = 0;
}
void LightSource::slots_clickAccont(int i)
{
	//���ð�ť��ʽ
	if(nPositon != i)
	{
		if(nBrightness[nPositon]!=0)
		{
			nListButton[nPositon]->setStyleSheet("background-color:green");//background-color:green
		}else{
			nListButton[nPositon]->setStyleSheet("background-color:gray");//background-color:green
		}
		nPositon = i;
	}
	//nListButton[i]->setStyleSheet("border:4px solid rgb(0,0,255)");//background-color:green
	ui.spinBox->setValue(nBrightness[i]);
	if(nBrightness[i]!=0)
	{
		nListButton[i]->setStyleSheet("background-color:green;border:4px solid rgb(0,0,255)");
	}else{
		nListButton[i]->setStyleSheet("background-color:gray;border:4px solid rgb(0,0,255)");
	}
	//��ȡĳ���ڵ������

}
void LightSource::slot_radioButton(bool test)
{
	if(test)
	{
		nStyle = 1;
	}else{
		nStyle = 2;
	}
	getStyle();
}
void LightSource::slot_setPushbotton(int i)
{
	switch (i)
	{
	case 1://�ָ�Ĭ������
		if(ui.radioButton->isChecked())
		{
			for(int i=0;i<60;i++)
			{
				nBrightness[i]=50;
			}
		}else{
			for(int i=0;i<60;)
			{
				nBrightness[i] = 50;
				nBrightness[i+1] = 50;
				nBrightness[i+2] = 0;
				nBrightness[i+3] = 0;
				i+=4;
			}
		}
		getStyle();
	case 4:
		SaveParameter();
		break;
	case 5:
		nBrightness[nPositon] = ui.spinBox->text().toInt();//���浥��������ȵȼ�
		if(nBrightness[nPositon]!=0)
		{
			nListButton[nPositon]->setStyleSheet("background-color:green;border:4px solid rgb(0,0,255)");
		}else{
			nListButton[nPositon]->setStyleSheet("background-color:gray;border:4px solid rgb(0,0,255)");
		}
		break;
	default:
		break;
	}
}
void LightSource::SaveParameter()
{
	if(nState)
	{
		nled_setAll(nNetID,nStyle,nBrightness);//���������������ȵȼ�
		nled_SetLightTime(nNetID,ui.lineEdit->text().toInt());//���õ���ʱ��
		nled_SelectStyle(nNetID,nStyle);//����������ʽ
		nled_SaveAll(nNetID);//��������
	}
}
void LightSource::getStyle()
{
	//��ȡ������ʽ
	char str[67];
	memset(str,0,67);
	if(67 == nled_readAll(nNetID,nStyle,str) && nState)
	{
		for(int i=4;i<64;i++)
		{
			nBrightness[i-4] = str[i];
		}
	}
	ui.spinBox->setValue(nBrightness[0]);
	for(int i=0;i<60;i++)
	{
		if(nBrightness[i]!=0)
		{
			nListButton[i]->setStyleSheet("background-color:green");
		}else{
			nListButton[i]->setStyleSheet("background-color:gray");
		}
	}
}
void LightSource::getLight()
{
	//��ȡ��Դ��ʽ
	char sTest[12];
	memset(sTest,0,12);
	if(12 == nLed_readInfo(nNetID,sTest) && nState)
	{
		nStyle = sTest[4];
		if(nStyle == 1)
		{
			ui.radioButton->setChecked(true);//��ʽ1
		}else if(nStyle == 2)
		{
			ui.radioButton_2->setChecked(true);//��ʽ2
		}
		if((int)sTest[6]<0)
		{
			ui.lineEdit->setText(QString::number((sTest[5]<<8) + sTest[6]+256));
		}else{
			ui.lineEdit->setText(QString::number((sTest[5]<<8) + sTest[6]));
		}
	}
}