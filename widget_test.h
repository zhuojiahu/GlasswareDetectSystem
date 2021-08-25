#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <QTimer>
#include "widget_errortype.h"
#include "widget_camera.h"
#include "ui_widget_test.h"
#include "camerasatuslabel.h"
#include "qconsole.h"

class WidgetTest : public QWidget
{
	Q_OBJECT

public:

	WidgetTest(QWidget *parent = 0);
	~WidgetTest();
	Ui::WidgetTest ui;

	Widget_ErrorType *widget_ErrorType;
	Widget_Camera *widget_Camera;
public:
	void init();
	void initEquipAlarmTablewidget();
	void initWidgetName();
	bool leaveWidget();
	void updateIOCardParam();
	void getIOCardParam();
//	void InitIOCardInfo();
	void initInformation();
signals:
	void signals_changeStatisMode(int iStatisMode,int iMinute,int iStatisNumber);
	void signals_ReadDistance1();
	void signals_sendAlarm(int warningType, QString warningInfo);
	void signals_ShowWarning(int , QString );
	void signal_UsualSend(int);
public slots:
	void slot_openPlcSet();
	void slots_intoWidget();
	void slots_ChoseCamera();
	void slots_ChoseErrorType();
	void slots_OKSave();
	void slots_OKCameraSurveillance();
	void slots_updateAlert(int);
	void slots_Cancel();
	void slots_KickModeChanged(int iMode);

	void slots_SaveModeChanged(int index);
	void slots_readDelay();
	void slots_setToCard();
	void slots_setToFile();
	void slots_advance1();

	void slots_choseAllCamera();
	void slots_choseNoneCamera();
	void slots_choseAllErrorType();
	void slots_choseNoneErrorType();

	void slots_updateIOcardCounter();
	void slots_ifCheckShowImage();
	void slots_CameraOffAreet();
	void SetCameraMonitorStatus();
	void slots_EquipAlarmCheckBox(bool);
	void slots_EquipAlarmInfoShowbtn(bool);
	void slots_EquipAlarmSave();
	void slots_EquipAlarmClear();
	void slots_SetEquipAlarmSatus(int,bool);
	
private:
	int iSaveMode;
	int iLastSaveMode;
	int iKickMode;
	int iLastKickMode;
	int iSaveImgCount;
	int iLastSaveImgCount;
	
	int iChoseCamera;
	int iChoseErrorType;
	
	int m_nDelay1;
	int m_nDelay2;
	int m_nDelay3;
	int m_nDelay4;
	int m_nDelay5;
	int m_nDelay6;
	int m_nKickDelay;
	int m_nKickWidth;
	int m_nSampleDelay;
	int m_nSampleWidth;

	QTimer* timerUpdateIOCardCounter;
	QTimer* CameraOffAreet;

	QList<CameraStatusLabel*> m_EquipAlarmStatusList;

public:
	QConsole* nConsole;
	int iIOCardOffSet;
	int ifshowImage;
	int CameraOFF[CAMERA_MAX_COUNT];
};

#endif // TESTWIDGET_H