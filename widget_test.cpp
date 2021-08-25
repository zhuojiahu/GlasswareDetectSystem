#include "widget_test.h"
#include "glasswaredetectsystem.h"
#include "EquipRuntime.h"

extern GlasswareDetectSystem *pMainFrm;

WidgetTest::WidgetTest(QWidget *parent)
	: QWidget(parent)
{
	ifshowImage = 0;
	iSaveMode = -1;
	ui.setupUi(this);
	for (int i = 0;i<pMainFrm->m_sSystemInfo.iCamCount;i++)
	{
		connect(pMainFrm->pdetthread[i], SIGNAL(signals_updateAlert(int)), this, SLOT(slots_updateAlert(int)));
	}

	//connect(this,SIGNAL(signals_ShowWarning(int , QString )),pMainFrm->widget_Warning,SLOT(slots_ShowWarning(int , QString )));
	int widgetWidth = pMainFrm->statked_widget->geometry().width();
	init();
	initWidgetName();
	ui.btnChoseCamera->setVisible(false);
	ui.btnChoseErrorType->setVisible(false);
	timerUpdateIOCardCounter = new QTimer(this);
	timerUpdateIOCardCounter->setInterval(1000);//每毫秒刷新一次计数
	CameraOffAreet = new QTimer(this);
	CameraOffAreet->setInterval(10000);
 	connect(CameraOffAreet, SIGNAL(timeout()), this, SLOT(slots_CameraOffAreet()));   
	if (pMainFrm->m_sSystemInfo.bCameraOffLineSurveillance)
	{
		CameraOffAreet->start();
	}

	//增加图片刷新选择
	ui.comboBox->insertItem(0,tr("Refresh All")); 
	ui.comboBox->insertItem(1,tr("Only Bad Images Are Refreshed"));
	ui.comboBox->insertItem(2,tr("All Not Refresh"));

	ui.label_MissNumber->setText("");
	ui.spinBox_OffLineNumber->setVisible(false);
	ui.checkBox_CameraContinueReject->setVisible(false);
	ui.label_rejectNumber->setVisible(false);
	ui.spinBox_RejectNo->setVisible(false);

	QSettings iniCarveSet(pMainFrm->m_sConfigInfo.m_strGrabInfoPath,QSettings::IniFormat);
	QString strSession;
	iIOCardOffSet = iniCarveSet.value("/system/iIOCardOffSet",200).toInt();
	ui.Distance1to4->setText(QString::number(iIOCardOffSet));

	//瓶口瓶底增加控件控制PLC通讯
	nConsole = new QConsole();
	nConsole->setStyleSheet("background:white");
	connect(ui.pushButton_set,SIGNAL(clicked()),this,SLOT(slot_openPlcSet()));
	if(pMainFrm->m_sSystemInfo.m_iSystemType == 2)//隐藏瓶身的按钮，防止前后壁误操作到第二块接口卡
	{
		ui.pushButton_set->setVisible(true);
	}else{
		ui.pushButton_set->setVisible(false);
	}
}
WidgetTest::~WidgetTest()
{
	delete widget_ErrorType;
	delete widget_Camera;
	delete nConsole;
}
void WidgetTest::slot_openPlcSet()
{
	nConsole->show();
}
void WidgetTest::slots_intoWidget()
{	
	connect(timerUpdateIOCardCounter, SIGNAL(timeout()), this, SLOT(slots_updateIOcardCounter()));
	timerUpdateIOCardCounter->stop();
	Sleep(10);
	timerUpdateIOCardCounter->start();

	ui.checkBox_CameraOffLine->setChecked(pMainFrm->m_sSystemInfo.bCameraOffLineSurveillance);
	ui.checkBox_CameraContinueReject->setChecked(pMainFrm->m_sSystemInfo.bCameraContinueRejectSurveillance);
	ui.spinBox_OffLineNumber->setValue(pMainFrm->m_sSystemInfo.iCamOfflineNo);
	ui.spinBox_RejectNo->setValue(pMainFrm->m_sSystemInfo.iCamContinueRejectNumber);

	if (pMainFrm->m_sSystemInfo.m_iSaveNormalErrorImageByTime)
	{
		ui.checkBox_saveFailureNormalImage->setChecked(true);
	}
	else{
		ui.checkBox_saveFailureNormalImage->setChecked(false);
	}
	if (pMainFrm->m_sSystemInfo.m_iSaveStressErrorImageByTime)
	{
		ui.checkBox_saveFailureStressImage->setChecked(true);
	}
	else{
		ui.checkBox_saveFailureStressImage->setChecked(false);
	}

	switch (pMainFrm->m_sRunningInfo.m_eSaveImageType)
	{
	case NotSave:
		ui.comboBox_SaveMode->setCurrentIndex(0);
		iLastSaveMode = 0;
		iSaveMode = 0;
		break;
	case AllImageInCount:
		ui.comboBox_SaveMode->setCurrentIndex(1);
		iLastSaveMode = 1;
		iSaveMode = 1;
		break;
	case FailureImageInCount:
		ui.comboBox_SaveMode->setCurrentIndex(2);
		iLastSaveMode = 2;
		iSaveMode = 2;
		break;
	}
	
	slots_SaveModeChanged(iSaveMode);

	iSaveImgCount = 0;
	for(int i = 0; i<pMainFrm->m_sSystemInfo.iCamCount; i++)
	{
		if (iSaveImgCount < pMainFrm->m_sRunningInfo.m_iSaveImgCount[i])
		{		
			iSaveImgCount = pMainFrm->m_sRunningInfo.m_iSaveImgCount[i];
		}
	}
	ui.spinBox_Number->setValue(iSaveImgCount);
	initInformation();//更新接口卡配置
	ui.comboBox->setCurrentIndex(ifshowImage);
}
bool WidgetTest::leaveWidget()
{
	return true;
}

