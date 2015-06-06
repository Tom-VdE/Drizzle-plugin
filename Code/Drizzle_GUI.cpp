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
#include "Statistics.h"
#include "DataAccessorImpl.h"



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

	Image1 = new QLabel("Image 1", this);
	Image2 = new QLabel("Image 2", this);

	Size1 = new QLabel("Size of image 1", this);
	Size2 = new QLabel("Size of image 2", this);
	Geo1 = new QLabel("Coordinates of image 1", this);
	Geo2 = new QLabel("Coordinates of image 2", this);

	Rasterlist1 = new QComboBox(this);
	Rasterlist2 = new QComboBox(this);
	
	/*LAYOUT*/
   
   QGridLayout* pLayout = new QGridLayout(this);
   
   pLayout->addWidget(Cancel, 6, 4);
   pLayout->addWidget(Apply, 6, 1);

   pLayout->addWidget( Rasterlist1, 1, 0, 1, 2 );
   pLayout->addWidget( Rasterlist2, 1, 3, 1, 2 );

   pLayout->addWidget( Image1, 0, 0);
   pLayout->addWidget( Image2, 0, 3);
   pLayout->addWidget( Size1, 2, 0);
   pLayout->addWidget( Size2, 2, 3);
   pLayout->addWidget( Geo1, 3, 0);
   pLayout->addWidget( Geo2, 3, 3);

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
	connect(Rasterlist1, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInfo1()));
	connect(Rasterlist2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInfo2()));

	//Get RasterElements
	Service<ModelServices> Model;
	RasterElements = Model->getElementNames("RasterElement");

   for (unsigned int i = 0; i < RasterElements.size(); i++)
   {
      Rasterlist1->insertItem(i, QString::fromStdString(RasterElements[i]));
	  Rasterlist2->insertItem(i, QString::fromStdString(RasterElements[i]));
   }
	const RasterDataDescriptor *Des1 = dynamic_cast<const RasterDataDescriptor*>(Model->getElement(RasterElements.at(0),"",NULL)->getDataDescriptor());
	const std::vector<DimensionDescriptor>& Rows = Des1->getRows();
	const std::vector<DimensionDescriptor>& Columns = Des1->getColumns();
	Size1->setText("Size of image 1: "+ QString::number(Rows.size()) + "x" + QString::number(Columns.size()));
	Size2->setText("Size of image 2: "+ QString::number(Rows.size()) + "x" + QString::number(Columns.size()));
}

void Drizzle_GUI::updateInfo1(){
	Service<ModelServices> Model;
	const RasterDataDescriptor *Des = dynamic_cast<const RasterDataDescriptor*>(Model->getElement(RasterElements.at(Rasterlist1->currentIndex()),"",NULL)->getDataDescriptor());
	const std::vector<DimensionDescriptor>& Rows = Des->getRows();
	const std::vector<DimensionDescriptor>& Columns = Des->getColumns();
	Size1->setText("Size of image 1: "+ QString::number(Rows.size()) + "x" + QString::number(Columns.size()));
}

void Drizzle_GUI::updateInfo2(){
	Service<ModelServices> Model;
	const RasterDataDescriptor *Des = dynamic_cast<const RasterDataDescriptor*>(Model->getElement(RasterElements.at(Rasterlist2->currentIndex()),"",NULL)->getDataDescriptor());
	const std::vector<DimensionDescriptor>& Rows = Des->getRows();
	const std::vector<DimensionDescriptor>& Columns = Des->getColumns();
	Size2->setText("Size of image 2: "+ QString::number(Rows.size()) + "x" + QString::number(Columns.size()));
}

void Drizzle_GUI::closeGUI(){
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

   //Get DataAccessors
   DataAccessor data1 = image1->getDataAccessor();
   DataAccessor data2 = image2->getDataAccessor();
   
   if(!data1->isValid() || !data2->isValid())
   {
	    pProgress->updateProgress("Data is not valid", 100, ERRORS);
		return;	
   }

   //Align images based on geographical information/detection algorithms
   pProgress->updateProgress("Done", 100, NORMAL);
	this->accept();
}