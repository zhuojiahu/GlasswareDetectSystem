#ifndef WIDGET_PLC_H
#define WIDGET_PLC_H
#include <QWidget>
#include <QTimer>
#include "ui_Widget_plc.h"
#include <QtNetwork/QUdpSocket> 
#include <windows.h>
#include <QDebug>
#include <QVector>
#include <QCheckBox>
class Widget_PLC : public QWidget
{
	Q_OBJECT

public:
	Widget_PLC(QWidget *parent,int);
	~Widget_PLC();
public slots:
	void slots_Pushbuttonsure();
	void slots_Pushbuttonsave();
	void slots_Pushbuttonread();
	void slots_readFromPLC();
	void slots_TimeOut();
	void slots_CrashTimeOut();
	void slots_clickBox(int);
public:
	void EnterPLC();
public:
	Ui::widget_plc ui;
public:
	template<typename T>
	void DataToByte(T& xx, QByteArray& st);
	template<typename T>
	void ByteToData(QByteArray& st, int nStart, int nEnd,T&);
	int SendMessage(int,QByteArray&,int,int,int);//读取写入数据，需要最后给读取数据长度
	void SendDataToPLCHead(int nAddr, QByteArray& st, int state,int id,int DataSize);
public:
	QUdpSocket* m_pSocket;
	QTimer* m_zTimer;
	QTimer* m_CrashTimer;
	int nErrorType;
	int nErrorCameraID;
	int nSystemType;
	QWidget * nAlertSet;
	QList<QCheckBox*> nlistCheckBox;
	int* nAlertDataList;
	int nPlcAlertNumber;
};

#endif // WIDGET_PLC_H
