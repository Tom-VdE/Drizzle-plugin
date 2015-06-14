/*
 * The information in this file is
 * Copyright(c) 2015 Tom Van den Eynde
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "DataAccessor.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "Drizzle.h"
#include "DesktopServices.h"
#include "ModelServices.h"
#include "Progress.h"


#include <Qt\qapplication.h>
#include <Qt\qmessagebox.h>


REGISTER_PLUGIN_BASIC(ImageEnhancement, Drizzle);

Drizzle::Drizzle() : gui(NULL)
{
   setDescriptorId("{4539C009-F756-41A4-A94D-9867C0FF3B87}");
   setName("Drizzle");
   setVersion("Sample");
   setDescription("Combine several images using Drizzle algorithm");
   setCreator("Tom Van den Eynde");
   setCopyright("Copyright (C) 2015, Tom Van den Eynde");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Image Enhancement");
   setMenuLocation("[Image Enhancement]/Drizzle");
   setAbortSupported(true);
   destroyAfterExecute(false);
}

Drizzle::~Drizzle()
{
}

bool Drizzle::getInputSpecification(PlugInArgList*& pInArgList)
{
   pInArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pInArgList != NULL);
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   return true;
}

bool Drizzle::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pOutArgList != NULL);
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool Drizzle::openGUI()
{
	Service<ModelServices> Model;
	StepResource pStep( "Drizzle GUI", "app", "2CAC3F3C-9723-48EB-869B-745303B6227E" );
	std::vector<DataElement*> RasterElements = Model->getElements( "RasterElement" );
	if ( RasterElements.size() == 0 ){
		QMessageBox::critical( NULL, "Drizzle", "No RasterElements found!", "Back" );
		pStep->finalize( Message::Failure, "No RasterElements found!" );
		return false;
	}
	gui = new Drizzle_GUI(NULL, "GUI");
	//connect(gui, SIGNAL( finished() ), this, SLOT( abort()) );
    gui->show();

	pStep->finalize(Message::Success);
    return true;
}

bool Drizzle::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   return openGUI();
}

QWidget* Drizzle::getWidget() const
{
   return gui;
}