void WidgetTest::init()
{
	iChoseCamera = 1;
	iChoseErrorType = 1;
	iSaveMode = 0;
	iLastSaveMode = 0;
	iKickMode = 2;
	iLastKickMode = 2;
	iSaveImgCount = 0;
	iLastSaveImgCount = 0;

	m_nDelay1 = 0;
	m_nDelay2 = 0;
	m_nDelay3 = 0;
	m_nDelay4 = 0;
	m_nDelay5 = 0;
	m_nDelay6 = 0;

	m_nKickDelay = 0;
	m_nKickWidth = 0;
	m_nSampleDelay = 0;
	m_nSampleWidth = 0;
	
	ui.read->setVisible(true);
	ui.label_Number->setEnabled(false);
	ui.spinBox_Number->setEnabled(false);
	ui.comboBox_SaveMode->setCurrentIndex(0);

	connect(ui.comboBox_SaveMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slots_SaveModeChanged(int)));
	QButtonGroup * buttonGroup  = new QButtonGroup(this);
	buttonGroup->addButton(ui.radioButton_bad,0);
	buttonGroup->addButton(ui.radioButton_good,1);
	buttonGroup->addButton(ui.radioButton_normal,2);
	connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slots_KickModeChanged(int)));
	ui.radioButton_normal->setChecked(true);

	widget_Camera = new Widget_Camera();
	ui.layoutChoseCamera->addWidget(widget_Camera);
	widget_ErrorType = new Widget_ErrorType();
	ui.layoutChoseErrorType->addWidget(widget_ErrorType);

	ui.btnOK->setVisible(false);
	ui.btnCancel->setVisible(false);

	initEquipAlarmTablewidget();
	connect(ui.btnChoseCamera, SIGNAL(clicked()), this, SLOT(slots_ChoseCamera()));
	connect(ui.btnChoseErrorType, SIGNAL(clicked()), this, SLOT(slots_ChoseErrorType()));
	connect(ui.btnOK_Save, SIGNAL(clicked()), this, SLOT(slots_OKSave()));
	connect(ui.btnOK_CameraSurveillance, SIGNAL(clicked()), this, SLOT(slots_OKCameraSurveillance()));
	connect(ui.pushButton_2,SIGNAL(clicked()), this, SLOT(slots_ifCheckShowImage()));
	connect(ui.read, SIGNAL(clicked()), this, SLOT(slots_readDelay()));
	connect(ui.settocard, SIGNAL(clicked()), this, SLOT(slots_setToCard()));
	connect(ui.settofile, SIGNAL(clicked()), this, SLOT(slots_setToFile()));

	connect(ui.advance1, SIGNAL(clicked()), this, SLOT(slots_advance1()));
	connect(ui.pushButton_choseAllCamera, SIGNAL(clicked()), this, SLOT(slots_choseAllCamera()));
	connect(ui.pushButton_choseNoneCamera, SIGNAL(clicked()), this, SLOT(slots_choseNoneCamera()));

	connect(ui.pushButton_choseAllErrorType, SIGNAL(clicked()), this, SLOT(slots_choseAllErrorType()));
	connect(ui.pushButton_choseNoneErrorType, SIGNAL(clicked()), this, SLOT(slots_choseNoneErrorType()));
	connect(ui.checkBox_EquipAlarm,SIGNAL(clicked(bool)),this,SLOT(slots_EquipAlarmCheckBox(bool)));
	connect(ui.ShowAlarmSet_btn,SIGNAL(clicked(bool)),this,SLOT(slots_EquipAlarmInfoShowbtn(bool)));
	connect(ui.btnOk_EquipAlarm,SIGNAL(clicked()),this,SLOT(slots_EquipAlarmSave()));
	connect(ui.btn_ClearAlarm,SIGNAL(clicked()),this,SLOT(slots_EquipAlarmClear()));
	connect(EquipRuntime::Instance(),SIGNAL(SendAlarms(int,bool)),this,SLOT(slots_SetEquipAlarmSatus(int,bool)));
	initInformation();
}

