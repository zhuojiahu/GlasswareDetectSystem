#include "widget_title.h"
#include <QCheckBox>
#include "glasswaredetectsystem.h"
extern GlasswareDetectSystem *pMainFrm;


WidgetTitle::WidgetTitle(QWidget *parent)
	: QWidget(parent)
{
	setObjectName("WidgetTitle");
	version_title = new QLabel();
	labelVersion = new QLabel();
	min_button = new PushButton();
	close_button = new PushButton();
	QFont ft;
	ft.setPointSize(12);
	version_title->setFont(ft);
	ft.setPointSize(8);
	version_title->setFont(ft);

	min_button->setPicName(QString(":/sysButton/min"));
	close_button->setPicName(QString(":/sysButton/close"));
	close_button->hide();
	connect(min_button, SIGNAL(clicked()), this, SIGNAL(showMin()));
	connect(close_button, SIGNAL(clicked()), this, SIGNAL(closeWidget()));

    QHBoxLayout *title_layout = new QHBoxLayout();
//	title_layout->addWidget(title_label,0,Qt::AlignVCenter);
    title_layout->addStretch();
    title_layout->addWidget(version_title,0,Qt::AlignVCenter);
    title_layout->addStretch();
    title_layout->addWidget(min_button, 0, Qt::AlignTop);
    title_layout->addWidget(close_button, 0, Qt::AlignTop);
	title_layout->setSpacing(0);
	title_layout->setContentsMargins(5, 0, 0, 0);
	version_title->setContentsMargins(15, 0, 0, 0);

	QStringList string_list;
	string_list<<":/toolWidget/bottle"<<":/toolWidget/management"\
		<<":/toolWidget/set"<<":/toolWidget/algset"<<":/toolWidget/start"<<":/toolWidget/exit"\
		<<":/toolWidget/GoDown"<<":/toolWidget/home"<<":/toolWidget/UpDown"<<":/toolWidget/InterFace";//
	QHBoxLayout *button_layout = new QHBoxLayout();//ˮƽ���ֹ�����

	QSignalMapper *signal_mapper = new QSignalMapper(this);//���������źŹ���
	for(int i=0; i<string_list.size(); i++)
	{ 
		ToolButton *tool_button = new ToolButton(string_list.at(i));
		tool_button->btnStyle = TITELSTYLE;
		button_list.append(tool_button);
		connect(tool_button, SIGNAL(clicked()), signal_mapper, SLOT(map()));
		signal_mapper->setMapping(tool_button, QString::number(i, 10));
		button_layout->addWidget(tool_button, 0, Qt::AlignBottom);
	}
	connect(signal_mapper, SIGNAL(mapped(QString)), this, SLOT(turnPage(QString)));
	
	QLabel *logo_label = new QLabel();
	QPixmap pixmap(":/toolWidget/daheng");
	logo_label->setPixmap(pixmap);
	logo_label->setFixedSize(pixmap.size());

	QVBoxLayout *layoutLogo = new QVBoxLayout();//ˮƽ���ֹ�����
	QSizePolicy sizePolicyLogo(QSizePolicy::Preferred, QSizePolicy::Expanding);
	sizePolicyLogo.setHorizontalStretch(0);
	sizePolicyLogo.setVerticalStretch(0);
	 logo_label->setSizePolicy(sizePolicyLogo);
	QSizePolicy sizePolicyVersion(QSizePolicy::Preferred, QSizePolicy::Minimum);
	sizePolicyVersion.setHorizontalStretch(0);
	sizePolicyVersion.setVerticalStretch(0);
	labelVersion->setSizePolicy(sizePolicyVersion);

	layoutLogo->addWidget(logo_label);
	labelVersion->setAlignment(Qt::AlignRight);

	button_layout->addStretch();
	button_layout->addLayout(layoutLogo);
	button_layout->setSpacing(8);
	button_layout->setContentsMargins(15, 0, 15, 0);

	QVBoxLayout *main_layout = new QVBoxLayout();
	main_layout->addLayout(title_layout);
	main_layout->addLayout(button_layout);
	main_layout->setSpacing(0);
	main_layout->setContentsMargins(0, 0, 0, 0);

	this->addToolName();

    setLayout(main_layout); 
    setFixedHeight(TITEL_HEIGHT);
    is_move = false;
}

void WidgetTitle::addToolName()
{
	// 	labelVersion->setText(pMainFrm->sVersion);
	version_title->setText(pMainFrm->m_sSystemInfo.m_strWindowTitle);
	min_button->setToolTip(tr("minimize"));
	close_button->setToolTip(tr("close"));
	button_list.at(0)->setText(tr("Image"));	
	button_list.at(1)->setText(tr("Management"));
	button_list.at(2)->setText(tr("Test"));
	button_list.at(3)->setText(tr("Algorithm"));
	button_list.at(4)->setText(tr("Start"));
	button_list.at(5)->setText(tr("Exit"));
	button_list.at(6)->setText(tr("UpDown"));
	button_list.at(7)->setText(tr("ClampDown"));
	button_list.at(8)->setText(tr("GoDown"));
	button_list.at(9)->setText(tr("InterFace"));
}

void WidgetTitle::turnPage(QString current_page)
{
	bool ok;  
	int current_index = current_page.toInt(&ok, 10);
	for(int i=0; i<button_list.count(); i++)
	{
		ToolButton *tool_button = button_list.at(i);
		if(i == current_index)
		{
			tool_button->setMousePress(true);
		}
		else
		{
			tool_button->setMousePress(false);
		}

	}
	emit turnPage(current_index);//����main_widget
}
