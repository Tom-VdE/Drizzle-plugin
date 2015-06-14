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
#include "RasterUtilities.h"
#include "switchOnEncoding.h"
#include "Georeference.h"
#include "GeoreferenceDescriptor.h"
#include "GeoPoint.h"
#include "Layerlist.h"

#include <Qt/qgridlayout.h>
#include <Qt/qapplication.h>
#include <Qt/qmessagebox.h>


namespace
{
	template<typename T>
	void Drizzle(T* pData, DataAccessor pSrcAcc, DataAccessor pSrcAcc2, int row, int col, int rowSize, int colSize)
	{
		int prevCol = std::max(col - 1, 0);
		int prevRow = std::max(row - 1, 0);
		int nextCol = std::min(col + 1, colSize - 1);
		int nextRow = std::min(row + 1, rowSize - 1);

		pSrcAcc->toPixel(prevRow, prevCol);
		VERIFYNRV(pSrcAcc.isValid());
		T upperLeftVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

		pSrcAcc->toPixel(prevRow, col);
		VERIFYNRV(pSrcAcc.isValid());
		T upVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

		pSrcAcc->toPixel(prevRow, nextCol);
		VERIFYNRV(pSrcAcc.isValid());
		T upperRightVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

		pSrcAcc->toPixel(row, prevCol);
		VERIFYNRV(pSrcAcc.isValid());
		T leftVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

		pSrcAcc->toPixel(row, nextCol);
		VERIFYNRV(pSrcAcc.isValid());
		T rightVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

		pSrcAcc->toPixel(nextRow, prevCol);
		VERIFYNRV(pSrcAcc.isValid());
		T lowerLeftVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

		pSrcAcc->toPixel(nextRow, col);
		VERIFYNRV(pSrcAcc.isValid());
		T downVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

		pSrcAcc->toPixel(nextRow, nextCol);
		VERIFYNRV(pSrcAcc.isValid());
		T lowerRightVal = *reinterpret_cast<T*>(pSrcAcc->getColumn());

		double gx = -1.0 * upperLeftVal + -2.0 * leftVal + -1.0 * lowerLeftVal + 1.0 * upperRightVal + 2.0 *
			rightVal + 1.0 * lowerRightVal;
		double gy = -1.0 * lowerLeftVal + -2.0 * downVal + -1.0 * lowerRightVal + 1.0 * upperLeftVal + 2.0 *
			upVal + 1.0 * upperRightVal;
		double magnitude = sqrt(gx * gx + gy * gy);

		*pData = static_cast<T>(magnitude);

		/*for(int srccol = 0; srccol < colSize; srccol++){
		for(int srcrow = 0; srcrow < rowSize; srcrow++){
		pSrcAcc1->toPixel(srccol, srcrow);
		VERIFYNRV(pSrcAcc1.isValid());
		T pixel = *reinterpret_cast<T*>(pSrcAcc1->getColumn());

		LocationType geo1 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,0)));
		LocationType geo2 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,Des->getRows().size()-1)));
		LocationType geo3 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getColumns().size()-1,0)));
		LocationType geo4 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getColumns().size()-1,Des->getRows().size()-1)));

		double tlx1 = geo1.mX;
		double tly1 = geo1.mY;
		double trx1 = geo2.mX;
		double try1 = geo2.mY;
		double blx1 = geo3.mX;
		double bly1 = geo3.mY;
		double brx1 = geo4.mX;
		double bry1 = geo4.mY;

		double gx = -1.0 * upperLeftVal + -2.0 * leftVal + -1.0 * lowerLeftVal + 1.0 * upperRightVal + 2.0 *
		rightVal + 1.0 * lowerRightVal;
		double gy = -1.0 * lowerLeftVal + -2.0 * downVal + -1.0 * lowerRightVal + 1.0 * upperLeftVal + 2.0 *
		upVal + 1.0 * upperRightVal;
		double magnitude = sqrt(gx * gx + gy * gy);
		}
		}*/

		*pData = static_cast<T>(magnitude);
	}
};

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

	Size1 = new QLabel("Size:", this);
	Size2 = new QLabel("Size:", this);
	Geo1 = new QLabel("Coordinates:", this);
	Geo2 = new QLabel("Coordinates:", this);

	Rasterlist1 = new QComboBox(this);
	Rasterlist2 = new QComboBox(this);

	x_out_text = new QLabel("Output size (x)");
	y_out_text = new QLabel("Output size (y)");
	dropsize_text = new QLabel("Dropsize");
	x_out = new QLineEdit(this);
	y_out = new QLineEdit(this);
	dropsize = new QLineEdit(this);

	/*LAYOUT*/

	QGridLayout* pLayout = new QGridLayout(this);

	pLayout->addWidget( Rasterlist1, 1, 0, 1, 3 );
	pLayout->addWidget( Rasterlist2, 1, 4, 1, 3 );

	pLayout->addWidget( Image1, 0, 0, 1, 3);
	pLayout->addWidget( Image2, 0, 4, 1, 3);
	pLayout->addWidget( Size1, 2, 0, 1, 3);
	pLayout->addWidget( Size2, 2, 4, 1, 3);
	pLayout->addWidget( Geo1, 3, 0, 1, 3);
	pLayout->addWidget( Geo2, 3, 4, 1, 3);

	pLayout->addWidget( x_out_text,4,0);
	pLayout->addWidget( y_out_text,4,1);
	pLayout->addWidget( dropsize_text,4,2);
	pLayout->addWidget( x_out,5,0);
	pLayout->addWidget( y_out,5,1);
	pLayout->addWidget( dropsize,5,2);

	pLayout->addWidget(Cancel, 6, 4,1,3);
	pLayout->addWidget(Apply, 6, 0,1,3);

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
	Size1->setText("Size:\t"+ QString::number(Rows.size()) + "x" + QString::number(Columns.size()));
	Size2->setText("Size:\t"+ QString::number(Rows.size()) + "x" + QString::number(Columns.size()));
}