void WidgetTest::initEquipAlarmTablewidget()
{
	ui.tableWidget_EquipAlarm->setColumnWidth(0,22);
	ui.tableWidget_EquipAlarm->setColumnWidth(1,150);
	ui.tableWidget_EquipAlarm->setColumnWidth(2,400);
	for (int i=0;i<ui.tableWidget_EquipAlarm->rowCount();i++)
	{
		QTableWidgetItem *check=new QTableWidgetItem;
		check->setCheckState (Qt::Unchecked);
		check->setFlags(check->flags() & (~Qt::ItemIsEditable) & (~Qt::ItemIsSelectable));
		ui.tableWidget_EquipAlarm->setItem(i,0,check); //插入复选框
		QTableWidgetItem *m_item1=new QTableWidgetItem;
		m_item1->setTextAlignment(Qt::AlignCenter); 
		ui.tableWidget_EquipAlarm->setItem(i,1,m_item1); 
		QTableWidgetItem *m_item2=new QTableWidgetItem;
		ui.tableWidget_EquipAlarm->setItem(i,2,m_item2); 

		CameraStatusLabel *pLabel = new CameraStatusLabel(this);
		pLabel->setAlignment(Qt::AlignCenter);
		pLabel->setText(QString::number(i+1));
		QFont font ( "Microsoft YaHei", 7, 50); 
		pLabel->setFont(font);
		m_EquipAlarmStatusList<<pLabel;
		if (i < 10 )
			ui.gridLayout_EquipAlarm->addWidget(pLabel,0,i);
		else
			ui.gridLayout_EquipAlarm->addWidget(pLabel,1,i-10);
		
	}
	ui.tableWidget_EquipAlarm->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
	ui.tableWidget_EquipAlarm->horizontalHeader()->setStretchLastSection(true);

	ui.checkBox_EquipAlarm->setChecked(pMainFrm->m_sRuntimeInfo.isEnable);
	for (int i=0;i<pMainFrm->m_sRuntimeInfo.total;i++)
	{
		if (pMainFrm->m_sRuntimeInfo.AlarmsEnable.at(i))
			ui.tableWidget_EquipAlarm->item(i,0)->setCheckState(Qt::Checked);
		else
			ui.tableWidget_EquipAlarm->item(i,0)->setCheckState(Qt::Unchecked);
		ui.tableWidget_EquipAlarm->item(i,1)->setText(QString::number((pMainFrm->m_sRuntimeInfo.AlarmsDays.at(i))));
		ui.tableWidget_EquipAlarm->item(i,2)->setText(pMainFrm->m_sRuntimeInfo.AlarmsInfo.at(i));
	}
	slots_EquipAlarmInfoShowbtn(false);
	slots_EquipAlarmCheckBox(pMainFrm->m_sRuntimeInfo.isEnable);

	if (pMainFrm->m_sRuntimeInfo.isEnable)
	{
		EquipRuntime::Instance()->start();
	}
	else
	{
		EquipRuntime::Instance()->stop();
	}
}

