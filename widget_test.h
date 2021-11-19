#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <QTimer>
#include "widget_errortype.h"
#include "widget_camera.h"
#include "ui_widget_test.h"
#include "camerasatuslabel.h"
#include "widget_plc.h"
#include "CIOCard.h"
enum StateEnum
{
	SENDDATA,
	GETSUCCESS,
	CLEAR,
	CONNECT,
	LOCKSCREEN,
	ALERT,
	SEVERS,//������
	IMAGE,
	NLEADING,
	NCLAMPING,
	NBACKING,
	MAININTERFACE,
	SYSTEMMODEADD,
	SYSTEMMODESELECT,
	SYSTEMMODEDELTE
};
enum UnitEnum
{
	LEADING, //ǰ��
	CLAMPING,//�г�
	BACKING,//���
};
struct MyStruct
{
	StateEnum nState;//����״̬��־λ
	UnitEnum nUnit;//���͵��豸��־λ
	int nCount;//ǰ��ͨ��in0�õ��Ĺ�������
	int nFail;//ǰ��ͨ��out3�õ����߷�����
	char nTemp[256];//ǰ�˵Ķ������
};
struct NingInfo
{
	int m_checkedNum;
	int m_passNum;
	int m_failureNum;
	int m_checkedNum2;
	NingInfo()
	{
		m_checkedNum = 0;
		m_passNum = 0;
		m_failureNum = 0;
		m_checkedNum2 = 0;
	}
};
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
	void slot_ConnectSever();
	void slot_readIoCard();
	void slot_openPlcSet();
	void slots_intoWidget();
	void slots_ChoseCamera();
	void slots_ChoseErrorType();
	void slots_OKSave();
	void slots_OKCameraSurveillance();
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
	QMutex nTestCounter;
	CIOCard* m_vIOCard;
	QTimer* nReadIOcard;
public:
	NingInfo nInfo;
	Widget_PLC * m_plc;
	//QConsole* nConsole;
	int iIOCardOffSet;
	int ifshowImage;
	int CameraOFF[CAMERA_MAX_COUNT];
};

#endif // TESTWIDGET_H