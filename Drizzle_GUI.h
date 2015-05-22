/*
* The information in this file is
* Copyright(c) 2015, Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef Drizzle_GUI_H
#define Drizzle_GUI_H

#include <Qt/qdialog.h>
#include <Qt/qpushbutton.h>
#include <Qt/qmessagebox.h>
#include <Qt/qcombobox.h>


class Drizzle_GUI : public QDialog
{
	//Q_OBJECT
public:
	Drizzle_GUI(QWidget* Parent, const char* Name);
	~Drizzle_GUI();
public slots:
	void closeGUI();
	void PerformDrizzle();
private:
	QPushButton *Cancel;
	QPushButton *Apply;
	QComboBox *Rasterlist1;
	QComboBox *Rasterlist2;

	std::vector<std::string> RasterElements;

	void init();

};
#endif