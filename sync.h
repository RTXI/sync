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

#include <default_gui_model.h>
#include "/usr/local/lib/rtxi_includes/data_recorder.h"
#include <QtGui>

class Sync : public DefaultGUIModel {

	Q_OBJECT

	public:

		Sync(void);
		virtual ~Sync(void);
		virtual void execute(void);
		void customizeGUI(void);

	protected:

		virtual void update(DefaultGUIModel::update_flags_t);

	private:

		QTimer *syncTimer;
		QSpinBox *timerWheel;
		bool startDataRecorder;
		QString ModelIDString;
		QStringList ModelIDList;
		DefaultGUIModel * Model;
		int *Model_ID_List;
		int ListLen;
		int i;
		double systime;
    long long count;
    double dt;

		private slots:

			void toggleRecord(bool);
		void unpauseSync(void);
		void toggleTimer(bool);
};
