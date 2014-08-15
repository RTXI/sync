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
	{ "Models", "Models to synch 0-255", DefaultGUIModel::PARAMETER, }
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

Sync::Sync(void) : DefaultGUIModel("Sync", ::vars, ::num_vars), ModelIDString("0") {
	setWhatsThis("<p><b>Sync:</b><br>This module allows you to synchronize other modules that are derived from the DefaultGUIModel class. It does not work with other custom user modules. Type in a comma-separated list (with or without spaces) of numbers that are the instance IDs of the modules you want to synchronize. Instance IDs are located in the left-hand corner of the module's toolbar.</p>");
	update(INIT);
	DefaultGUIModel::createGUI(vars, num_vars);
	ListLen = 0;
	refresh();
}

Sync::~Sync(void) {}

void Sync::execute(void) {
	return;
}

void Sync::update(DefaultGUIModel::update_flags_t flag) {
	switch (flag) {
		case INIT:
			setParameter("Models", ModelIDString);
			break;
			
		case MODIFY:
			ModelIDString = getParameter("Models");
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
			for (i = 0; i < ListLen; i++) {
				Model = dynamic_cast<DefaultGUIModel*> (Settings::Manager::getInstance()->getObject(Model_ID_List[i]));
				Model->setActive(true);
				Model->refresh();
			}
			break;
		
		case PAUSE:
			for (i = 0; i < ListLen; i++) {
				Model = dynamic_cast<DefaultGUIModel*> (Settings::Manager::getInstance()->getObject(Model_ID_List[i]));
				Model->setActive(false);
				Model->refresh();
			}
			break;
		
		default:
			break;
	}
}