void WidgetTest::initInformation()
{
	if (pMainFrm->m_sSystemInfo.m_bIsIOCardOK)
	{
		if(pMainFrm->m_sSystemInfo.m_iSystemType != 2)
		{
			m_nDelay1 = pMainFrm->m_vIOCard[0]->readParam(32);
			m_nDelay2 = pMainFrm->m_vIOCard[0]->readParam(61);
			m_nDelay3 = pMainFrm->m_vIOCard[0]->readParam(57);
			m_nDelay4 = pMainFrm->m_vIOCard[0]->readParam(59);
			m_nDelay5 = pMainFrm->m_vIOCard[0]->readParam(157);
			m_nDelay6 = pMainFrm->m_vIOCard[0]->readParam(158);
		}else{
			m_nDelay1 = pMainFrm->m_vIOCard[0]->readParam(32);
			m_nDelay2 = pMainFrm->m_vIOCard[0]->readParam(61);
			m_nDelay3 = pMainFrm->m_vIOCard[0]->readParam(159);
			m_nDelay4 = pMainFrm->m_vIOCard[0]->readParam(160);
		}
		m_nKickDelay = pMainFrm->m_vIOCard[0]->readParam(49);
		m_nKickWidth = pMainFrm->m_vIOCard[0]->readParam(46);
		m_nSampleDelay = pMainFrm->m_vIOCard[0]->readParam(42);
	}
	updateIOCardParam();

}
void WidgetTest::initWidgetName()
{
	ui.widget_LoginHoldTime->setWidgetName(tr("Login Hold Time"));
	ui.widget_LoginHoldTime->widgetName->setMaximumHeight(25);
	ui.namelayout_LoginHoldTime->addWidget(ui.widget_LoginHoldTime->widgetName);//,Qt::AlignTop);

	ui.widget_saveImageSet->setWidgetName(tr("Save Mode"));
	ui.widget_saveImageSet->widgetName->setMaximumHeight(25);
	ui.namelayout_saveImage->addWidget(ui.widget_saveImageSet->widgetName);//,Qt::AlignTop);

	ui.widget_CameraSurveillance->setWidgetName(tr("Camera Surveillance"));
	ui.widget_CameraSurveillance->widgetName->setMaximumHeight(25);
	ui.namelayout_CameraSurveillance->addWidget(ui.widget_CameraSurveillance->widgetName);

	ui.widget_IOCardSet->setWidgetName(tr("IOCard Delay"));
	ui.widget_IOCardSet->widgetName->setMaximumHeight(25);
	ui.widget_IOCounter->addWidget(ui.widget_IOCardSet->widgetName);

	//modif 2020-11-05 Joge
	ui.widget_EquipAlarm->setWidgetName(tr("Equipment Maintenance Alarm Set"));
}
void WidgetTest::slots_ChoseCamera()
{
	if (0 == iChoseCamera)
	{
		ui.groupBox_ChoseCamera->setVisible(true);
		iChoseCamera = 1;
	}
	else
	{
		ui.groupBox_ChoseCamera->setVisible(false);
		iChoseCamera = 0;
	}
}
void WidgetTest::slots_ChoseErrorType()
{
	if (0 == iChoseErrorType)
	{
		ui.groupBox_ChoseErrorType->setVisible(true);
		iChoseErrorType = 1;
	}
	else
	{
		ui.groupBox_ChoseErrorType->setVisible(false);
		iChoseErrorType = 0;
	}
}
void WidgetTest::slots_OKSave()
{	
	QSettings settingSave(pMainFrm->m_sConfigInfo.m_strConfigPath,QSettings::IniFormat);
	settingSave.setIniCodec(QTextCodec::codecForName("GBK"));
	settingSave.beginGroup("system");
	if (ui.checkBox_saveFailureNormalImage->isChecked())
	{
		pMainFrm->m_sSystemInfo.m_iSaveNormalErrorImageByTime = 1;
		settingSave.setValue("SaveNormalErrorImageByTime",1);
	}
	else{
		pMainFrm->m_sSystemInfo.m_iSaveNormalErrorImageByTime = 0;
		settingSave.setValue("SaveNormalErrorImageByTime",0);
	}
	if (ui.checkBox_saveFailureStressImage->isChecked())
	{
		pMainFrm->m_sSystemInfo.m_iSaveStressErrorImageByTime = 1;
		settingSave.setValue("SaveStressErrorImageByTime",1);
	}
	else{
		pMainFrm->m_sSystemInfo.m_iSaveStressErrorImageByTime = 0;
		settingSave.setValue("SaveStressErrorImageByTime",0);
	}
	settingSave.endGroup();

	if	(iLastSaveMode != iSaveMode||iSaveImgCount != ui.spinBox_Number->value())
	{
		iLastSaveMode = iSaveMode;
		iLastSaveImgCount = iSaveImgCount;

		iSaveImgCount = ui.spinBox_Number->value();
		for (int i = 0;i<pMainFrm->m_sSystemInfo.iCamCount;i++ )
		{
			if (widget_Camera->bIsChosed[i])
			{
				pMainFrm->m_sRunningInfo.m_iSaveImgCount[i] = iSaveImgCount;
			}
			else
			{
				pMainFrm->m_sRunningInfo.m_iSaveImgCount[i] = 0;
			}
		}
		switch (iSaveMode)
		{
		case 0:
			pMainFrm->m_sRunningInfo.m_eSaveImageType = NotSave;
			break;
		case 1:
			pMainFrm->m_sRunningInfo.m_eSaveImageType = AllImageInCount;
			break;
		case 2:
			pMainFrm->m_sRunningInfo.m_eSaveImageType = FailureImageInCount;
			break;
		}
	}
	for (int i = 0; i<CAMERA_MAX_COUNT;i++)
	{
		pMainFrm->m_sSystemInfo.m_bSaveCamera[i] = widget_Camera->bIsChosed[i];
	}
	for (int i = 0; i<ERRORTYPE_MAX_COUNT;i++)
	{
		pMainFrm->m_sSystemInfo.m_bSaveErrorType[i] = widget_ErrorType->bIsChosed[i];
	}
}
void WidgetTest::slots_OKCameraSurveillance()
{

	pMainFrm->m_sSystemInfo.bCameraOffLineSurveillance = ui.checkBox_CameraOffLine->isChecked();
	pMainFrm->m_sSystemInfo.bCameraContinueRejectSurveillance = ui.checkBox_CameraContinueReject->isChecked();
	pMainFrm->m_sSystemInfo.iCamOfflineNo = ui.spinBox_OffLineNumber->value();
	pMainFrm->m_sSystemInfo.iCamContinueRejectNumber = ui.spinBox_RejectNo->value();


	QSettings iniStatisSet(pMainFrm->m_sConfigInfo.m_strConfigPath,QSettings::IniFormat);
	iniStatisSet.setIniCodec(QTextCodec::codecForName("GBK"));

	iniStatisSet.setValue("/system/bCameraOffLineSurveillance",pMainFrm->m_sSystemInfo.bCameraOffLineSurveillance);
	iniStatisSet.setValue("/system/bCameraContinueRejectSurveillance",pMainFrm->m_sSystemInfo.bCameraContinueRejectSurveillance);	
	iniStatisSet.setValue("/system/iCamOfflineNo",pMainFrm->m_sSystemInfo.iCamOfflineNo);
	iniStatisSet.setValue("/system/iCamContinueRejectNumber",pMainFrm->m_sSystemInfo.iCamContinueRejectNumber);

	if(pMainFrm->m_sSystemInfo.bCameraOffLineSurveillance)
	{
		if (CameraOffAreet->isActive())
		{
			ui.CameraMonitor_Status->setText(tr("Camera Offline monitoring already start"));
			QTimer::singleShot(1000,this,SLOT(SetCameraMonitorStatus()));
			return;
		}
		CameraOffAreet->start();
		ui.CameraMonitor_Status->setText(tr("Camera Offline monitoring start"));
		QTimer::singleShot(1000,this,SLOT(SetCameraMonitorStatus()));
	}
	else
	{
		if (!CameraOffAreet->isActive())
		{
			ui.CameraMonitor_Status->setText(tr("Camera Offline monitoring already stop"));
			QTimer::singleShot(1000,this,SLOT(SetCameraMonitorStatus()));
			return;
		}
		CameraOffAreet->stop();
		ui.CameraMonitor_Status->setText(tr("Camera Offline monitoring stop"));
		QTimer::singleShot(1000,this,SLOT(SetCameraMonitorStatus()));
	}
}

