#ifndef QCONSOLE_H
#define QCONSOLE_H

#include <QMainWindow>
#include "ui_qconsole.h"
#include <QString>
#include "CIOCard.h"
#include <QMessageBox>
#include <QTimer>
#include "widget_plc.h"
//#include "widgetwarning.h"
#include <QFile>
#include <QCloseEvent>
enum StateEnum
{
	SENDDATA,
	GETSUCCESS,
	CLEAR,
	CONNECT,
	LOCKSCREEN,
	ALERT,
	SEVERS,//服务器
	IMAGE,
	NLEADING,
	NCLAMPING,
	NBACKING,
	MAININTERFACE
};
enum UnitEnum
{
	LEADING, //前端
	CLAMPING,//夹持
	BACKING,//后端
};
struct MyStruct
{
	StateEnum nState;//发送状态标志位
	UnitEnum nUnit;//发送的设备标志位
	int nCount;//前端通过in0得到的过检总数
	int nFail;//前端通过out3得到的踢废总数
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
class QConsole : public QMainWindow
{
	Q_OBJECT

public:
	QConsole(QWidget *parent = 0);
	~QConsole();
	void closeEvent(QCloseEvent *event);
	bool WritePrivateProfileQString(QString strSectionName, QString strKeyName, QString strValue, QString strFileName);
	void SendDataToSever();
public:
	Ui::QConsoleClass ui;
signals:
	void signal_HideWidget();
public slots:
	void slot_SaveCard();
	void slot_OpenCard();
	void slot_OpenPLC();
	void slot_ResetIoCard();
	void slot_readIoCard();
public:
	CIOCard* m_vIOCard;
	Widget_PLC * m_plc;
	//WidgetWarning* m_warning;
	QTimer* nReadIOcard;
public:
	NingInfo nInfo;
};

#endif // QCONSOLE_H
