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

// Risa Lin
// Wed 13 Jul 2011 18:58:18 PM EDT
// Now accepts ", " or "," as delimiters between integers.


#include <sync.h>
#include <QtGui>

extern "C" Plugin::Object * createRTXIPlugin(void) {
	return new Sync();
}

static DefaultGUIModel::variable_t vars[] = {
	{ "Model IDs", "Models to synch 0-255", DefaultGUIModel::PARAMETER, },
	{ "Time (s)", "Elapsed Time (s)", DefaultGUIModel::STATE, }, 
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

Sync::Sync(void) : DefaultGUIModel("Sync", ::vars, ::num_vars), ModelIDString("0") {
	setWhatsThis("<p><b>Sync:</b><br>This module allows you to synchronize other modules that are derived from the DefaultGUIModel class. It does not work with other custom user modules. Type in a comma-separated list (with or without spaces) of numbers that are the instance IDs of the modules you want to synchronize. Instance IDs are located in the left-hand corner of the module's toolbar.</p>");
	DefaultGUIModel::createGUI(vars, num_vars);
	customizeGUI();
	update(INIT);
	ListLen = 0;
	syncTimer = new QTimer;
	QObject::connect(syncTimer, SIGNAL(timeout(void)), this, SLOT(unpauseSync(void)));
	refresh();
	QTimer::singleShot(0, this, SLOT(resizeMe()));
}

Sync::~Sync(void) {}

void Sync::execute(void) {
	systime = count * dt; // time in seconds
	count++;
}

void Sync::update(DefaultGUIModel::update_flags_t flag) {
	switch (flag) {
		case INIT:
			startDataRecorder = true;
			systime = 0;
			count = 0;
			dt = RT::System::getInstance()->getPeriod() * 1e-9; // time in seconds
			setParameter("Model IDs", ModelIDString);
			setState("Time (s)", systime);
			break;

		case MODIFY:
			ModelIDString = getParameter("Model IDs");
			ModelIDString.replace(QChar(', '), QChar(','));
			ModelIDList = ModelIDString.split(",");
			ListLen = ModelIDList.size();
			Model_ID_List = new int[ListLen];
			i = 0;
			for (QStringList::Iterator it = ModelIDList.begin(); it != ModelIDList.end(); ++it) {
				Model_ID_List[i] = (*it).toInt();
				i++;
			}
			break;

		case UNPAUSE:
			if(startDataRecorder)
			{
				if(DataRecorder::Plugin::getInstance()->recStatus)
				{
					DataRecorder::startRecording();
				}
				else
				{
					QMessageBox::critical(
							this, "DataRecorder not ready.",
							"Please make sure a data file is specified and at least one channel is in the list.",
							QMessageBox::Ok, QMessageBox::NoButton);
					pauseButton->setChecked(true);
					break;
				}
			}

			for (i = 0; i < ListLen; i++) {
				Model = dynamic_cast<DefaultGUIModel*> (Settings::Manager::getInstance()->getObject(Model_ID_List[i]));
				Model->setActive(true);
				Model->pauseButton->setEnabled(false);
				Model->refresh();
			}
			syncTimer->start(timerWheel->value()*1e3);
			systime = 0;
			count = 0;
			break;

		case PERIOD:
			dt = RT::System::getInstance()->getPeriod() * 1e-9; // time in seconds
			break;

		case PAUSE:
			if(startDataRecorder && DataRecorder::Plugin::getInstance()->recStatus)
				DataRecorder::stopRecording();

			for (i = 0; i < ListLen; i++) {
				Model = dynamic_cast<DefaultGUIModel*> (Settings::Manager::getInstance()->getObject(Model_ID_List[i]));
				Model->setActive(false);
				Model->pauseButton->setEnabled(true);
				Model->refresh();
			}
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
	if(timerStatus)
		timerWheel->setEnabled(true);
	else
	{
		timerWheel->setEnabled(false);
		syncTimer->stop();
	}
}

void Sync::unpauseSync(void)
{
	pauseButton->setChecked(true);
}

void Sync::customizeGUI(void)
{
	QGridLayout *customlayout = DefaultGUIModel::getLayout();
	QCheckBox *checkBox = new QCheckBox("&Sync Data Recorder");
	checkBox->setEnabled(true);
	checkBox->setChecked(true);
	QObject::connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(toggleRecord(bool)));
	QCheckBox *timerCheckBox = new QCheckBox("&Sync Timer");
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
	timerLayout->addWidget(checkBox, 0, 0);
	timerLayout->addWidget(timerCheckBox, 1, 0);
	timerLayout->addWidget(timerWheel, 1, 1);
	timerBox->setLayout(timerLayout);
	customlayout->addWidget(timerBox, 2, 0);
	setLayout(customlayout);
}