void Drizzle_GUI::updateInfo1(){
	Service<ModelServices> Model;
	//Get size information
	const RasterDataDescriptor *Des = dynamic_cast<const RasterDataDescriptor*>(Model->getElement(RasterElements.at(Rasterlist1->currentIndex()),"",NULL)->getDataDescriptor());
	const std::vector<DimensionDescriptor>& Rows = Des->getRows();
	const std::vector<DimensionDescriptor>& Columns = Des->getColumns();
	Size1->setText("Size:\t"+ QString::number(Rows.size()) + "x" + QString::number(Columns.size()));
	//Get geo information
	FactoryResource<DataRequest> pRequest;
	DataAccessor pSrcAcc = dynamic_cast<RasterElement*>(Model->getElement(RasterElements.at(Rasterlist1->currentIndex()),"",NULL ))->getDataAccessor(pRequest.release());
	LocationType geo1 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,0)));
	LocationType geo2 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,Des->getRows().size()-1)));
	LocationType geo3 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getColumns().size()-1,0)));
	LocationType geo4 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getColumns().size()-1,Des->getRows().size()-1)));
	Geo1->setText("Coordinates:\t(" + QString::number(geo1.mX) + "," + QString::number(geo1.mY) + "," + QString::number(geo1.mZ)+")\t(" + QString::number(geo2.mX) + "," + QString::number(geo2.mY) + "," + QString::number(geo2.mZ)+") \n" +
		"\n\t(" + QString::number(geo3.mX) + "," + QString::number(geo3.mY) + "," + QString::number(geo3.mZ)+")\t(" + QString::number(geo4.mX) + "," + QString::number(geo4.mY) + "," + QString::number(geo4.mZ)+")");
}

void Drizzle_GUI::updateInfo2(){
	Service<ModelServices> Model;
	//Get size information
	const RasterDataDescriptor *Des = dynamic_cast<const RasterDataDescriptor*>(Model->getElement(RasterElements.at(Rasterlist2->currentIndex()),"",NULL)->getDataDescriptor());
	const std::vector<DimensionDescriptor>& Rows = Des->getRows();
	const std::vector<DimensionDescriptor>& Columns = Des->getColumns();
	Size2->setText("Size:\t"+ QString::number(Rows.size()) + "x" + QString::number(Columns.size()));
	//Get geo information
	FactoryResource<DataRequest> pRequest;
	DataAccessor pSrcAcc = dynamic_cast<RasterElement*>(Model->getElement(RasterElements.at(Rasterlist2->currentIndex()),"",NULL ))->getDataAccessor(pRequest.release());
	LocationType geo1 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,0)));
	LocationType geo2 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,Des->getRows().size()-1)));
	LocationType geo3 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getColumns().size()-1,0)));
	LocationType geo4 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getColumns().size()-1,Des->getRows().size()-1)));
	Geo2->setText("Coordinates:\t(" + QString::number(geo1.mX) + "," + QString::number(geo1.mY) + "," + QString::number(geo1.mZ)+")\t(" + QString::number(geo2.mX) + "," + QString::number(geo2.mY) + "," + QString::number(geo2.mZ)+") \n" +
		"\n\t(" + QString::number(geo3.mX) + "," + QString::number(geo3.mY) + "," + QString::number(geo3.mZ)+")\t(" + QString::number(geo4.mX) + "," + QString::number(geo4.mY) + "," + QString::number(geo4.mZ)+")");
}