void WidgetTest::slots_CameraOffAreet()
{
	bool test= false;
	for(int i=0;i < pMainFrm->m_sSystemInfo.iRealCamCount;i++)
	{
		QString DeviceName = QString(pMainFrm->struGrabCardPara[i].strDeviceName);
		if ( DeviceName  != "MER" )
		{
			continue;
		}
		int temp = 0;
		bool ret = true;
		try
		{
			ret = ((CDHGrabberMER*)pMainFrm->m_sRealCamInfo[i].m_pGrabber)->MERGetParamInt(MERExposure,temp,temp,temp);
		}
		catch(...)
		{

		}
		if(!ret)
		{
			//相机掉线报警
			pMainFrm->cameraStatus_list.at(i)->SetCameraStatus(2);
			emit signals_ShowWarning(i,QString(tr("Camera %1 Offline ! \nPlease check the camera and restart the software!")).arg(i+1));
			test = true;
			break;
		}
	}
	if(test)
	{
		emit signal_UsualSend(1);
	}else{
		emit signal_UsualSend(0);
	}
}

void WidgetTest::SetCameraMonitorStatus()
{
	ui.CameraMonitor_Status->setText("");
}

void WidgetTest::slots_EquipAlarmCheckBox(bool checked)
{
	ui.ShowAlarmSet_btn->setVisible(checked);
	ui.ShowAlarmSet_btn->setChecked(false);
	ui.ShowAlarmSet_btn->setText(tr("Show"));
	if (!checked)
	{
		ui.tableWidget_EquipAlarm->setVisible(checked);
	}
}

void WidgetTest::slots_EquipAlarmInfoShowbtn(bool checked)
{
	ui.tableWidget_EquipAlarm->setVisible(checked);
	if (checked)
		ui.ShowAlarmSet_btn->setText(tr("Hide"));
	else
		ui.ShowAlarmSet_btn->setText(tr("Show"));
}

void WidgetTest::slots_EquipAlarmSave()
{
	QSettings runtimeCfg(pMainFrm->m_sConfigInfo.m_sRuntimePath,QSettings::IniFormat);
	runtimeCfg.setIniCodec(QTextCodec::codecForName("GBK"));
	bool ret = ui.checkBox_EquipAlarm->isChecked();
	pMainFrm->m_sRuntimeInfo.isEnable = ret;
	if (ret)
		runtimeCfg.setValue("EquipAlarm/Enable",1);
	else
		runtimeCfg.setValue("EquipAlarm/Enable",0);
	runtimeCfg.setValue("EquipAlarm/total",20);
	pMainFrm->m_sRuntimeInfo.total =20;
	for (int i=0;i<pMainFrm->m_sRuntimeInfo.total;i++)
	{
		int pchecked = ui.tableWidget_EquipAlarm->item(i,0)->checkState();
		pMainFrm->m_sRuntimeInfo.AlarmsEnable[i] = bool(pchecked);
		pMainFrm->m_sRuntimeInfo.AlarmsDays[i] = ui.tableWidget_EquipAlarm->item(i,1)->text().toInt();
		pMainFrm->m_sRuntimeInfo.AlarmsInfo[i] = ui.tableWidget_EquipAlarm->item(i,2)->text();

		runtimeCfg.setValue(QString("EquipAlarm/Alarm%1_Enable").arg(i+1),ui.tableWidget_EquipAlarm->item(i,0)->checkState());
		runtimeCfg.setValue(QString("EquipAlarm/Alarm%1_Days").arg(i+1),ui.tableWidget_EquipAlarm->item(i,1)->text());
		runtimeCfg.setValue(QString("EquipAlarm/Alarm%1_Info").arg(i+1),ui.tableWidget_EquipAlarm->item(i,2)->text());
	}
	if (pMainFrm->m_sRuntimeInfo.isEnable)
	{
		EquipRuntime::Instance()->start();
	}
	else
	{
		EquipRuntime::Instance()->stop();
	}
}

