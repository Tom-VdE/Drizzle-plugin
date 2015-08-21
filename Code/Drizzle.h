/********************************************//*
* 
* @file: Drizzle.h
*
* The information in this file is
* Copyright(c) 2015 Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
***********************************************/


#ifndef DRIZZLE_H
#define DRIZZLE_H

#include "Drizzle_GUI.h"
#include "DrizzleVideo_GUI.h"
#include "ViewerShell.h"

#include <Qt/qaction.h>
#include <Qt/qgroupbox.h>
#include <Qt/qradiobutton.h>

/**
*
* The base class for the Drizzle plugin that manages the general GUI
* and handles GUIs for Drizzle_GUI and DrizzleVideo_GUI.
*/
class Drizzle : public QObject, public ViewerShell
{
	Q_OBJECT
public:
	/**
	* Constructor for base class of Drizzle plugin.
	*
	*/
	Drizzle();

	/**
	* Destructor for base class of Drizzle plugin.
	*/
	~Drizzle();

	bool serialize(SessionItemSerializer& serializer) const;
	bool deserialize(SessionItemDeserializer& deserializer);
	bool getInputSpecification(PlugInArgList*& pInArgList);
	bool getOutputSpecification(PlugInArgList*& pOutArgList);
	bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
	QWidget* getWidget() const;

public slots:
	/**
	* Slot for closing the general GUI, connected to 'Cancel' button.
	*/
	void closemainGUI();

	/**
	* Slot to initialise the Drizzle plugin for images, connected
	* to the 'Image' button.
	*/
	void imageGUI();

	/**
	* Slot to initialise the Drizzle plugin for videos, connected
	* to the 'Video' button.
	*/
	void videoGUI();

private:
	/**
	* Initialises the general GUI which lets the user choose between
	* video or images input.
	* 
	* @return	True when initialising of GUI is succesfull. 
	*			Default implementation returns \c true.
	*/
	bool openGUI();

	/**
	* QDialog of the main window of the Drizzle plugin.
	*/
	QDialog* gui;

};

#endif
