/*
			Copyright (C) 2011 Georgia Institute of Technology

			This program is free software: you can redistribute it and/or modify
			it under the terms of the GNU General Public License as published by
			the Free Software Foundation, either version 3 of the License, or
			(at your option) any later version.

			This program is distributed in the hope that it will be useful,
			but WITHOUT ANY WARRANTY; without even the implied warranty of
			MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
			GNU General Public License for more details.

			You should have received a copy of the GNU General Public License
			along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <sync.h>

extern "C" Plugin::Object * createRTXIPlugin(void) {
	return new Sync();
}

static DefaultGUIModel::variable_t vars[] = {
	{ "Model IDs", "Models to synch 0-255", DefaultGUIModel::PARAMETER, },
	{ "Time (s)", "Elapsed Time (s)", DefaultGUIModel::STATE, }, 
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

Sync::Sync(void) : DefaultGUIModel("Sync", ::vars, ::num_vars), ModelIDString("") {
	setWhatsThis("<p><b>Sync:</b><br>This module allows you to synchronize other modules that are derived from the DefaultGUIModel class. It does not work with other custom user modules. Type in a comma-separated list (with or without spaces) of numbers that are the instance IDs of the modules you want to synchronize. Instance IDs are located in the left-hand corner of the module's toolbar.</p>");
	DefaultGUIModel::createGUI(vars, num_vars);
	customizeGUI();
	update(INIT);
	ListLen = 0;
	refresh();
	QTimer::singleShot(0, this, SLOT(resizeMe()));
}

Sync::~Sync(void) {}

void Sync::execute(void) {
	systime = count * dt; // time in seconds
	if(timerCheckBox->isChecked())
		if(systime >= timerValue)
			pauseButton->setChecked(true);
	count++;
}

void Sync::update(DefaultGUIModel::update_flags_t flag) {
	switch (flag) {
		case INIT:
			startDataRecorder = false;
			systime = 0;
			count = 0;
			timerValue = 0.0;
			dt = RT::System::getInstance()->getPeriod() * 1e-9; // time in seconds
			setParameter("Model IDs", ModelIDString);
			setState("Time (s)", systime);
			break;

		case MODIFY:
			ModelIDString = getParameter("Model IDs");
			if(!ModelIDString.isEmpty())
			{
				ModelIDString.replace(QChar(', '), QChar(','));
				ModelIDList = ModelIDString.split(",");
				ListLen = ModelIDList.size();
				Model_ID_List = new int[ListLen];
				i = 0;
				for (QStringList::Iterator it = ModelIDList.begin(); it != ModelIDList.end(); ++it) {
					Model_ID_List[i] = (*it).toInt();
					i++;
				}
			}
			if(timerCheckBox->isChecked())
				timerValue = timerWheel->value();
			systime = 0;
			count = 0;
			break;

		case PERIOD:
			dt = RT::System::getInstance()->getPeriod() * 1e-9; // time in seconds
			break;

		case PAUSE:
			if(startDataRecorder)
				DataRecorder::Plugin::getInstance()->panelList.front()->stopRecordClicked();

			for (i = 0; i < ListLen; i++) {
				Model = dynamic_cast<DefaultGUIModel*> (Settings::Manager::getInstance()->getObject(Model_ID_List[i]));
				if(Model->getActive())
					Model->setActive(false);
				Model->pauseButton->setEnabled(true);
				Model->refresh();
			}
			timerCheckBox->setEnabled(true);
			dataCheckBox->setEnabled(true);
			if(timerCheckBox->isChecked())
				timerWheel->setEnabled(true);
			count = 0;
			break;

		case UNPAUSE:
			systime = 0;
			if(startDataRecorder)
				DataRecorder::Plugin::getInstance()->panelList.front()->startRecordClicked();

			if(!ModelIDString.isEmpty())
			{
				for (i = 0; i < ListLen; i++) {
					Model = dynamic_cast<DefaultGUIModel*> (Settings::Manager::getInstance()->getObject(Model_ID_List[i]));
					if(!Model->getActive())
						Model->setActive(true);
					Model->pauseButton->setEnabled(false);
					Model->refresh();
				}
			}
			timerCheckBox->setEnabled(false);
			dataCheckBox->setEnabled(false);
			timerWheel->setEnabled(false);
			break;

		default:
			break;
	}
}

void Sync::toggleRecord(bool)
{
	startDataRecorder = startDataRecorder == false ? true : false;
}

void Sync::toggleTimer(bool timerStatus)
{
	timerWheel->setEnabled(timerStatus);
}

void Sync::customizeGUI(void)
{
	QGridLayout *customlayout = DefaultGUIModel::getLayout();
	dataCheckBox = new QCheckBox("&Sync Data Recorder");
	dataCheckBox->setEnabled(true);
	dataCheckBox->setChecked(false);
	QObject::connect(dataCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleRecord(bool)));
	timerCheckBox = new QCheckBox("&Sync Timer (s)");
	QGroupBox *timerBox = new QGroupBox;
	QGridLayout *timerLayout = new QGridLayout;
	timerCheckBox->setWhatsThis("Timer for turning sync off after a user-specified time period");
	timerCheckBox->setEnabled(true);
	timerCheckBox->setChecked(false);
	QObject::connect(timerCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleTimer(bool)));
	timerWheel = new QSpinBox(this);
	timerWheel->setEnabled(false);
	timerWheel->setValue(10);
	timerWheel->setFixedWidth(75);
	timerWheel->setMaximum(9999);
	timerWheel->setWhatsThis("Timer value in seconds");
	timerLayout->addWidget(dataCheckBox, 0, 0);
	timerLayout->addWidget(timerCheckBox, 1, 0);
	timerLayout->addWidget(timerWheel, 1, 1);
	timerBox->setLayout(timerLayout);
	customlayout->addWidget(timerBox, 2, 0);
	setLayout(customlayout);
}