void WidgetTest::slots_EquipAlarmClear()
{
	EquipRuntime::Instance()->ResetLogFile();
	pMainFrm->sVersion = pMainFrm->getVersion(NULL);
}

void WidgetTest::slots_SetEquipAlarmSatus(int index,bool pStatus)
{
	if (pStatus)
	{
		m_EquipAlarmStatusList[index]->SetCameraStatus(1);
	}else{
		m_EquipAlarmStatusList[index]->SetCameraStatus(0);
	}
}

void WidgetTest::slots_Cancel()
{
	//恢复踢废
	iKickMode = iLastKickMode;
	switch (iLastKickMode)
	{
	case 0:  
		ui.radioButton_bad->setChecked(true);
		break;
	case 1:
		ui.radioButton_good->setChecked(true);
		break;	
	case 2:
		ui.radioButton_normal->setChecked(true);
		break;
	}
	//恢复存图
	if (iSaveMode != iLastSaveMode)
	{
		iSaveMode = iLastSaveMode;
		ui.comboBox_SaveMode->setCurrentIndex(iLastSaveMode);
		iSaveImgCount = iLastSaveImgCount;
		ui.spinBox_Number->setValue(iSaveImgCount);
	}
}

void WidgetTest::slots_KickModeChanged(int iMode)
{
	iKickMode = iMode;
	if (iKickMode != iLastKickMode)
	{
		iLastKickMode = iKickMode;
		pMainFrm->m_sRunningInfo.m_iKickMode = iKickMode;
		switch (pMainFrm->m_sRunningInfo.m_iKickMode)
		{
		case 0:
			ui.radioButton_bad->setChecked(true);
			break;
		case 1:
			ui.radioButton_good->setChecked(true);
			break;
		default:
			ui.radioButton_normal->setChecked(true);
			break;
		}
	}
}

