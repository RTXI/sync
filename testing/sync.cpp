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
#include <iostream>

extern "C" Plugin::Object *
createRTXIPlugin(void) {
  return new Synch();
}

static DefaultGUIModel::variable_t vars[] =
  {
    { "Models", "Models to synch 0-255", DefaultGUIModel::PARAMETER, }, };

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

Synch::Synch(void) : DefaultGUIModel("Synch", ::vars, ::num_vars), ModelIDString("0") {

  setWhatsThis(
       "<p><b>Synch:</b><br>This module allows you to synchronize other modules that are derived from the DefaultGUIModel class. It does not work with other custom user modules. Type in a comma-separated list (with or without spaces) of numbers that are the instance IDs of the modules you want to synchronize. Instance IDs are located in the left-hand corner of the module's toolbar.</p>");
  update(INIT);
  ListLen = 0;
	std::cout<<"flag 2"<<std::endl;

//  refresh();
	std::map<QString, param_t>::iterator aaaah = parameter.begin();
	std::cout<<(aaaah->second.str_value)->toStdString()<<std::endl;
	std::cout<<"flag 3"<<std::endl;
}

Synch::~Synch(void) {
}

void
Synch::execute(void) {
  return;
}
/*
void Synch::refresh(void) {
	std::cout<<"Started refresh()"<<std::endl;
		for (std::map<QString, param_t>::iterator i = parameter.begin(); i!= parameter.end(); ++i) {
			std::cout<<"Hello"<<std::endl;
			std::cout<<"Iterator = "<< i->second.edit << std::endl;
		if (i->second.type & (STATE | EVENT)) {
			std::cout<<"IT's a STATE or EVENT!!"<<std::endl;
			i->second.edit->setText(QString::number(getValue(i->second.type, i->second.index)));
			palette.setBrush(i->second.edit->foregroundRole(), Qt::darkGray);
			i->second.edit->setPalette(palette);
		} else if ((i->second.type & PARAMETER) && !i->second.edit->isModified() && i->second.edit->text() != *i->second.str_value) {
			std::cout<<"IT's a PARAMETER!!"<<std::endl;
			i->second.edit->setText(*i->second.str_value);
		} else if ((i->second.type & COMMENT) && !i->second.edit->isModified() && i->second.edit->text() != QString::fromStdString(getValueString(COMMENT, i->second.index))) {
			std::cout<<"IT's a COMMENT!!"<<std::endl;
			i->second.edit->setText(QString::fromStdString(getValueString(COMMENT, i->second.index)));
		}
	}
	pauseButton->setChecked(!getActive());
}
*/


void
Synch::update(DefaultGUIModel::update_flags_t flag) {
	std::cout<<"Update called"<<std::endl;
  switch (flag) {
    case INIT:
      setParameter("Models", ModelIDString);
      break;
    case MODIFY:
      ModelIDString = getParameter("Models");
      ModelIDString.replace(QChar(', '), QChar(','));
      ModelIDList = ModelIDString.split(",");
      ListLen = ModelIDList.count();
      Model_ID_List = new int[ListLen];
      i = 0;
      for (QStringList::Iterator it = ModelIDList.begin(); it != ModelIDList.end(); ++it) {
        Model_ID_List[i] = (*it).toInt();
//        printf("%d\n", Model_ID_List[i]);
        i++;
      }
      break;
    case UNPAUSE:
//      printf("SYNCH UNPAUSE\n");
      for (i = 0; i < ListLen; i++) {
        Model = dynamic_cast<DefaultGUIModel*> (Settings::Manager::getInstance()->getObject(Model_ID_List[i]));
        Model->setActive(true);
        Model->refresh();
      }
      break;
    case PAUSE:
//      printf("SYNCH PAUSE\n");
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

