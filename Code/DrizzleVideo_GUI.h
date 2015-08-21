/********************************************//*
* 
* @file: DrizzleVideo_GUI.h
*
* The information in this file is
* Copyright(c) 2015 Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
***********************************************/

#ifndef DrizzleVideo_GUI_H
#define DrizzleVideo_GUI_H

#include <Qt/qdialog.h>
#include <Qt/qpushbutton.h>
#include <Qt/qmessagebox.h>
#include <Qt/qcombobox.h>
#include <Qt/qlabel.h>
#include <Qt/qlineedit.h>
#include <Qt/qlistwidget.h>


/**
*
* The class of the Drizzle plugin which handles video input.
*/
class DrizzleVideo_GUI : public QDialog
{
	Q_OBJECT
public:
	/**
	* Constructor for video input.
	* Handles GUI, input, calculations and output.
	*
	* @param Parent QWidget as parent for construction of QDialog.
	*/
	DrizzleVideo_GUI(QWidget* Parent);

	/**
	* Destructor for video input.
	* 
	*/
	~DrizzleVideo_GUI();

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
	* Slot to browse for an input video.
	* Connected to 'Browse' button.
	*/
	void browse();

	/**
	* Slot to update the information of the other selected video
	* displayed on the image GUI.
	*/
	void updateInfo();
private:
	/**
	* QLabel for video.
	*/
	QLabel *Video;

	/**
	* QLabel for frame size of video.
	*/
	QLabel *Size;

	/**
	* QLabel for number of frames of video.
	*/
	QLabel *NumImages;

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
	* QPushButton to browse for video.
	* Connects to Browse() SLOT.
	*/
	QPushButton *Browse;

	/**
	* QLineEdit to input directory.
	*/
	QLineEdit *Dir;

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
	* QLabel for number of frames.
	*/
	QLabel *num_images_text;
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
	* QLineEdit to input the number of frames to Drizzle.
	*/
	QLineEdit *num_images;

	/**
	* QString containing path to input video.
	*/
	QString fileName;

	/**
	* Initialisations needed for image GUI:
	* Connects buttons to SLOTS.
	* Set size of GUI
	*/
	void init();

};
#endif