void WidgetTest::slots_SaveModeChanged(int index)
{
	iSaveMode = index;
	if (1 == iSaveMode)
	{
		ui.label_Number->setEnabled(true);
		ui.spinBox_Number->setEnabled(true);
		if (1 == iChoseErrorType)
		{
			slots_ChoseErrorType();
		}
		if (0 == iChoseCamera)
		{
			slots_ChoseCamera();
		}
	}
	else if (2 == iSaveMode)
	{
		ui.label_Number->setEnabled(true);
		ui.spinBox_Number->setEnabled(true);
		if (0 == iChoseErrorType)
		{
			slots_ChoseErrorType();
		}
		if (0 == iChoseCamera)
		{
			slots_ChoseCamera();
		}
	}
	else
	{
		ui.label_Number->setEnabled(false);
		ui.spinBox_Number->setEnabled(false);
		if (1 == iChoseErrorType)
		{
			slots_ChoseErrorType();
		}
		if (1 == iChoseCamera)
		{
			slots_ChoseCamera();
		}
	}
}
void WidgetTest::slots_readDelay()
{
	if (pMainFrm->m_sSystemInfo.m_bIsIOCardOK)
	{
		int i,j,k;
		i=pMainFrm->m_vIOCard[0]->readStatus(13);
		j=pMainFrm->m_vIOCard[0]->readStatus(46);
		k=pMainFrm->m_vIOCard[0]->readStatus(15);
		m_nKickWidth = pMainFrm->m_vIOCard[0]->readParam(46);
		if(pMainFrm->m_sSystemInfo.m_iSystemType != 2)//前壁的参数设置
		{
			m_nKickDelay = j-i-iIOCardOffSet;
			ui.KickDelay->setText(QString::number(m_nKickDelay));
			ui.lineDelay->setText("0");
			ui.KickWidth->setText(QString::number(m_nKickWidth));
		}else{  //瓶口瓶底的参数设置
			m_nKickDelay = k-i-iIOCardOffSet;
			ui.KickDelay->setText(QString::number(m_nKickDelay));
			m_nSampleDelay = j-k-iIOCardOffSet;
			ui.lineDelay->setText(QString::number(m_nSampleDelay));
			ui.KickWidth->setText(QString::number(m_nKickWidth));
		}
	}
}
void WidgetTest::slots_setToCard()
{
	getIOCardParam();
	if (m_nDelay1>0xFFFF||m_nDelay2>0xFFFF||m_nDelay3>0xFFFF||m_nDelay4>0xFFFF||m_nDelay5>0xFFFF||m_nDelay6>0xFFFF||m_nKickWidth>0xFFFF||m_nKickDelay>0xFFFF)
	{
		QString str(tr("The set value is out of range!"));
		QMessageBox::information(this,"Error",str);	
		return; 
	}
	if (pMainFrm->m_sSystemInfo.m_bIsIOCardOK)
	{
		if(pMainFrm->m_sSystemInfo.m_iSystemType != 2)
		{
			pMainFrm->m_vIOCard[0]->writeParam(32,m_nDelay1);
			pMainFrm->m_vIOCard[0]->writeParam(61,m_nDelay2);
			pMainFrm->m_vIOCard[0]->writeParam(57,m_nDelay3);
			pMainFrm->m_vIOCard[0]->writeParam(59,m_nDelay4);
			pMainFrm->m_vIOCard[0]->writeParam(157,m_nDelay5);
			pMainFrm->m_vIOCard[0]->writeParam(158,m_nDelay6);
		}else{
			pMainFrm->m_vIOCard[0]->writeParam(32,m_nDelay1);
			pMainFrm->m_vIOCard[0]->writeParam(61,m_nDelay2);
			pMainFrm->m_vIOCard[0]->writeParam(159,m_nDelay3);
			pMainFrm->m_vIOCard[0]->writeParam(160,m_nDelay4);
		}
		pMainFrm->m_vIOCard[0]->writeParam(49,m_nKickDelay);
		pMainFrm->m_vIOCard[0]->writeParam(46,m_nKickWidth);
		pMainFrm->m_vIOCard[0]->writeParam(42,m_nSampleDelay);
	}
}
void WidgetTest::slots_setToFile()
{
	getIOCardParam();
	if (m_nDelay1>0xFFFF||m_nDelay2>0xFFFF||m_nDelay3>0xFFFF||m_nDelay4>0xFFFF||m_nDelay5>0xFFFF||m_nDelay6>0xFFFF||m_nKickWidth>0xFFFF||m_nKickDelay>0xFFFF||m_nSampleDelay>0xFFFF||m_nSampleWidth>0xFFFF)
	{
		QString str(tr("The set value is out of range!"));
		QMessageBox::information(this,"Error",str);	
		return;
	}
	QString strValue,strPara;
	if(pMainFrm->m_sSystemInfo.m_iSystemType != 2)
	{
		strValue = strValue.setNum(m_nDelay1,10);
		strPara = strPara.setNum(32,10);
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);
		
		strValue = strValue.setNum(m_nDelay2,10);
		strPara = strPara.setNum(61,10);
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);
		
		strValue = strValue.setNum(m_nDelay3,10);
		strPara = strPara.setNum(57,10);
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);
		
		strValue = strValue.setNum(m_nDelay4,10);
		strPara = strPara.setNum(59,10);
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);
		
		strValue = strValue.setNum(m_nDelay5,10);
		strPara = strPara.setNum(157,10);//
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);
		
		strValue = strValue.setNum(m_nDelay6,10);
		strPara = strPara.setNum(158,10);
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);
	}else{
		strValue = strValue.setNum(m_nDelay1,10);
		strPara = strPara.setNum(32,10);
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);
		
		strValue = strValue.setNum(m_nDelay2,10);
		strPara = strPara.setNum(61,10);
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);

		strValue = strValue.setNum(m_nDelay3,10);
		strPara = strPara.setNum(159,10);
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);

		strValue = strValue.setNum(m_nDelay4,10);
		strPara = strPara.setNum(160,10);
		StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);

	}
	strValue = strValue.setNum(m_nKickDelay,10);
	strPara = strPara.setNum(49,10);
	StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);

	strValue = strValue.setNum(m_nKickWidth,10);
	strPara = strPara.setNum(46,10);

	StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);

	strValue = strValue.setNum(m_nSampleDelay,10);
	strPara = strPara.setNum(42,10);

	StateTool::WritePrivateProfileQString("PIO24B",strPara,strValue,pMainFrm->m_sSystemInfo.m_sConfigIOCardInfo[0].strCardInitFile);

	QSettings iniCarveSet(pMainFrm->m_sConfigInfo.m_strGrabInfoPath,QSettings::IniFormat);
	QString strSession;
	iniCarveSet.setValue ("/system/iIOCardOffSet",pMainFrm->test_widget->iIOCardOffSet);
}
void WidgetTest::slots_advance1()
{
	if (pMainFrm->m_sSystemInfo.m_bIsIOCardOK)
	{
		pMainFrm->m_vIOCard[0]->Show_PIO24B_DebugDialog(this);
	}
	else
	{
		QMessageBox::information(this,"ERROR","IOCard is not available");
	}
}
void WidgetTest::updateIOCardParam()
{
	QString str;

	str = str.setNum(m_nDelay1,10);
	ui.Delay1->setText(str);
	str = str.setNum(m_nDelay2,10);
	ui.Delay2->setText(str);
	str = str.setNum(m_nDelay3,10);
	ui.Delay3->setText(str);
	str = str.setNum(m_nDelay4,10);
	ui.Delay4->setText(str);
	str = str.setNum(m_nDelay5,10);
	ui.Delay5->setText(str);
	str = str.setNum(m_nDelay6,10);
	ui.Delay6->setText(str);

	str = str.setNum(m_nSampleDelay,10);
	ui.lineDelay->setText(str);

	str = str.setNum(m_nKickDelay,10);
	ui.KickDelay->setText(str);
	str = str.setNum(m_nKickWidth,10);
	ui.KickWidth->setText(str);
}
void WidgetTest::getIOCardParam()
{
	m_nDelay1 = ui.Delay1->text().toInt();
	m_nDelay2 = ui.Delay2->text().toInt();
	m_nDelay3 = ui.Delay3->text().toInt();
	m_nDelay4 = ui.Delay4->text().toInt();
	m_nDelay5 = ui.Delay5->text().toInt();
	m_nDelay6 = ui.Delay6->text().toInt();
	m_nSampleDelay = ui.lineDelay->text().toInt();
	m_nKickDelay = ui.KickDelay->text().toInt();
	m_nKickWidth = ui.KickWidth->text().toInt();
	iIOCardOffSet = ui.Distance1to4->text().toInt();
}

