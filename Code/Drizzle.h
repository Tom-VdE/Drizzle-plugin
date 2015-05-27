/*
 * The information in this file is
 * Copyright(c) 2015 Tom Van den Eynde
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#ifndef DRIZZLE_H
#define DRIZZLE_H

#include "Drizzle_GUI.h"
#include "ViewerShell.h"


#include <Qt/qaction.h>
#include <Qt/qgroupbox.h>
#include <Qt/qradiobutton.h>


class Drizzle : public QObject, public ViewerShell
{
	Q_OBJECT
public:
   Drizzle();
   virtual ~Drizzle();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   QWidget* getWidget() const;
   bool openGUI();

   Drizzle_GUI* gui;
};

#endif
