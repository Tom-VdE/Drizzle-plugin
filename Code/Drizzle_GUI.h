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
#include <Qt/qlabel.h>
#include <Qt/qlineedit.h>


class Drizzle_GUI : public QDialog
{
	Q_OBJECT
public:
	Drizzle_GUI(QWidget* Parent, const char* Name);
	~Drizzle_GUI();
public slots:
	void closeGUI();
	bool PerformDrizzle();
	void updateInfo1();
	void updateInfo2();
private:
	QLabel *Image1;
	QLabel *Image2;
	QLabel *Size1;
	QLabel *Size2;
	QLabel *Geo1;
	QLabel *Geo2;

	QPushButton *Cancel;
	QPushButton *Apply;

	QComboBox *Rasterlist1;
	QComboBox *Rasterlist2;

	QLabel *x_out_text;
	QLabel *y_out_text;
	QLabel *dropsize_text;
	QLineEdit *x_out;
	QLineEdit *y_out;
	QLineEdit *dropsize;

	std::vector<std::string> RasterElements;

	void init();

};
#endif