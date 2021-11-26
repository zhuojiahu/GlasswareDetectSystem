#include "widget_plc.h"
#include <QLayout>
#include <QGroupBox>
#include <QSettings>
#include <QTextCodec>
#include <QMouseEvent>
#include <QPainter>
#include <QGridLayout>
#include <QSignalMapper>
#include "glasswaredetectsystem.h"
extern GlasswareDetectSystem *pMainFrm;

Widget_PLC::Widget_PLC(QWidget *parent,int SystemType)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowIcon(QIcon("./Resources/LOGO.png"));
	setWindowTitle(QString::fromLocal8Bit("PLC设置"));
	//setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);//去掉标题栏
	connect(ui.SureButton,SIGNAL(clicked()),this,SLOT(slots_Pushbuttonsure()));
	connect(ui.pushButton_save,SIGNAL(clicked()),this,SLOT(slots_Pushbuttonsave()));
	connect(ui.pushButton_read,SIGNAL(clicked()),this,SLOT(slots_Pushbuttonread()));
	
	m_pSocket = new QUdpSocket();
	m_pSocket->connectToHost("192.168.250.1", 9600);
	if (m_pSocket->state() == QAbstractSocket::ConnectedState || m_pSocket->waitForConnected(2000))
	{
		connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slots_readFromPLC()));
	}
	QIntValidator* IntValidator = new QIntValidator;
	IntValidator->setRange(1, 60);
	//ui.lineEdit_2->setValidator(IntValidator);
	ui.lineEdit_3->setValidator(IntValidator);
	ui.lineEdit_4->setValidator(IntValidator);
	ui.lineEdit_5->setValidator(IntValidator);
	ui.lineEdit_6->setValidator(IntValidator);
	IntValidator->setRange(1,450);
	ui.lineEdit_1->setValidator(IntValidator);

	if(SystemType==2)
	{
		m_zTimer = new QTimer(this);
		connect(m_zTimer,SIGNAL(timeout()),this,SLOT(slots_TimeOut()));
		m_zTimer->start(2000);
	}
	
	m_CrashTimer = new QTimer(this);
	connect(m_CrashTimer,SIGNAL(timeout()),this,SLOT(slots_CrashTimeOut()));
	m_CrashTimer->start(10000);

	//获取PLC报警信息
	nErrorType = 0;
	nErrorCameraID = 0;
	nSystemType = SystemType;
	
	QButtonGroup* test4=new QButtonGroup(this);
	test4->addButton(ui.radioButton_9);
	test4->addButton(ui.radioButton_10);
	/////////////////////
	if(SystemType==2)
	{
		nPlcAlertNumber = pMainFrm->m_vstrPLCInfoType.count();
		nAlertDataList = new int[nPlcAlertNumber*3];
		memset(nAlertDataList,0,nPlcAlertNumber*3*sizeof(int));
		nAlertSet = new QWidget(this);
		QPalette pal(nAlertSet->palette());
		pal.setColor(QPalette::Background,QColor(90,90,90,120));
		nAlertSet->setAutoFillBackground(true);
		nAlertSet->setPalette(pal);

		QGridLayout *gridlayout = new QGridLayout(nAlertSet);
		QSignalMapper* signalmapper = new QSignalMapper(this);//工具栏的信号管理
		QLabel * labelinfo1= new QLabel(this);
		labelinfo1->setText(QString::fromLocal8Bit("报警状态"));//勾选表示要报警
		gridlayout->addWidget(labelinfo1,0,1);

		QLabel * labelinfo2= new QLabel(this);
		labelinfo2->setText(QString::fromLocal8Bit("输送线状态"));//勾选表示要停止输送线
		gridlayout->addWidget(labelinfo2,0,2);

		QLabel * labelinfo3= new QLabel(this);
		labelinfo3->setText(QString::fromLocal8Bit("理瓶器状态"));//勾选表示要停止理瓶器
		gridlayout->addWidget(labelinfo3,0,3);

		for (int i = 1;i <= nPlcAlertNumber;i++)
		{
			QLabel * label = new QLabel(this);
			label->setText(pMainFrm->m_vstrPLCInfoType[i-1]);
			QCheckBox *checkBox = new QCheckBox(this);
			connect(checkBox, SIGNAL(stateChanged(int)), signalmapper, SLOT(map()));
			signalmapper->setMapping(checkBox, i);
			nlistCheckBox.append(checkBox);

			QCheckBox *checkBox1 = new QCheckBox(this);
			connect(checkBox1, SIGNAL(stateChanged(int)), signalmapper, SLOT(map()));
			signalmapper->setMapping(checkBox1, i+nPlcAlertNumber);
			nlistCheckBox.append(checkBox1);

			QCheckBox *checkBox2 = new QCheckBox(this);
			connect(checkBox2, SIGNAL(stateChanged(int)), signalmapper, SLOT(map()));
			signalmapper->setMapping(checkBox2, i+2*nPlcAlertNumber);
			nlistCheckBox.append(checkBox2);

			if(pMainFrm->m_vstrPLCInfoType[i-1] != "")
			{
				gridlayout->addWidget(label,i,0);
				gridlayout->addWidget(checkBox,i,1);
				gridlayout->addWidget(checkBox1,i,2);
				gridlayout->addWidget(checkBox2,i,3);
			}else{
				label->setVisible(false);
				checkBox->setVisible(false);
				checkBox1->setVisible(false);
				checkBox2->setVisible(false);
			}
		}
		connect(signalmapper, SIGNAL(mapped(int)), this, SLOT(slots_clickBox(int)));
		QVBoxLayout *mainLayout = new QVBoxLayout();
		mainLayout->addLayout(gridlayout);
		mainLayout->setSpacing(6);
		mainLayout->setContentsMargins(0,0,0,0);

		nAlertSet->setLayout(mainLayout);
		ui.scrollArea->setWidget(nAlertSet);
	}
	//80000200010000020005 0101B200 D4000002
}