void WidgetTest::slots_choseAllCamera()
{
	for (int i=0;i<widget_Camera->listCheckBox.length();i++)
	{
		widget_Camera->listCheckBox.at(i)->setChecked(true);
	}
}
void WidgetTest::slots_choseNoneCamera()
{
	for (int i=0;i<widget_Camera->listCheckBox.length();i++)
	{
		widget_Camera->listCheckBox.at(i)->setChecked(false);
	}
}
void WidgetTest::slots_choseAllErrorType()
{
	for (int i=0;i<widget_ErrorType->listCheckBox.length();i++)
	{
		widget_ErrorType->listCheckBox.at(i)->setChecked(true);
	}
}
void WidgetTest::slots_choseNoneErrorType()
{
	for (int i=0;i<widget_ErrorType->listCheckBox.length();i++)
	{
		widget_ErrorType->listCheckBox.at(i)->setChecked(false);
	}
}

void WidgetTest::slots_updateIOcardCounter()
{
	if (pMainFrm->m_sSystemInfo.m_bIsIOCardOK)
	{
		int iCounter;
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(16);
		ui.label_frequency->setText(tr("Frequency:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(0);
		pMainFrm->nIOCard[0] = iCounter;
		ui.label_IN0->setText(tr("IN0:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(1);
		pMainFrm->nIOCard[1] = iCounter;
		ui.label_IN1->setText(tr("IN1:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(2);
		pMainFrm->nIOCard[2] = iCounter;
		ui.label_IN2->setText(tr("IN2:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(3);
		pMainFrm->nIOCard[3] = iCounter;
		ui.label_IN3->setText(tr("IN3:")+QString::number(iCounter));

		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(4);
		pMainFrm->nIOCard[4] = iCounter;
		ui.label_OUT0->setText(tr("OUT0:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(5);
		pMainFrm->nIOCard[5] = iCounter;
		ui.label_OUT1->setText(tr("OUT1:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(6);
		pMainFrm->nIOCard[6] = iCounter;
		ui.label_OUT2->setText(tr("OUT2:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(7);
		pMainFrm->nIOCard[7] = iCounter;
		ui.label_OUT3->setText(tr("OUT3:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(9)%0x100;
		pMainFrm->nIOCard[8] = iCounter;
		ui.label_OUT4->setText(tr("OUT4:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(9)/0x100;
		pMainFrm->nIOCard[9] = iCounter;
		ui.label_OUT5->setText(tr("OUT5:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(10)%0x100;
		pMainFrm->nIOCard[10] = iCounter;
		ui.label_OUT6->setText(tr("OUT6:")+QString::number(iCounter));
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(10)/0x100;
		pMainFrm->nIOCard[11] = iCounter;
		ui.label_OUT7->setText(tr("OUT7:")+QString::number(iCounter));
		//记录接口中的数据 by zl
		iCounter = pMainFrm->m_vIOCard[0]->ReadCounter(37);
		ui.label_23->setText(QString::fromLocal8Bit("接口卡补踢总数：")+QString::number(iCounter));		//接口卡补踢总数
		ui.label_15->setText(QString::fromLocal8Bit("综合过检总数：")+QString::number(pMainFrm->m_sRunningInfo.nGSoap_ErrorTypeCount[0])); //综合过检总数
		ui.label_24->setText(QString::fromLocal8Bit("综合过检踢废：")+QString::number(pMainFrm->m_sRunningInfo.nGSoap_ErrorCamCount[0]));  //综合过检踢废
		ui.label_28->setText(QString::fromLocal8Bit("补踢过检总数：")+QString::number(pMainFrm->m_sRunningInfo.nGSoap_ErrorTypeCount[2])); //补踢过检总数
		ui.label_29->setText(QString::fromLocal8Bit("补踢踢废总数：")+QString::number(pMainFrm->m_sRunningInfo.nGSoap_ErrorCamCount[2]));  //补踢踢废总数
		pMainFrm->nIOCard[12] = pMainFrm->m_sRunningInfo.nGSoap_ErrorTypeCount[0];
		pMainFrm->nIOCard[13] = pMainFrm->m_sRunningInfo.nGSoap_ErrorCamCount[0];
		pMainFrm->nIOCard[14] = pMainFrm->m_sRunningInfo.nGSoap_ErrorTypeCount[2];
		pMainFrm->nIOCard[15] = pMainFrm->m_sRunningInfo.nGSoap_ErrorCamCount[2];
		//
	}
}

void WidgetTest::slots_updateAlert(int i)
{
}
void WidgetTest::slots_ifCheckShowImage()
{
	ifshowImage=ui.comboBox->currentIndex();
}