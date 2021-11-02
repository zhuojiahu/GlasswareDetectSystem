#ifndef WIDGET_PLC_H
#define WIDGET_PLC_H
#include <QWidget>
#include <QTimer>
#include "ui_Widget_plc.h"
#include <QtNetwork/QUdpSocket> 
#include <windows.h>
#include <QDebug>
#include <QVector>
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
public:
	void EnterPLC();
	int GetImageNo(int nAddr,int CameraId,int& ImageNo);
signals:
	void signals_ResetCard();
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
	//HANDLE nHandle[40];
	//int nImageNum[40];
};

#endif // WIDGET_PLC_H
