/********************************************//*
* 
* @file: Drizzle.cpp
*
* The information in this file is
* Copyright(c) 2015 Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
***********************************************/



#include "DataAccessor.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "Drizzle.h"
#include "DesktopServices.h"
#include "ModelServices.h"
#include "Progress.h"
#include "SessionItemSerializer.h"
#include "hdf5.h"

#include <Qt\qapplication.h>
#include <Qt\qmessagebox.h>
#include <Qt\qlayout.h>

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
   allowMultipleInstances(true);
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
	gui = new QDialog(NULL);
	gui->setWindowTitle("Drizzle algorithm");

	//WIDGET
	QPushButton* Image = new QPushButton( "imageButton", gui);
	Image->setText("Drizzle images.");

	QPushButton* Video = new QPushButton( "videoButton", gui);
	Video->setText("Drizzle video to image.");

	QPushButton* Cancel = new QPushButton( "cancelButton", gui);
	Cancel->setText("Cancel");

	QGridLayout* pLayout = new QGridLayout(gui);

	//LAYOUT
	pLayout->addWidget(Image, 0, 0);
	pLayout->addWidget(Video, 0, 1);
	pLayout->addWidget(Cancel, 0, 2);

	//Make connections slots & signals
	connect(Image, SIGNAL(clicked()), this, SLOT(imageGUI()));
	connect(Video, SIGNAL(clicked()), this, SLOT(videoGUI()));
	connect(Cancel, SIGNAL(clicked()), this, SLOT(closemainGUI()));
	
	gui->show();

    return true;
}

void Drizzle::imageGUI()
{
		
	StepResource pStep( "Drizzle GUI", "app", "2CAC3F3C-9723-48EB-869B-745303B6227E" );
	//Get all opened RasterElements
	Service<ModelServices> Model;
	std::vector<DataElement*> RasterElements = Model->getElements( "RasterElement" );
	
	//Check whether input RasterElements are present
	if ( RasterElements.size() == 0 ){
		QMessageBox::critical( gui, "Drizzle", "No RasterElements found!", "Back" );
		pStep->finalize( Message::Failure, "No RasterElements found!" );
		return;
	}
	//Close main GUI
	delete gui;

	//Open Image GUI
	Service<DesktopServices> pDesktop;
	gui = new Drizzle_GUI(pDesktop->getMainWidget());
    gui->show();

	pStep->finalize(Message::Success);
}

void Drizzle::videoGUI()
{
	StepResource pStep( "Drizzle GUI", "app", "96DCEBD9-32CA-44E5-AEA8-2DDAD7DAD939" );
	
	//Close main GUI
	delete gui;
	
	//Open Video GUI
	Service<DesktopServices> pDesktop;
	gui = new DrizzleVideo_GUI(pDesktop->getMainWidget());
    gui->show();

	pStep->finalize(Message::Success);
}

bool Drizzle::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   return openGUI();
}

QWidget* Drizzle::getWidget() const
{
   return gui;
}

bool Drizzle::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0);
}

bool Drizzle::deserialize(SessionItemDeserializer &deserializer)
{
   return openGUI();
}

void Drizzle::closemainGUI()
{
	gui->accept();
}
