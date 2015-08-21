/********************************************//*
* 
* @file: Drizzle_GUI.h
*
* The information in this file is
* Copyright(c) 2015 Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
***********************************************/

#ifndef Drizzle_GUI_H
#define Drizzle_GUI_H

#include <Qt/qdialog.h>
#include <Qt/qpushbutton.h>
#include <Qt/qmessagebox.h>
#include <Qt/qcombobox.h>
#include <Qt/qlabel.h>
#include <Qt/qlineedit.h>
#include <Qt/qlistwidget.h>

/**
*
* The class of the Drizzle plugin which handles image input.
*/
class Drizzle_GUI : public QDialog
{
	Q_OBJECT
public:
	/**
	* Constructor for image input.
	* Handles GUI, input, calculations and output.
	*
	* @param Parent QWidget as parent for construction of QDialog.
	*/
	Drizzle_GUI(QWidget* Parent);

	/**
	* Destructor for image input.
	* 
	*/
	~Drizzle_GUI();

public slots:
	/**
	* Slot for closing the image GUI, connected to 'Cancel' button.
	*/
	void closeGUI();

	/**
	* Slot to perform the Drizzling. Performs necessary preliminary
	* calculations for Drizzle function and calls it.
	* Connected to 'Drizzle' button.
	*
	* @return True when Drizzling is successfull, false otherwise.
	*/
	bool PerformDrizzle();

	/**
	* Slot to update the information of the first selected input image
	* displayed on the image GUI.
	*/
	void updateInfo1();

	/**
	* Slot to update the information of the other selected input images
	* displayed on the image GUI.
	*/
	void updateInfo2();

private:
	/**
	* QLabel for base image.
	*/
	QLabel *Image1;

	/**
	* QLabel for other images.
	*/
	QLabel *Image2;

	/**
	* QLabel for size of base image.
	*/
	QLabel *Size1;

	/**
	* QLabel for size of other images.
	*/
	QLabel *Size2;

	/**
	* QLabel for geographical coordinates of base image.
	*/
	QLabel *Geo1;

	/**
	* QLabel for geographical coordinates of other images.
	*/
	QLabel *Geo2;

	/**
	* QPushButton to cancel plugin.
	* Connects to closeGUI() SLOT.
	*/
	QPushButton *Cancel;

	/**
	* QPushButton to apply.
	* Connects to performDrizzle() SLOT.
	*/
	QPushButton *Apply;

	/**
	* QComboBox to select base image.
	*/
	QComboBox *Rasterlist1;

	/**
	* QListWidget containing open RasterElements with checkboxes
	* to select multiple input images.
	*/
	QListWidget *Rasterlist2;

	/**
	* QLabel for width of output image.
	*/
	QLabel *x_out_text;

	/**
	* QLabel for height of output image.
	*/
	QLabel *y_out_text;

	/**
	* QLabel for dropsize.
	*/
	QLabel *dropsize_text;

	/**
	* QLineEdit to input width of output image.
	*/
	QLineEdit *x_out;

	/**
	* QLineEdit to input height of output image.
	*/
	QLineEdit *y_out;

	/**
	* QLineEdit to input dropsize.
	*/
	QLineEdit *dropsize;

	/**
	* vector containing all open RasterElements.
	*/
	std::vector<std::string> RasterElements;
	
	/**
	* Initialisations needed for image GUI:
	* Connects buttons to SLOTS.
	* Get RasterElements and construct vector RasterElements.
	* Set size of GUI
	*/
	void init();

};
#endif