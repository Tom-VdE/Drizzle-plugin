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
#include "GcpList.h"
#include "PluginResource.h"
#include "Executable.h"

#include "AppConfig.h"
#include "AppVerify.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "StringUtilities.h"

#include <Qt/QInputDialog.h>
#include <Qt/qgridlayout.h>
#include <Qt/qapplication.h>
#include <Qt/qmessagebox.h>

namespace
{
	template<typename T>
	void Drizzle(T* pData, DataAccessor pDestAcc, DataAccessor pSrcAcc, unsigned int row, unsigned int col, unsigned int rowSize, unsigned int colSize)
	{
		double pixelvalue=0;
		LocationType desgeo1 = pDestAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,0)));
		LocationType desgeo2 = pDestAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,dynamic_cast<const RasterDataDescriptor*>(pDestAcc->getAssociatedRasterElement()->getDataDescriptor())->getRows().size()-1)),0);
		LocationType desgeo3 = pDestAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(dynamic_cast<const RasterDataDescriptor*>(pDestAcc->getAssociatedRasterElement()->getDataDescriptor())->getColumns().size()-1,0)),0);
		LocationType desgeo4 = pDestAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(dynamic_cast<const RasterDataDescriptor*>(pDestAcc->getAssociatedRasterElement()->getDataDescriptor())->getColumns().size()-1,dynamic_cast<const RasterDataDescriptor*>(pDestAcc->getAssociatedRasterElement()->getDataDescriptor())->getRows().size()-1)),0);

		double dtlx1 = desgeo1.mY;			//x coordinate of top left pixel
		double dtly1 = desgeo1.mX;			//y coordinate of top left pixel
		double dtrx1 = desgeo2.mY;			//x coordinate of top right pixel
		double dtry1 = desgeo2.mX;			//y coordinate of top right pixel
		double dblx1 = desgeo3.mY;			//x coordinate of bottom left pixel
		double dbly1 = desgeo3.mX;			//y coordinate of bottom left pixel
		double dbrx1 = desgeo4.mY;			//x coordinate of bottom right pixel
		double dbry1 = desgeo4.mX;			//y coordinate of bottom right pixel

		double ddtx1 = dtrx1 - dtlx1;		//difference in x coordinate over top of image
		double ddty1 = dtry1 - dtly1;		//difference in y coordinate over top of image
		double ddlx1 = dblx1 - dtlx1;		//difference in x coordinate over left side of image
		double ddly1 = dbly1 - dtly1;		//difference in y coordinate over left side of image
		double ddbx1 = dbrx1 - dblx1;		//difference in x coordinate over bottom of image
		double ddby1 = dbry1 - dbly1;		//difference in y coordinate over bottom of image
		double ddrx1 = dbrx1 - dtrx1;		//difference in x coordinate over right side of image
		double ddry1 = dbry1 - dtry1;		//difference in y coordinate over right side of image

		/*double ddxv = (((ddbx1-ddtx1)/rowSize)*double(row) + ddtx1)/double(colSize);	//total difference in x coordinate in vertical direction
		double ddxh = (((ddrx1-ddlx1)/colSize)*double(col) + ddlx1)/double(rowSize);	//total difference in x coorindate in horizontal direction
		double ddyv = (((ddby1-ddty1)/rowSize)*double(row) + ddty1)/double(colSize);	//total difference in y coorindate in vertical direction
		double ddyh = (((ddry1-ddly1)/colSize)*double(col) + ddly1)/double(rowSize);	//total difference in y coorindate in horizontal direction

		double dptlx1 = dtlx1 + ddxv*double(col) + ddxh*double(row);					//top left x coordinate of pixel
		double dptly1 = dtly1 + ddyv*double(col) + ddyh*double(row);					//top left y coordinate of pixel
		double dptrx1 = dtlx1 + ddxv*double(col) + ddxh*double(row+1);				//top right x coordinate of pixel
		double dptry1 = dtly1 + ddyv*double(col) + ddyh*double(row+1);				//top right y coordinate of pixel
		double dpblx1 = dtlx1 + ddxv*double(col+1) + ddxh*double(row);				//bottom left x coordinate of pixel
		double dpbly1 = dtly1 + ddyv*double(col+1) + ddyh*double(row);				//bottom left y coordinate of pixel
		double dpbrx1 = dtlx1 + ddxv*double(col+1) + ddxh*double(row+1);			//bottom right x coordinate of pixel
		double dpbry1 = dtly1 + ddyv*double(col+1) + ddyh*double(row+1);			//bottom right y coordinate of pixel*/

		double dptlx1 = dtlx1 + ((((ddbx1-ddtx1)/rowSize)*double(row) + ddtx1)/double(colSize))*double(col) + ((((ddrx1-ddlx1)/colSize)*double(col) + ddlx1)/double(rowSize))*double(row);					//top left x coordinate of pixel
		double dptly1 = dtly1 + ((((ddby1-ddty1)/rowSize)*double(row) + ddty1)/double(colSize))*double(col) + ((((ddry1-ddly1)/colSize)*double(col) + ddly1)/double(rowSize))*double(row);					//top left y coordinate of pixel
		double dptrx1 = dtlx1 + ((((ddbx1-ddtx1)/rowSize)*double(row+1) + ddtx1)/double(colSize))*double(col) + ((((ddrx1-ddlx1)/colSize)*double(col) + ddlx1)/double(rowSize))*double(row+1);				//top right x coordinate of pixel
		double dptry1 = dtly1 + ((((ddby1-ddty1)/rowSize)*double(row+1) + ddty1)/double(colSize))*double(col) + ((((ddry1-ddly1)/colSize)*double(col) + ddly1)/double(rowSize))*double(row+1);				//top right y coordinate of pixel
		double dpblx1 = dtlx1 + ((((ddbx1-ddtx1)/rowSize)*double(row) + ddtx1)/double(colSize))*double(col+1) + ((((ddrx1-ddlx1)/colSize)*double(col+1) + ddlx1)/double(rowSize))*double(row);				//bottom left x coordinate of pixel
		double dpbly1 = dtly1 + ((((ddby1-ddty1)/rowSize)*double(row) + ddty1)/double(colSize))*double(col+1) + ((((ddry1-ddly1)/colSize)*double(col+1) + ddly1)/double(rowSize))*double(row);				//bottom left y coordinate of pixel
		double dpbrx1 = dtlx1 + ((((ddbx1-ddtx1)/rowSize)*double(row+1) + ddtx1)/double(colSize))*double(col+1) + ((((ddrx1-ddlx1)/colSize)*double(col+1) + ddlx1)/double(rowSize))*double(row+1);			//bottom right x coordinate of pixel
		double dpbry1 = dtly1 + ((((ddby1-ddty1)/rowSize)*double(row+1) + ddty1)/double(colSize))*double(col+1) + ((((ddry1-ddly1)/colSize)*double(col+1) + ddly1)/double(rowSize))*double(row+1);			//bottom right y coordinate of pixel*/
		
		unsigned int tlsrcrow = pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*(new LocationType(dptly1,dptlx1))).mX;
		unsigned int tlsrccol = pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*(new LocationType(dptly1,dptlx1))).mY;
		unsigned int trsrcrow = pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*(new LocationType(dptry1,dptrx1))).mX;
		unsigned int trsrccol = pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*(new LocationType(dptry1,dptrx1))).mY;
		unsigned int brsrcrow = pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*(new LocationType(dpbry1,dpbrx1))).mX;
		unsigned int brsrccol = pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*(new LocationType(dpbry1,dpbrx1))).mY;
		unsigned int blsrcrow = pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*(new LocationType(dpbly1,dpblx1))).mX;
		unsigned int blsrccol = pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*(new LocationType(dpbly1,dpblx1))).mY;

		LocationType geo1 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,0)));
		LocationType geo2 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,dynamic_cast<const RasterDataDescriptor*>(pSrcAcc->getAssociatedRasterElement()->getDataDescriptor())->getRows().size()-1)));
		LocationType geo3 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(dynamic_cast<const RasterDataDescriptor*>(pSrcAcc->getAssociatedRasterElement()->getDataDescriptor())->getColumns().size()-1,0)));
		LocationType geo4 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(dynamic_cast<const RasterDataDescriptor*>(pSrcAcc->getAssociatedRasterElement()->getDataDescriptor())->getColumns().size()-1,dynamic_cast<const RasterDataDescriptor*>(pSrcAcc->getAssociatedRasterElement()->getDataDescriptor())->getRows().size()-1)));

		double tlx1 = geo1.mY;			//x coordinate of top left pixel
		double tly1 = geo1.mX;			//y coordinate of top left pixel
		double trx1 = geo2.mY;			//x coordinate of top right pixel
		double try1 = geo2.mX;			//y coordinate of top right pixel
		double blx1 = geo3.mY;			//x coordinate of bottom left pixel
		double bly1 = geo3.mX;			//y coordinate of bottom left pixel
		double brx1 = geo4.mY;			//x coordinate of bottom right pixel
		double bry1 = geo4.mX;			//y coordinate of bottom right pixel

		double dtx1 = trx1 - tlx1;		//difference in x coordinate over top of image
		double dty1 = try1 - tly1;		//difference in y coordinate over top of image
		double dlx1 = blx1 - tlx1;		//difference in x coordinate over left side of image
		double dly1 = bly1 - tly1;		//difference in y coordinate over left side of image
		double dbx1 = brx1 - blx1;		//difference in x coordinate over bottom of image
		double dby1 = bry1 - bly1;		//difference in y coordinate over bottom of image
		double drx1 = brx1 - trx1;		//difference in x coordinate over right of image
		double dry1 = bry1 - try1;		//difference in y coordinate over right of image

		unsigned int srcrowSize = dynamic_cast<const RasterDataDescriptor*>(pSrcAcc->getAssociatedRasterElement()->getDataDescriptor())->getRows().size();
		unsigned int srccolSize = dynamic_cast<const RasterDataDescriptor*>(pSrcAcc->getAssociatedRasterElement()->getDataDescriptor())->getColumns().size();

		for(unsigned int srccol = std::min(std::min(tlsrccol,trsrccol),std::min(blsrccol,brsrccol)); srccol <= std::max(std::max(tlsrccol,trsrccol),std::max(blsrccol,brsrccol)); srccol++){
			for(unsigned int srcrow = std::min(std::min(tlsrcrow,trsrcrow),std::min(blsrcrow,brsrcrow)); srcrow <= std::max(std::max(tlsrcrow,trsrcrow),std::max(blsrcrow,brsrcrow)); srcrow++){
				/*double dxv = (((dbx1-dtx1)/srcrowSize)*double(srcrow) + dtx1)/double(srccolSize);	//total difference in x coordinate in vertical direction
				double dxh = (((drx1-dlx1)/srccolSize)*double(srccol) + dlx1)/double(srcrowSize);	//total difference in x coorindate in horizontal direction
				double dyv = (((dby1-dty1)/srcrowSize)*double(srcrow) + dty1)/double(srccolSize);	//total difference in y coorindate in vertical direction
				double dyh = (((dry1-dly1)/srccolSize)*double(srccol) + dly1)/double(srcrowSize);	//total difference in y coorindate in horizontal direction

				//		(ptlx1,ptly1)	o-------------o (ptrx1,ptry1)
				//						|			  |
				//						|  one pixel  |
				//						|			  |
				//						|			  |
				//		(pblx1,pbly1)	o-------------o (pbrx1,pbry1)

				//double dropsize = 0.7;

				double ptlx1 = tlx1 + dxv*srccol + dxh*srcrow;					//top left x coordinate of pixel
				double ptly1 = tly1 + dyv*srccol + dyh*srcrow;					//top left y coordinate of pixel
				double ptrx1 = tlx1 + dxv*srccol + dxh*(srcrow+1);				//top right x coordinate of pixel
				double ptry1 = tly1 + dyv*srccol + dyh*(srcrow+1);				//top right y coordinate of pixel
				double pblx1 = tlx1 + dxv*(srccol+1) + dxh*srcrow;				//bottom left x coordinate of pixel
				double pbly1 = tly1 + dyv*(srccol+1) + dyh*srcrow;				//bottom left y coordinate of pixel
				double pbrx1 = tlx1 + dxv*(srccol+1) + dxh*(srcrow+1);			//bottom right x coordinate of pixel
				double pbry1 = tly1 + dyv*(srccol+1) + dyh*(srcrow+1);			//bottom right y coordinate of pixel*/

				double ptlx1 = dtlx1 + ((((dbx1-dtx1)/srcrowSize)*double(srcrow) + dtx1)/double(srccolSize))*double(srccol) + ((((drx1-dlx1)/srccolSize)*double(srccol) + dlx1)/double(srcrowSize))*double(srcrow);					//top left x coordinate of pixel
				double ptly1 = dtly1 + ((((dby1-dty1)/srcrowSize)*double(srcrow) + dty1)/double(srccolSize))*double(srccol) + ((((dry1-dly1)/srccolSize)*double(srccol) + dly1)/double(srcrowSize))*double(srcrow);					//top left y coordinate of pixel
				double ptrx1 = dtlx1 + ((((dbx1-dtx1)/srcrowSize)*double(srcrow+1) + dtx1)/double(srccolSize))*double(srccol) + ((((drx1-dlx1)/srccolSize)*double(srccol) + dlx1)/double(srcrowSize))*double(srcrow+1);				//top right x coordinate of pixel
				double ptry1 = dtly1 + ((((dby1-dty1)/srcrowSize)*double(srcrow+1) + dty1)/double(srccolSize))*double(srccol) + ((((dry1-dly1)/srccolSize)*double(srccol) + dly1)/double(srcrowSize))*double(srcrow+1);				//top right y coordinate of pixel
				double pblx1 = dtlx1 + ((((dbx1-dtx1)/srcrowSize)*double(srcrow) + dtx1)/double(srccolSize))*double(srccol+1) + ((((drx1-dlx1)/srccolSize)*double(srccol+1) + dlx1)/double(srcrowSize))*double(srcrow);				//bottom left x coordinate of pixel
				double pbly1 = dtly1 + ((((dby1-dty1)/srcrowSize)*double(srcrow) + dty1)/double(srccolSize))*double(srccol+1) + ((((dry1-dly1)/srccolSize)*double(srccol+1) + dly1)/double(srcrowSize))*double(srcrow);				//bottom left y coordinate of pixel
				double pbrx1 = dtlx1 + ((((dbx1-dtx1)/srcrowSize)*double(srcrow+1) + dtx1)/double(srccolSize))*double(srccol+1) + ((((drx1-dlx1)/srccolSize)*double(srccol+1) + dlx1)/double(srcrowSize))*double(srcrow+1);			//bottom right x coordinate of pixel
				double pbry1 = dtly1 + ((((dby1-dty1)/srcrowSize)*double(srcrow+1) + dty1)/double(srccolSize))*double(srccol+1) + ((((dry1-dly1)/srccolSize)*double(srccol+1) + dly1)/double(srcrowSize))*double(srcrow+1);			//bottom right y coordinate of pixel*/
								
				//CHECK WHETHER INPUT AND OUTPUT PIXEL COULD OVERLAP
				if((std::max(std::max(pbrx1,pblx1),std::max(ptrx1,ptlx1))>=std::min(std::min(dpbrx1,dpblx1),std::min(dptrx1,dptlx1)))
					&& (std::min(std::min(pbrx1,pblx1),std::min(ptrx1,ptlx1))<=std::max(std::max(dpbrx1,dpblx1),std::max(dptrx1,dptlx1)))
					&& (std::max(std::max(pbry1,pbly1),std::max(ptry1,ptly1))>=std::min(std::min(dpbry1,dpbly1),std::min(dptry1,dptly1)))
					&& (std::min(std::min(pbry1,pbly1),std::min(ptry1,ptly1))<=std::max(std::max(dpbry1,dpbly1),std::max(dptry1,dptly1))))
				{
					*pData = static_cast<T>(srcrow);
				}
				/*pSrcAcc->toPixel(srccol, srcrow);
				VERIFYNRV(pSrcAcc.isValid());
				T srcpixel = *reinterpret_cast<T*>(pSrcAcc->getColumn());*/
				//pixelvalue += 1.0;
				//pixelvalue = area*srcpixel;
			}
		}
		//*pData = static_cast<T>(50);
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
	RasterDataDescriptor* pDestDesc = static_cast<RasterDataDescriptor*>(pResultCube->getDataDescriptor());

	/* GET GCPs OF INPUT IMAGE */
	GcpList * GCPs = NULL;		//New GCPList

	std::vector<DataElement*> pGcpLists = pModel->getElements(pSrcAcc1->getAssociatedRasterElement(), TypeConverter::toString<GcpList>());		//Get GCPLists of input image

	if (!pGcpLists.empty())
	{
		QStringList aoiNames("<none>");
		for (std::vector<DataElement*>::iterator it = pGcpLists.begin(); it != pGcpLists.end(); ++it)
		{
			aoiNames << QString::fromStdString((*it)->getName());
		}
		QString aoi = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
			"Select a GCP List", "Select a GCP List for processing", aoiNames);

		if (aoi != "<none>")
		{
			std::string strAoi = aoi.toStdString();
			for (std::vector<DataElement*>::iterator it = pGcpLists.begin(); it != pGcpLists.end(); ++it)
			{
				if ((*it)->getName() == strAoi)
				{
					GCPs = static_cast<GcpList*>(*it);
					break;
				}
			}
			if (GCPs == NULL)
			{
				std::string msg = "Invalid GCPList.";
				pProgress->updateProgress(msg, 0, ERRORS);
				return false;
			}
		}
		else
		{
			std::string msg = "A set of GCPs must be specified.";
			if (pProgress.get() != NULL)
			{
				pProgress->updateProgress(msg, 0, ERRORS);
			}
			return false;
		}
	}

	/*Compensate for increase in resolution of output image */
	std::list<GcpPoint> pNewGcpList = GCPs->getSelectedPoints();

	for (std::list<GcpPoint>::iterator it = (pNewGcpList.begin()); it != pNewGcpList.end(); ++it)
	{
		(*it).mPixel.mX *= (x_out->text().toDouble() / pDesc1->getColumnCount());
		(*it).mPixel.mY *= (y_out->text().toDouble() / pDesc1->getRowCount());
	}

	GcpList* newGCPList = static_cast<GcpList*>(pModel->createElement("Corner coordinates","GcpList",pResultCube.get()));
	newGCPList->addPoints(pNewGcpList);

	pDestDesc->setGeoreferenceDescriptor(pDesc1->getGeoreferenceDescriptor());
	GeoreferenceDescriptor *pDestGeoDesc = pDestDesc->getGeoreferenceDescriptor();
	pDestDesc->getGeoreferenceDescriptor()->setLayerName("GEO_RESULT");
	pDestDesc->getGeoreferenceDescriptor()->setCreateLayer(true);
	pDestDesc->getGeoreferenceDescriptor()->setDisplayLayer(true);
	pDestDesc->getGeoreferenceDescriptor()->setSettingAutoGeoreference(true);

	Service<DesktopServices> pDesktop;

	SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pResultCube->getName(), SPATIAL_DATA_WINDOW));

	SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();
	if (pView == NULL){
		std::string msg = "Unable to create view.";
		pStep->finalize(Message::Failure, msg);
		pProgress->updateProgress(msg, 0, ERRORS);
		return false;
	}

	const std::string &plugInName = pDestGeoDesc->getGeoreferencePlugInName();
	if (!plugInName.empty()){
		ExecutableResource geoPlugIn(plugInName);
		PlugInArgList& argList = geoPlugIn->getInArgList();
		argList.setPlugInArgValue(Executable::DataElementArg(), pResultCube.get());
		argList.setPlugInArgValue(Executable::ProgressArg(), pProgress.get());
		argList.setPlugInArgValueLoose(Executable::ViewArg(), pView);
		if (geoPlugIn->execute() == false)
		{
			std::string message = "Could not georeference the data set.";
			pProgress->updateProgress(message, 0, WARNING);

			pStep->addMessage(message, "app", "A8050A4B-824A-4E60-88E5-729367DEEAD0");
		}
		else
		{
			geoPlugIn.release();
			pStep->finalize(Message::Success);
		}
	}
	else
	{
		std::string message = "A georeference plug-in is not available to georeference the data set.";


		pProgress->updateProgress(message, 0, WARNING);


		pStep->addMessage(message, "app", "44E8D3C8-64C3-44DC-AB65-43F433D69DC8");
	}
	for (unsigned int row = 0; row < pDestDesc->getRowCount(); ++row){ 
		pProgress->updateProgress("Calculating result", row * 100 / pDesc1->getRowCount(), NORMAL);
		if (!pDestAcc.isValid())
		{
			std::string msg = "Unable to access the cube data.";
			pStep->finalize(Message::Failure, msg);
			pProgress->updateProgress(msg, 0, ERRORS);
			return false;
		}
		
		for (unsigned int col = 0; col < pDestDesc->getColumnCount(); ++col)
		{
			switchOnEncoding(pDestDesc->getDataType(), Drizzle, pDestAcc->getColumn(), pDestAcc, pSrcAcc1, row, col, pDestDesc->getRowCount(), pDestDesc->getColumnCount());
			pDestAcc->nextColumn();
		}

		pDestAcc->nextRow();
	}

	pView->setPrimaryRasterElement(pResultCube.get());
	pView->createLayer(RASTER, pResultCube.get());

	pView->createLayer(GCP_LAYER,newGCPList,"Corner Coordinates");

	pResultCube.release();

	pStep->finalize();
	pProgress->updateProgress("Done", 100, NORMAL);
	this->accept();
	return true;
}