void Drizzle_GUI::closeGUI(){
	this->reject();
}

bool Drizzle_GUI::PerformDrizzle(){
	Service<ModelServices> pModel;
	StepResource pStep("Drizzle", "app", "4539C009-F756-41A4-A94D-9867C0FF3B87");
	ProgressResource pProgress("ProgressBar");
	RasterElement *image1 =  dynamic_cast<RasterElement*>(pModel->getElement(RasterElements.at(Rasterlist1->currentIndex()),"",NULL ));
	RasterElement *image2 =  dynamic_cast<RasterElement*>(pModel->getElement(RasterElements.at(Rasterlist2->currentIndex()),"",NULL ));

	if (image1 == NULL || image2 == NULL)
	{				
		pProgress->updateProgress("Image import failed", 100, ERRORS);
		return false;	
	}
	pProgress->updateProgress("Image import succesfull", 0, NORMAL);

	if(!image1->isGeoreferenced() || !image2->isGeoreferenced())
	{
		pProgress->updateProgress("Images are not georeferenced", 100, ERRORS);
		return false;	
	}

	RasterDataDescriptor* pDesc1 = static_cast<RasterDataDescriptor*>(image1->getDataDescriptor());
	RasterDataDescriptor* pDesc2 = static_cast<RasterDataDescriptor*>(image2->getDataDescriptor());

	if (pDesc1->getDataType() == INT4SCOMPLEX || pDesc1->getDataType() == FLT8COMPLEX || pDesc2->getDataType() == INT4SCOMPLEX || pDesc2->getDataType() == FLT8COMPLEX)
	{
		std::string msg = "Drizzle cannot be performed on complex types.";
		pStep->finalize(Message::Failure, msg);
		return false;
	}

	FactoryResource<DataRequest> pRequest1;
	FactoryResource<DataRequest> pRequest2;
	DataAccessor pSrcAcc1 = image1->getDataAccessor(pRequest1.release());
	DataAccessor pSrcAcc2 = image2->getDataAccessor(pRequest2.release());

	if(x_out->text().isNull() || y_out->text().isNull() || x_out->text().isEmpty() || y_out->text().isEmpty())
	{
		std::string msg = "No output size specified!";
		pStep->finalize(Message::Failure, msg);
		return false;
	}

	ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(image1->getName() + "_Drizzled", x_out->text().toDouble(), y_out->text().toDouble(), pDesc1->getDataType()));

	if (pResultCube.get() == NULL){
		std::string msg = "A raster cube could not be created.";
		pStep->finalize(Message::Failure, msg);
		return false;
	}

	FactoryResource<DataRequest> pResultRequest;
	pResultRequest->setWritable(true);
	DataAccessor pDestAcc = pResultCube->getDataAccessor(pResultRequest.release());

	for (unsigned int row = 0; row < pDesc1->getRowCount(); ++row){ 
		pProgress->updateProgress("Calculating result", row * 100 / pDesc1->getRowCount(), NORMAL);
		if (!pDestAcc.isValid())
		{
			std::string msg = "Unable to access the cube data.";
			pStep->finalize(Message::Failure, msg);
			pProgress->updateProgress(msg, 0, ERRORS);
			return false;
		}

		for (unsigned int col = 0; col < pDesc1->getColumnCount(); ++col)
		{
			switchOnEncoding(pDesc1->getDataType(), Drizzle, pDestAcc->getColumn(), pSrcAcc1, pSrcAcc2, row, col, pDesc1->getRowCount(), pDesc1->getColumnCount());
			pDestAcc->nextColumn();
		}

		pDestAcc->nextRow();
	}


	Service<DesktopServices> pDesktop;

	SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pResultCube->getName(), SPATIAL_DATA_WINDOW));

	SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();
	if (pView == NULL){
		std::string msg = "Unable to create view.";
		pStep->finalize(Message::Failure, msg);
		pProgress->updateProgress(msg, 0, ERRORS);
		return false;
	}

	pView->setPrimaryRasterElement(pResultCube.get());
	pView->createLayer(RASTER, pResultCube.get());

	pResultCube.release();

	pStep->finalize();
	pProgress->updateProgress("Done", 100, NORMAL);
	this->accept();
	return true;
}