Widget_PLC::~Widget_PLC()
{
	delete m_pSocket;
}
void Widget_PLC::slots_clickBox(int mTemp)
{
	if(nAlertDataList[mTemp-1]==1)
	{
		nAlertDataList[mTemp-1]=0;
	}else{
		nAlertDataList[mTemp-1]=1;
	}
}
void Widget_PLC::slots_Pushbuttonread()
{
	QByteArray st;
	SendMessage(20,st,1,1,40);
}
void Widget_PLC::EnterPLC()
{
	QByteArray st;
	SendMessage(92,st,1,2,120);//暂时获取界面显示的所有数据2+8*5+8*8+4
}
void Widget_PLC::slots_CrashTimeOut()
{
	QByteArray st;
	int zTest = 1;
	if(nErrorCameraID)
	{
		int test = 1;
		DataToByte(test,st);
	}else{
		int test = 0;
		DataToByte(test,st);
	}
	DataToByte(zTest,st);
	if(nSystemType == 1)
	{
		SendMessage(200,st,2,1,8);
	}else if(nSystemType == 2){
		SendMessage(204,st,2,1,8);
	}else if(nSystemType == 3){
		SendMessage(208,st,2,1,8);
	}
}
void Widget_PLC::slots_TimeOut()
{
	//获取PLC的报警信息
	QByteArray st;
	SendMessage(0,st,1,1,4);//读取报警数据
}
void Widget_PLC::SendDataToPLCHead(int address, QByteArray& st, int state,int id,int DataSize) //参数1为相机ID号，参数2为组装后的数据，参数3为读写状态,参数4为通道ID(可以为任意整数),参数5为数据大小
{
	QByteArray v_szTmp;
	v_szTmp.append(QChar(0x80).toLatin1());//ICF  display frame information
	v_szTmp.append(QChar(0x00).toLatin1());//RSV  reserved by system
	v_szTmp.append(QChar(0x02).toLatin1());//GCT  permissible number of gateways
	v_szTmp.append(QChar(0x00).toLatin1());//DNA  destination network address
	v_szTmp.append(QChar(0x01).toLatin1());//DA1  destination node address
	v_szTmp.append(QChar(0x00).toLatin1());//DA2  destination unit address
	v_szTmp.append(QChar(0x00).toLatin1());//SNA  source network address
	v_szTmp.append(QChar(0x02).toLatin1());//SA1  source node address
	v_szTmp.append(QChar(0x00).toLatin1());//SA2  source unit address
	v_szTmp.append(id);//SID  service id   预先计划赋值sendId
	v_szTmp.append(QChar(0x01).toLatin1());//MRC  main request code
	v_szTmp.append(state);//SRC  sub request code
	v_szTmp.append(QChar(0xB2).toLatin1());//H区都是B2,D区是82
	v_szTmp.append(address/256); //内存地址
	v_szTmp.append(address%256); //内存地址
	v_szTmp.append(QChar(0x00).toLatin1());
	v_szTmp.append(DataSize/2/256);//数据长度
	v_szTmp.append(DataSize/2%256);//数据长度
	v_szTmp.append(st);
	st = v_szTmp;
}

