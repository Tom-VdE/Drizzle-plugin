/*
* The information in this file is
* Copyright(c) 2015, Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "Layer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "Progress.h"
#include "ProgressResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "Drizzle_GUI.h"
#include "Service.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"

#include <Qt/qgridlayout.h>
#include <Qt/qapplication.h>
#include <Qt/qmessagebox.h>



Drizzle_GUI::Drizzle_GUI(QWidget* Parent, const char* Name): QDialog(Parent)
{
	this->setWindowTitle("Drizzle algorithm");

	/* WIDGETS */
	Apply = new QPushButton( "applyButton", this );
	Apply->setText("Drizzle!");

	Cancel = new QPushButton( "cancelButton", this );
	Cancel->setText("Cancel");

	Rasterlist1 = new QComboBox(this);
	Rasterlist2 = new QComboBox(this);
	
	/*LAYOUT*/
   
   QGridLayout* pLayout = new QGridLayout(this);
   
   pLayout->addWidget(Cancel, 6, 4);
   pLayout->addWidget(Apply, 6, 1);

   pLayout->addWidget( Rasterlist1, 1, 0, 1, 2 );
   pLayout->addWidget( Rasterlist2, 1, 3, 1, 2 );

   Rasterlist1->setFixedSize(400,20);
   Rasterlist2->setFixedSize(400,20);

   init();
}

Drizzle_GUI::~Drizzle_GUI()
{

}

void Drizzle_GUI::init(){

	//Initialize buttons
	connect(Cancel, SIGNAL(clicked()), this, SLOT(closeGUI()));
	connect(Apply, SIGNAL(clicked()), this, SLOT(PerformDrizzle()));

	//Get RasterElements
	Service<ModelServices> Model;
	RasterElements = Model->getElementNames("RasterElement");

   for (unsigned int i = 0; i < RasterElements.size(); i++)
   {
      Rasterlist1->insertItem(i, QString::fromStdString(RasterElements[i]));
	  Rasterlist2->insertItem(i, QString::fromStdString(RasterElements[i]));
   }
}

void Drizzle_GUI::closeGUI(){
	//QMessageBox::critical( NULL, "Drizzle", "Cancelled!", "OK" );
	this->reject();
}

void Drizzle_GUI::PerformDrizzle(){
	Service<ModelServices> pModel;
	ProgressResource pProgress("ProgressBar");
	RasterElement *image1 =  dynamic_cast<RasterElement*>(pModel->getElement(RasterElements.at(Rasterlist1->currentIndex()),"",NULL ));
	RasterElement *image2 =  dynamic_cast<RasterElement*>(pModel->getElement(RasterElements.at(Rasterlist2->currentIndex()),"",NULL ));

   if (image1 == NULL || image2 == NULL)
		{				
			pProgress->updateProgress("Image import failed", 100, ERRORS);
			return;	
		}
   pProgress->updateProgress("Image import succesfull", 0, NORMAL);

   if(!image1->isGeoreferenced() || !image2->isGeoreferenced())
   {
	    pProgress->updateProgress("Images are not georeferenced", 100, ERRORS);
		return;	
   }
   //Align images based on geographical information/detection algorithms
	this->accept();
}




