/*
* The information in this file is
* Copyright(c) 2015, Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef DrizzleVideo_GUI_H
#define DrizzleVideo_GUI_H

#include <Qt/qdialog.h>
#include <Qt/qpushbutton.h>
#include <Qt/qmessagebox.h>
#include <Qt/qcombobox.h>
#include <Qt/qlabel.h>
#include <Qt/qlineedit.h>
#include <Qt/qlistwidget.h>


class DrizzleVideo_GUI : public QDialog
{
	Q_OBJECT
public:
	DrizzleVideo_GUI(QWidget* Parent, const char* Name);
	~DrizzleVideo_GUI();
public slots:
	void closeGUI();
	bool PerformDrizzle();
	void updateInfo();
private:
	QLabel *Video;
	QLabel *Size;

	QPushButton *Cancel;
	QPushButton *Apply;

	QComboBox *Rasterlist;

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