int Widget_PLC::SendMessage(int address,QByteArray& send,int state,int id,int DataSize) //异步发送数据改变PLC参数
{
	SendDataToPLCHead(address,send,state,id,DataSize);
	//qDebug()<<send.toHex();
	if (m_pSocket->state() == QAbstractSocket::ConnectedState)
	{
		if (NULL != m_pSocket)
		{
			m_pSocket->write(send);
		}
	}
	return 0;
}
void Widget_PLC::slots_readFromPLC()
{
	QByteArray v_receive = m_pSocket->readAll();
	if (v_receive.size() == 134)//120+14
	{
		double v_douTemp = 0;
		int v_Itmp = 0;
		int v_bit = 14;
		int j=0;
		for (;v_bit<26;v_bit+=2)
		{
			WORD v_Itmps=0;
			ByteToData(v_receive,v_bit,v_bit+1,v_Itmps);
			for(int i=0;i<16;i++)
			{
				if(v_Itmps >> i & 0x01)
				{
					//pMainFrm->Logfile.write(QString("%1---%2").arg(v_Itmps).arg(j),CheckLog);
					nAlertDataList[j]=1;
					nlistCheckBox[j]->setChecked(true);
				}
				j++;
			}
		}

		ByteToData(v_receive,v_bit,v_bit+3,v_Itmp); //H98
		ui.lineEdit_21->setText(QString::number(v_Itmp));
		v_bit+=4;
		ByteToData(v_receive,v_bit,v_bit+3,v_Itmp);
		ui.lineEdit_1->setText(QString::number(v_Itmp));
		v_bit+=4;
		ByteToData(v_receive,v_bit,v_bit+3,v_Itmp);
		if(v_Itmp == 0)
		{
			ui.radioButton_1->setChecked(true);
			ui.radioButton_2->setChecked(false);
		}else{
			ui.radioButton_1->setChecked(false);
			ui.radioButton_2->setChecked(true);
		}
		v_bit+=4;
		ByteToData(v_receive,v_bit,v_bit+3,v_Itmp);
		ui.lineEdit_2->setText(QString::number(v_Itmp*0.01,'f',2));
		v_bit+=4;
		ByteToData(v_receive,v_bit,v_bit+3,v_Itmp);
		ui.lineEdit_3->setText(QString::number(v_Itmp/100));
		v_bit+=4;
		ByteToData(v_receive,v_bit,v_bit+3,v_Itmp);
		ui.lineEdit_4->setText(QString::number(v_Itmp/100));
		v_bit+=4;
		ByteToData(v_receive,v_bit,v_bit+3,v_Itmp);
		ui.lineEdit_5->setText(QString::number(v_Itmp/100));
		v_bit+=4;
		ByteToData(v_receive,v_bit,v_bit+3,v_Itmp);
		ui.lineEdit_6->setText(QString::number(v_Itmp/100));
		v_bit+=4;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_7->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_9->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_10->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_11->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_12->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_14->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_15->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_17->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_19->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+3,v_Itmp);
		
		if(v_Itmp == 0)
		{
			ui.radioButton_9->setChecked(false);
			ui.radioButton_10->setChecked(true);
		}else{
			ui.radioButton_9->setChecked(true);
			ui.radioButton_10->setChecked(false);
		}
	}else if(v_receive.size() == 18)
	{
		WORD v_Itmp=0;
		int j=0;
		int m_byte=14;
		bool Asert = true;
		for (;m_byte<18;m_byte+=2)
		{
			ByteToData(v_receive,m_byte,m_byte+1,v_Itmp);
			for(int i=0;i<16;i++)
			{
				if(v_Itmp >> i & 0x01)
				{
					nErrorType = j;
					Asert = false;
				}
				j++;
			}
		}
		if(Asert)
		{
			nErrorType = -1;
		}
	}else if(v_receive.size() == 54)//14+40
	{
		double v_douTemp;
		int v_bit=14;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_8->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_13->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_16->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_18->setText(QString::number(v_douTemp,'f',2));
		v_bit+=8;
		ByteToData(v_receive,v_bit,v_bit+7,v_douTemp);
		ui.lineEdit_20->setText(QString::number(v_douTemp,'f',2));
	}
}

void Widget_PLC::slots_Pushbuttonsure()
{
	QByteArray st;
	WORD TempData = 3;
	DataToByte(TempData,st);
	SendMessage(90,st,2,1,2);//写入指令，命令下发完毕后再写一次
}

void Widget_PLC::slots_Pushbuttonsave()
{
	QByteArray st;
	WORD test = 1;
	WORD nData[6];
	memset(nData,0,sizeof(WORD)*6);
	int serial = 0;
	for(int i=0;i<nPlcAlertNumber*3;i++)
	{
		if(i<nPlcAlertNumber)
		{
			if(nAlertDataList[i])
			{
				if(i<16)
				{
					nData[0] += test<<i;
				}else{
					nData[1] += test<<(i-16);
				}
			}
		}else if(i>=nPlcAlertNumber&&i<nPlcAlertNumber*2)
		{
			if(nAlertDataList[i])
			{
				if(i<48)
				{
					nData[2] += test<<(i-32);
				}else{
					nData[3] += test<<(i-48);
				}
			}
		}else{
			if(nAlertDataList[i])
			{
				if(i<80)
				{
					nData[4] += test<<(i-64);
				}else{
					nData[5] += test<<(i-80);
				}
			}
		}
	}
	for(int i=0;i<6;i++)
	{
		DataToByte(nData[i],st);
	}

	int TempData = 0;
	TempData = ui.lineEdit_21->text().toInt();
	DataToByte(TempData,st);
	TempData = ui.lineEdit_1->text().toInt();
	DataToByte(TempData,st);
	if(ui.radioButton_1->isChecked())
	{
		TempData = 0;
	}else{
		TempData = 1;
	}
	DataToByte(TempData,st);
	TempData = ui.lineEdit_2->text().toDouble()*100;
	DataToByte(TempData,st);
	TempData = ui.lineEdit_3->text().toInt()*100;
	DataToByte(TempData,st);
	TempData = ui.lineEdit_4->text().toInt()*100;
	DataToByte(TempData,st);
	TempData = ui.lineEdit_5->text().toInt()*100;
	DataToByte(TempData,st);
	TempData = ui.lineEdit_6->text().toInt()*100;
	DataToByte(TempData,st);
	double TempSpeed = ui.lineEdit_7->text().toDouble();
	DataToByte(TempSpeed,st);

	TempSpeed = ui.lineEdit_9->text().toDouble();
	DataToByte(TempSpeed,st);
	TempSpeed = ui.lineEdit_10->text().toDouble();
	DataToByte(TempSpeed,st);
	TempSpeed = ui.lineEdit_11->text().toDouble();
	DataToByte(TempSpeed,st);
	TempSpeed = ui.lineEdit_12->text().toDouble();
	DataToByte(TempSpeed,st);
	TempSpeed = ui.lineEdit_14->text().toDouble();
	DataToByte(TempSpeed,st);
	TempSpeed = ui.lineEdit_15->text().toDouble();
	DataToByte(TempSpeed,st);
	TempSpeed = ui.lineEdit_17->text().toDouble();
	DataToByte(TempSpeed,st);
	TempSpeed = ui.lineEdit_19->text().toDouble();
	DataToByte(TempSpeed,st);
	if(ui.radioButton_9->isChecked())
	{
		TempData = 1;
	}else{
		TempData = 0;
	}
	DataToByte(TempData,st);
	SendMessage(92,st,2,1,120);
}
template<typename T>
void Widget_PLC::DataToByte(T& xx, QByteArray& st)
{
	//st.clear();
	char nChar = 0;
	char* f_pshort = reinterpret_cast<char*>(&xx);
	for (int i = 0; i < sizeof(T); ++i)
	{
		if (i % 2 == 0)
		{
			nChar = (char)(*(f_pshort + i + 1));
		}
		else
		{
			nChar = (char)(*(f_pshort + i - 1));
		}
		st.append(nChar);
	}
}
template<typename T>
void Widget_PLC::ByteToData(QByteArray& st, int nStart, int nEnd,T& xt)
{
	xt = 0;
	char* f_pshort = reinterpret_cast<char*>(&xt);
	for (int i = nStart, j = 0; i <= nEnd; ++i, ++j)
	{
		if (i % 2 == 0)
		{
			*(f_pshort + j) = st[i + 1];
		}
		else
		{
			*(f_pshort + j) = st[i - 1];
		}
	}
}