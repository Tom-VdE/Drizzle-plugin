/********************************************//*
* 
* @file: Drizzle_GUI.cpp
*
* The information in this file is
* Copyright(c) 2015 Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
***********************************************/

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
#include "Location.h"

#include "AppConfig.h"
#include "AppVerify.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "StringUtilities.h"
#include "drizzle_helper_functions.h"

#include <Qt/QInputDialog.h>
#include <Qt/qgridlayout.h>
#include <Qt/qapplication.h>
#include <Qt/qmessagebox.h>

namespace
{
	template<typename T>
	/**
	* Function which performs the drizzling for one pixel of the destination image.
	*
	* @param pData Typename T to be divided.
	* @param pDestAcc DataAccessor to the destination RasterElement.
	* @param pSrcAcc DataAccessor to the source RasterElement.
	* @param row Current row of the destination RasterElement.
	* @param col Current column of the destination RasterElement.
	* @param rowSize height of the destination RasterElement.
	* @param colSize width of the destination RasterElement.
	* @param drop Percentage of width and height of pixel of the source images which is taken into account (from 0 to 1).
	* @param overlapped Pointer to boolean indicating whether or not current pixel of destination RasterElement overlapped with the source RasterElement.
	*/
	void Drizzle(T* pData, DataAccessor pDestAcc, DataAccessor pSrcAcc, unsigned int row, unsigned int col, unsigned int rowSize, unsigned int colSize, double drop, bool* overlapped)
	{
		std::vector<LocationType> ipoints;	//initialise vector holding point of interest

		LocationType *tllt = new LocationType(0,0);					//top left pixel of destination image
		LocationType *bllt = new LocationType(0,rowSize);			//bottom left pixel of destination image
		LocationType *brlt = new LocationType(colSize,rowSize);		//bottom right pixel of destination image
		LocationType *trlt = new LocationType(colSize,0);			//top right pixel of destination image

		LocationType desgeo1 = pDestAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*tllt);		//coordinates of top left pixel of destination image
		LocationType desgeo2 = pDestAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*bllt);		//coordinates of bottom left pixel of destination image
		LocationType desgeo3 = pDestAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*trlt);		//coordinates of bottom right pixel of destination image
		LocationType desgeo4 = pDestAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*brlt);		//coordinates of top right pixel of destination image

		double dtlx1 = desgeo1.mX;			//x coordinate of top left pixel of destination image
		double dtly1 = desgeo1.mY;			//y coordinate of top left pixel of destination image
		double dblx1 = desgeo2.mX;			//x coordinate of top right pixel of destination image
		double dbly1 = desgeo2.mY;			//y coordinate of top right pixel of destination image
		double dtrx1 = desgeo3.mX;			//x coordinate of bottom left pixel of destination image
		double dtry1 = desgeo3.mY;			//y coordinate of bottom left pixel of destination image
		double dbrx1 = desgeo4.mX;			//x coordinate of bottom right pixel of destination image
		double dbry1 = desgeo4.mY;			//y coordinate of bottom right pixel of destination image

		//Delete temporary Locationtypes
		delete tllt;
		delete bllt;
		delete trlt;
		delete brlt;

		double ddtx1 = dtrx1 - dtlx1;		//difference in x coordinate over top of image
		double ddty1 = dtry1 - dtly1;		//difference in y coordinate over top of image
		double ddlx1 = dblx1 - dtlx1;		//difference in x coordinate over left side of image
		double ddly1 = dbly1 - dtly1;		//difference in y coordinate over left side of image
		double ddbx1 = dbrx1 - dblx1;		//difference in x coordinate over bottom of image
		double ddby1 = dbry1 - dbly1;		//difference in y coordinate over bottom of image
		double ddrx1 = dbrx1 - dtrx1;		//difference in x coordinate over right side of image
		double ddry1 = dbry1 - dtry1;		//difference in y coordinate over right side of image

		double dptlx1 = dtlx1 + ((((ddbx1-ddtx1)/rowSize)*double(row) + ddtx1)/double(colSize))*double(col) + ((((ddrx1-ddlx1)/colSize)*double(col) + ddlx1)/double(rowSize))*double(row);					//top left x coordinate of destination pixel
		double dptly1 = dtly1 + ((((ddby1-ddty1)/rowSize)*double(row) + ddty1)/double(colSize))*double(col) + ((((ddry1-ddly1)/colSize)*double(col) + ddly1)/double(rowSize))*double(row);					//top left y coordinate of destination pixel
		double dpblx1 = dtlx1 + ((((ddbx1-ddtx1)/rowSize)*double(row+1) + ddtx1)/double(colSize))*double(col) + ((((ddrx1-ddlx1)/colSize)*double(col) + ddlx1)/double(rowSize))*double(row+1);				//top right x coordinate of destination pixel
		double dpbly1 = dtly1 + ((((ddby1-ddty1)/rowSize)*double(row+1) + ddty1)/double(colSize))*double(col) + ((((ddry1-ddly1)/colSize)*double(col) + ddly1)/double(rowSize))*double(row+1);				//top right y coordinate of destination pixel
		double dptrx1 = dtlx1 + ((((ddbx1-ddtx1)/rowSize)*double(row) + ddtx1)/double(colSize))*double(col+1) + ((((ddrx1-ddlx1)/colSize)*double(col+1) + ddlx1)/double(rowSize))*double(row);				//bottom left x coordinate of destination pixel
		double dptry1 = dtly1 + ((((ddby1-ddty1)/rowSize)*double(row) + ddty1)/double(colSize))*double(col+1) + ((((ddry1-ddly1)/colSize)*double(col+1) + ddly1)/double(rowSize))*double(row);				//bottom left y coordinate of destination pixel
		double dpbrx1 = dtlx1 + ((((ddbx1-ddtx1)/rowSize)*double(row+1) + ddtx1)/double(colSize))*double(col+1) + ((((ddrx1-ddlx1)/colSize)*double(col+1) + ddlx1)/double(rowSize))*double(row+1);			//bottom right x coordinate of destination pixel
		double dpbry1 = dtly1 + ((((ddby1-ddty1)/rowSize)*double(row+1) + ddty1)/double(colSize))*double(col+1) + ((((ddry1-ddly1)/colSize)*double(col+1) + ddly1)/double(rowSize))*double(row+1);			//bottom right y coordinate of destination pixel

		//Equations of four lines (top, bottom, left, right) for one pixel: Y = MX + C
		double dtm = (dptly1 - dptry1)/(dptlx1 - dptrx1);
		double dtc = dptry1 - dtm*dptrx1;
		double dbm = (dpbly1 - dpbry1)/(dpblx1 - dpbrx1);
		double dbc = dpbry1 - dbm*dpbrx1;
		double dlm = (dptly1 - dpbly1)/(dptlx1 - dpblx1);
		double dlc = dpbly1 - dlm*dpblx1;
		double drm = (dptry1 - dpbry1)/(dptrx1 - dpbrx1);
		double drc = dpbry1 - drm*dpbrx1;

		int srcrowSize = dynamic_cast<const RasterDataDescriptor*>(pSrcAcc->getAssociatedRasterElement()->getDataDescriptor())->getRows().size();		//height of source image
		int srccolSize = dynamic_cast<const RasterDataDescriptor*>(pSrcAcc->getAssociatedRasterElement()->getDataDescriptor())->getColumns().size();	//width of source image

		LocationType *tlsrclt = new LocationType(dptlx1,dptly1);		//top left pixel of source image
		LocationType *blsrclt = new LocationType(dpblx1,dpbly1);		//bottom left pixel of source image
		LocationType *trsrclt = new LocationType(dptrx1,dptry1);		//top right pixel of source image
		LocationType *brsrclt = new LocationType(dpbrx1,dpbry1);		//bottom right pixel of source image

		double tlsrccol = ((pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*tlsrclt).mX) > 0) ? pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*tlsrclt).mX : 0;		//top left x coordinate of destination pixel wrt source image
		double tlsrcrow = ((pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*tlsrclt).mY) > 0) ? pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*tlsrclt).mY : 0;		//top left y coordinate of destination pixel wrt source image
		double trsrccol = ((pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*trsrclt).mX) > 0) ? pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*trsrclt).mX : 0;		//top right x coordinate of destination pixel wrt source image
		double trsrcrow = ((pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*trsrclt).mY) > 0) ? pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*trsrclt).mY : 0;		//top right y coordinate of destination pixel wrt source image
		double brsrccol = ((pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*brsrclt).mX) > 0) ? pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*brsrclt).mX : 0;		//bottom right x coordinate of destination pixel wrt source image
		double brsrcrow = ((pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*brsrclt).mY) > 0) ? pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*brsrclt).mY : 0;		//bottom right y coordinate of destination pixel wrt source image
		double blsrccol = ((pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*blsrclt).mX) > 0) ? pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*blsrclt).mX : 0;		//bottom left x coordinate of destination pixel wrt source image
		double blsrcrow = ((pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*blsrclt).mY) > 0) ? pSrcAcc->getAssociatedRasterElement()->convertGeocoordToPixel(*blsrclt).mY : 0;		//bottom left y coordinate of destination pixel wrt source image

		//Delete temporary Locationtypes
		delete tlsrclt;
		delete blsrclt;
		delete trsrclt;
		delete brsrclt;

		tlsrclt = new LocationType(0,0);					//top left pixel of source image
		blsrclt = new LocationType(0,srcrowSize);			//bottom left pixel of source image
		trsrclt = new LocationType(srccolSize,0);			//top right pixel of source image
		brsrclt = new LocationType(srccolSize,srcrowSize);	//bottom right pixel of source image

		LocationType geo1 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*tlsrclt);		//coordinates of top left pixel of source image
		LocationType geo2 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*blsrclt);		//coordinates of bottom left pixel of source image
		LocationType geo3 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*trsrclt);		//coordinates of top right pixel of source image
		LocationType geo4 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*brsrclt);		//coordinates of bottom right pixel of source image

		//Delete temporary Locationtypes
		delete tlsrclt;
		delete blsrclt;
		delete trsrclt;
		delete brsrclt;

		double tlx1 = geo1.mX;			//x coordinate of top left pixel of source image
		double tly1 = geo1.mY;			//y coordinate of top left pixel of source image
		double blx1 = geo2.mX;			//x coordinate of top right pixel of source image
		double bly1 = geo2.mY;			//y coordinate of top right pixel of source image 
		double trx1 = geo3.mX;			//x coordinate of bottom left pixel of source image 
		double try1 = geo3.mY;			//y coordinate of bottom left pixel of source image 
		double brx1 = geo4.mX;			//x coordinate of bottom right pixel of source image 
		double bry1 = geo4.mY;			//y coordinate of bottom right pixel of source image 

		double dtx1 = trx1 - tlx1;		//difference in x coordinate over top of source image 
		double dty1 = try1 - tly1;		//difference in y coordinate over top of source image
		double dlx1 = blx1 - tlx1;		//difference in x coordinate over left side of source image
		double dly1 = bly1 - tly1;		//difference in y coordinate over left side of source image
		double dbx1 = brx1 - blx1;		//difference in x coordinate over bottom of source image
		double dby1 = bry1 - bly1;		//difference in y coordinate over bottom of source image
		double drx1 = brx1 - trx1;		//difference in x coordinate over right side of source image
		double dry1 = bry1 - try1;		//difference in y coordinate over right side of source image
		
		//Get upper and lower bounds on searchable area for pixels of the source image
		int rtlsrccol = int(std::floor(tlsrccol));
		int rtlsrcrow = int(std::floor(tlsrcrow));
		int rtrsrccol = int(std::floor(trsrccol));
		int rtrsrcrow = int(std::ceil(trsrcrow));
		int rbrsrccol = int(std::ceil(brsrccol));
		int rbrsrcrow = int(std::ceil(brsrcrow));
		int rblsrccol = int(std::ceil(blsrccol));
		int rblsrcrow = int(std::floor(blsrcrow));

		for(int srcrow = std::min(std::min(rtlsrcrow,rtrsrcrow),std::min(rblsrcrow,rbrsrcrow)); srcrow <= std::max(std::max(rtlsrcrow,rtrsrcrow),std::max(rblsrcrow,rbrsrcrow)); srcrow++){
			for(int srccol = std::min(std::min(rtlsrccol,rtrsrccol),std::min(rblsrccol,rbrsrccol)); srccol <= std::max(std::max(rtlsrccol,rtrsrccol),std::max(rblsrccol,rbrsrccol)); srccol++){
				if(srccol < srccolSize && srcrow < srcrowSize){ 
					ipoints.clear();		//Clear interest points vector

					double ddrop = (1-drop)/2;

					double ptlx1 = tlx1 + ((((dbx1-dtx1)/srcrowSize)*double(srcrow + ddrop) + dtx1)/double(srccolSize))*double(srccol + ddrop) + ((((drx1-dlx1)/srccolSize)*double(srccol + ddrop) + dlx1)/double(srcrowSize))*double(srcrow + ddrop);					//top left x coordinate of source pixel 
					double ptly1 = tly1 + ((((dby1-dty1)/srcrowSize)*double(srcrow + ddrop) + dty1)/double(srccolSize))*double(srccol + ddrop) + ((((dry1-dly1)/srccolSize)*double(srccol + ddrop) + dly1)/double(srcrowSize))*double(srcrow + ddrop);					//top left y coordinate of source pixel
					double pblx1 = tlx1 + ((((dbx1-dtx1)/srcrowSize)*double(srcrow+1 - ddrop) + dtx1)/double(srccolSize))*double(srccol + ddrop) + ((((drx1-dlx1)/srccolSize)*double(srccol + ddrop) + dlx1)/double(srcrowSize))*double(srcrow+1 - ddrop);				//top right x coordinate of source pixel
					double pbly1 = tly1 + ((((dby1-dty1)/srcrowSize)*double(srcrow+1 - ddrop) + dty1)/double(srccolSize))*double(srccol + ddrop) + ((((dry1-dly1)/srccolSize)*double(srccol + ddrop) + dly1)/double(srcrowSize))*double(srcrow+1 - ddrop);				//top right y coordinate of source pixel
					double ptrx1 = tlx1 + ((((dbx1-dtx1)/srcrowSize)*double(srcrow + ddrop) + dtx1)/double(srccolSize))*double(srccol+1 - ddrop) + ((((drx1-dlx1)/srccolSize)*double(srccol+1 - ddrop) + dlx1)/double(srcrowSize))*double(srcrow + ddrop);				//bottom left x coordinate of source pixel
					double ptry1 = tly1 + ((((dby1-dty1)/srcrowSize)*double(srcrow + ddrop) + dty1)/double(srccolSize))*double(srccol+1 - ddrop) + ((((dry1-dly1)/srccolSize)*double(srccol+1 - ddrop) + dly1)/double(srcrowSize))*double(srcrow + ddrop);				//bottom left y coordinate of source pixel
					double pbrx1 = tlx1 + ((((dbx1-dtx1)/srcrowSize)*double(srcrow+1 - ddrop) + dtx1)/double(srccolSize))*double(srccol+1 - ddrop) + ((((drx1-dlx1)/srccolSize)*double(srccol+1 - ddrop) + dlx1)/double(srcrowSize))*double(srcrow+1 - ddrop);			//bottom right x coordinate of source pixel
					double pbry1 = tly1 + ((((dby1-dty1)/srcrowSize)*double(srcrow+1 - ddrop) + dty1)/double(srccolSize))*double(srccol+1 - ddrop) + ((((dry1-dly1)/srccolSize)*double(srccol+1 - ddrop) + dly1)/double(srcrowSize))*double(srcrow+1 - ddrop);			//bottom right y coordinate of source pixel

					//Check whether input and output pixel can overlap
					if((std::max(std::max(pbrx1,pblx1),std::max(ptrx1,ptlx1))>=std::min(std::min(dpbrx1,dpblx1),std::min(dptrx1,dptlx1)))
						&& (std::min(std::min(pbrx1,pblx1),std::min(ptrx1,ptlx1))<=std::max(std::max(dpbrx1,dpblx1),std::max(dptrx1,dptlx1)))
						&& (std::max(std::max(pbry1,pbly1),std::max(ptry1,ptly1))>=std::min(std::min(dpbry1,dpbly1),std::min(dptry1,dptly1)))
						&& (std::min(std::min(pbry1,pbly1),std::min(ptry1,ptly1))<=std::max(std::max(dpbry1,dpbly1),std::max(dptry1,dptly1))))
					{
						//SUTHERLAND-HODGMAN POLYGON CLIPPING

						//Use of geographical positions -> limited resolution of double
						/*std::vector<LocationType> subject;
						subject.push_back(*(new LocationType(ptlx1,ptly1)));
						subject.push_back(*(new LocationType(pblx1,pbly1)));
						subject.push_back(*(new LocationType(pbrx1,pbry1)));
						subject.push_back(*(new LocationType(ptrx1,ptry1)));
						std::vector<LocationType> clip;
						clip.push_back(*(new LocationType(dptlx1,dptly1)));
						clip.push_back(*(new LocationType(dpblx1,dpbly1)));
						clip.push_back(*(new LocationType(dpbrx1,dpbry1)));
						clip.push_back(*(new LocationType(dptrx1,dptry1)));*/

						//Use relative positions wrt source image instead of geographical positions due to limited resolution of double.
						std::vector<LocationType> subject;

						LocationType *tlsubjectlt = new LocationType(srccol + ddrop,srcrow + ddrop);
						LocationType *blsubjectlt = new LocationType(srccol + ddrop,srcrow+1 - ddrop);
						LocationType *brsubjectlt = new LocationType(srccol+1 - ddrop,srcrow+1  - ddrop);
						LocationType *trsubjectlt = new LocationType(srccol+1 - ddrop,srcrow + ddrop);

						subject.push_back(*tlsubjectlt);
						subject.push_back(*blsubjectlt);
						subject.push_back(*brsubjectlt);
						subject.push_back(*trsubjectlt);

						std::vector<LocationType> clip;

						LocationType *tlcliplt = new LocationType(tlsrccol,tlsrcrow);
						LocationType *blcliplt = new LocationType(blsrccol,blsrcrow);
						LocationType *brcliplt = new LocationType(brsrccol,brsrcrow);
						LocationType *trcliplt = new LocationType(trsrccol,trsrcrow);

						clip.push_back(*tlcliplt);
						clip.push_back(*blcliplt);
						clip.push_back(*brcliplt);
						clip.push_back(*trcliplt);
						
						std::vector<LocationType> p1;
						std::vector<LocationType> tmp;

						int dir = int(drizzle_helper_functions::left_of(clip[0], clip[1], clip[2]));
						
						drizzle_helper_functions::poly_edge_clip(subject, clip[clip.size()-1], clip[0], dir, &ipoints);

						for (int i = 0; i < clip.size()-1; i++) {
							tmp = ipoints; 
							ipoints = p1; 
							p1 = tmp;

							if(p1.size() == 0) {
								ipoints.clear();
								break;
							}
							drizzle_helper_functions::poly_edge_clip(p1, clip[i], clip[i+1], dir, &ipoints);
						}

						p1.clear();
						tmp.clear();

						if(ipoints.size() > 0){
							//CALCULATION OF AREA OF OVERLAP
							double s1 = 0;
							double s2 = 0;
							double area = 0;
							for (unsigned int i = 0; i < ipoints.size(); i++){
								s1 += ipoints.at(i).mY*ipoints.at((i+1)%ipoints.size()).mX;
								s2 += ipoints.at(i).mX*ipoints.at((i+1)%ipoints.size()).mY;
							}

							area = (s1-s2)/2.0;

							//Total area for geographical coordinate
							//double totalarea = (((ptly1*pblx1)+(pbly1*pbrx1)+(pbry1*ptrx1)+(ptry1*ptlx1))-((ptlx1*pbly1)+(pblx1*pbry1)+(pbrx1*ptry1)+(ptrx1*ptly1)))/2;

							//Total area for relative coordinate
							//double totalarea = (((tlsrcrow*blsrcrow)+(blsrcrow*brsrccol)+(brsrcrow*trsrccol)+(trsrcrow*tlsrccol))-((tlsrccol*blsrcrow)+(blsrccol*brsrcrow)+(brsrccol*trsrcrow)+(trsrccol*tlsrcrow)))/2;

							//Get source pixel value
							pSrcAcc->toPixel(srcrow, srccol);
							VERIFYNRV(pSrcAcc.isValid());
							T srcpixel = *reinterpret_cast<T*>(pSrcAcc->getColumn());

							//Add weighted source pixel to destination pixel
							*pData += static_cast<T>(area*srcpixel);

							//Set overlapped true to be able to determine the number of overlapping images for each destination pixel
							*overlapped=true;

							//Delete remainder of Locationtypes
							delete tlsubjectlt;
							delete blsubjectlt;
							delete trsubjectlt;
							delete brsubjectlt;
							delete tlcliplt;
							delete blcliplt;
							delete trcliplt;
							delete brcliplt;

							//ipoints.clear();
							//clip.clear();
							//subject.clear();
							//p1.clear();
							//tmp.clear();
						}
					}
				}
			}
		}
		//ipoints.clear();
	}
};

namespace
{
template<typename T>
	/**
	* Function to divide a pixel value of a rasterelement by a given integer.
	*
	* @param pData Typename T to be divided.
	* @param num_overlap_images Integer to be divided with.
	*/
	void Divide(T* pData, int num_overlap_images)
	{
		*pData = static_cast<T>(*pData/num_overlap_images);
	}
};

Drizzle_GUI::Drizzle_GUI(QWidget* Parent): QDialog(Parent)
{
	this->setWindowTitle("Drizzle algorithm");
	setModal(FALSE);

	//WIDGETS 
	Apply = new QPushButton( "applyButton", this );
	Apply->setText("Drizzle!");

	Cancel = new QPushButton( "cancelButton", this );
	Cancel->setText("Cancel");

	Image1 = new QLabel("Base image (determines geographical span of output image)", this);
	Image2 = new QLabel("Additional input images", this);

	Size1 = new QLabel("Size:", this);
	Size2 = new QLabel("Size:", this);
	Geo1 = new QLabel("Coordinates:", this);
	Geo2 = new QLabel("Coordinates:", this);

	Rasterlist1 = new QComboBox(this);
	Rasterlist2 = new QListWidget(this);

	x_out_text = new QLabel("Output size (x)");
	y_out_text = new QLabel("Output size (y)");
	dropsize_text = new QLabel("Dropsize");
	x_out = new QLineEdit(this);
	y_out = new QLineEdit(this);
	dropsize = new QLineEdit(this);

	//LAYOUT
	QGridLayout* pLayout = new QGridLayout(this);

	pLayout->addWidget( Rasterlist1, 1, 0, 1, 3 );
	pLayout->addWidget( Rasterlist2, 1, 4, 4, 3 );

	pLayout->addWidget( Image1, 0, 0, 1, 3);
	pLayout->addWidget( Image2, 0, 4, 1, 3);
	pLayout->addWidget( Size1, 2, 0, 1, 3);
	pLayout->addWidget( Size2, 4, 4, 1, 3);
	pLayout->addWidget( Geo1, 3, 0, 1, 3);
	pLayout->addWidget( Geo2, 5, 4, 1, 3);

	pLayout->addWidget( x_out_text,4,0);
	pLayout->addWidget( y_out_text,4,1);
	pLayout->addWidget( dropsize_text,4,2);
	pLayout->addWidget( x_out,5,0);
	pLayout->addWidget( y_out,5,1);
	pLayout->addWidget( dropsize,5,2);

	pLayout->addWidget(Cancel, 6, 4,1,3);
	pLayout->addWidget(Apply, 6, 0,1,3);

	//Call init() for the necessary initialisations
	init();
}

Drizzle_GUI::~Drizzle_GUI()
{

}

void Drizzle_GUI::init(){
	
	//Initialize buttons
	connect(Cancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(Apply, SIGNAL(clicked()), this, SLOT(PerformDrizzle()));
	connect(Rasterlist1, SIGNAL(currentIndexChanged(int)), this, SLOT(updateInfo1()));
	connect(Rasterlist2, SIGNAL(currentRowChanged(int)), this, SLOT(updateInfo2()));

	//Get RasterElements
	Service<ModelServices> Model;
	RasterElements = Model->getElementNames("RasterElement");

	//Add RasterElements to ComboBox and ListWidget
	for (unsigned int i = 0; i < RasterElements.size(); i++)
	{
		Rasterlist1->insertItem(i, QString::fromStdString(RasterElements[i]));
		QListWidgetItem *newitem = new QListWidgetItem(QString::fromStdString(RasterElements[i]), Rasterlist2);
		newitem->setFlags(newitem->flags() | Qt::ItemIsUserCheckable);
		newitem->setCheckState(Qt::Unchecked);
	}

	//Initialise ComboBox's size
	Rasterlist1->setMinimumWidth(Rasterlist1->sizeHint().rwidth());
	Rasterlist1->setMaximumWidth(Rasterlist1->sizeHint().rwidth());
	
	//Initialise ListWidget
	Rasterlist2->setCurrentRow(0);
	Rasterlist2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	Rasterlist2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	Rasterlist2->setFixedSize(Rasterlist2->sizeHintForColumn(0) + 2 * Rasterlist2->frameWidth(), Rasterlist2->sizeHintForRow(0) * Rasterlist2->count() + 2 * Rasterlist2->frameWidth());
	
	//Initialise size of image 1
	const RasterDataDescriptor *Des1 = dynamic_cast<const RasterDataDescriptor*>(Model->getElement(RasterElements.at(0),"",NULL)->getDataDescriptor());
	const std::vector<DimensionDescriptor>& Rows = Des1->getRows();
	const std::vector<DimensionDescriptor>& Columns = Des1->getColumns();
	Size1->setText("Size:\t"+ QString::number(Columns.size()) + "x" + QString::number(Rows.size()));

	//Fix size of GUI
	this->layout()->setSizeConstraint( QLayout::SetFixedSize );
}

void Drizzle_GUI::updateInfo1(){
	Service<ModelServices> Model;
	//Get size information
	const RasterDataDescriptor *Des = dynamic_cast<const RasterDataDescriptor*>(Model->getElement(RasterElements.at(Rasterlist1->currentIndex()),"",NULL)->getDataDescriptor());
	const std::vector<DimensionDescriptor>& Rows = Des->getRows();
	const std::vector<DimensionDescriptor>& Columns = Des->getColumns();
	Size1->setText("Size:\t"+ QString::number(Columns.size()) + "x" + QString::number(Rows.size()));
	//Get geo information
	FactoryResource<DataRequest> pRequest;
	DataAccessor pSrcAcc = dynamic_cast<RasterElement*>(Model->getElement(RasterElements.at(Rasterlist1->currentIndex()),"",NULL ))->getDataAccessor(pRequest.release());
	LocationType geo1 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,0)));
	LocationType geo2 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getRows().size()-1,0)));
	LocationType geo3 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,Des->getColumns().size()-1)));
	LocationType geo4 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getRows().size()-1,Des->getColumns().size()-1)));
	Geo1->setText("Coordinates:\t(" + QString::number(geo1.mX) + "," + QString::number(geo1.mY) + "," + QString::number(geo1.mZ)+")\t(" + QString::number(geo3.mX) + "," + QString::number(geo3.mY) + "," + QString::number(geo3.mZ)+") \n" +
		"\n\t(" + QString::number(geo2.mX) + "," + QString::number(geo2.mY) + "," + QString::number(geo2.mZ)+")\t(" + QString::number(geo4.mX) + "," + QString::number(geo4.mY) + "," + QString::number(geo4.mZ)+")");
}

void Drizzle_GUI::updateInfo2(){
	Service<ModelServices> Model;
	//Get size information
	const RasterDataDescriptor *Des = dynamic_cast<const RasterDataDescriptor*>(Model->getElement(RasterElements.at(Rasterlist2->currentRow()),"",NULL)->getDataDescriptor());
	const std::vector<DimensionDescriptor>& Rows = Des->getRows();
	const std::vector<DimensionDescriptor>& Columns = Des->getColumns();
	Size2->setText("Size:\t"+ QString::number(Columns.size()) + "x" + QString::number(Rows.size()));
	//Get geo information
	FactoryResource<DataRequest> pRequest;
	DataAccessor pSrcAcc = dynamic_cast<RasterElement*>(Model->getElement(RasterElements.at(Rasterlist2->currentRow()),"",NULL ))->getDataAccessor(pRequest.release());
	LocationType geo1 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,0)));
	LocationType geo2 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getRows().size()-1,0)));
	LocationType geo3 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(0,Des->getColumns().size()-1)));
	LocationType geo4 = pSrcAcc->getAssociatedRasterElement()->convertPixelToGeocoord(*(new LocationType(Des->getRows().size()-1,Des->getColumns().size()-1)));
	Geo2->setText("Coordinates:\t(" + QString::number(geo1.mX) + "," + QString::number(geo1.mY) + "," + QString::number(geo1.mZ)+")\t(" + QString::number(geo3.mX) + "," + QString::number(geo3.mY) + "," + QString::number(geo3.mZ)+") \n" +
		"\n\t(" + QString::number(geo2.mX) + "," + QString::number(geo2.mY) + "," + QString::number(geo2.mZ)+")\t(" + QString::number(geo4.mX) + "," + QString::number(geo4.mY) + "," + QString::number(geo4.mZ)+")");
}

void Drizzle_GUI::closeGUI(){
	this->reject();
}

bool Drizzle_GUI::PerformDrizzle(){
	Service<ModelServices> pModel;
	StepResource pStep("Drizzle", "app", "4539C009-F756-41A4-A94D-9867C0FF3B87");
	ProgressResource pProgress("ProgressBar");

	//Get base RasterElement from the ComboBox
	RasterElement *image1 =  dynamic_cast<RasterElement*>(pModel->getElement(RasterElements.at(Rasterlist1->currentIndex()),"",NULL ));
	std::vector<RasterElement*> images;


	//Get all other input RasterElement from the ListWidget
	for (int i=0; i < RasterElements.size();i++){
		if((Rasterlist2->item(i)->checkState())){
			if(QString::fromStdString(RasterElements[Rasterlist1->currentIndex()]) == Rasterlist2->item(i)->text()){
				pProgress->updateProgress("Can't select the same image twice.", 100, ERRORS);
				return false;
			}
			else{
				images.push_back(dynamic_cast<RasterElement*>(pModel->getElement(RasterElements.at(i),"",NULL )));
			}
		}
	}

	//Check image import
	if (image1 == NULL || images.size() == 0)
	{				
		pProgress->updateProgress("Image import failed", 100, ERRORS);
		return false;	
	}
	pProgress->updateProgress("Image import succesfull", 0, NORMAL);
	
	//Get RasterDataDescriptor of the base image
	RasterDataDescriptor* pDesc1 = static_cast<RasterDataDescriptor*>(image1->getDataDescriptor());

	//Check whether base image is complexvalued
	if (pDesc1->getDataType() == INT4SCOMPLEX || pDesc1->getDataType() == FLT8COMPLEX)
	{
		std::string msg = "Drizzle cannot be performed on complex types.";
		pStep->finalize(Message::Failure, msg);
		return false;
	}

	//Get RasterDataDescriptors for all other input images
	std::vector<RasterDataDescriptor*> pDesc;
	for (std::vector<RasterElement*>::iterator it = images.begin(); it != images.end(); ++it){

		//Check georeferencing
		if(!image1->isGeoreferenced() || !(*it)->isGeoreferenced()){
			pProgress->updateProgress("Images are not georeferenced", 100, ERRORS);
			return false;	
		}
		//Check whether input images are complexvalued
		if ((static_cast<RasterDataDescriptor*>((*it)->getDataDescriptor()))->getDataType() == INT4SCOMPLEX || (static_cast<RasterDataDescriptor*>((*it)->getDataDescriptor()))->getDataType() == FLT8COMPLEX){
			std::string msg = "Drizzle cannot be performed on complex types.";
			pStep->finalize(Message::Failure, msg);
			return false;
		}
		else{
			pDesc.push_back(static_cast<RasterDataDescriptor*>((*it)->getDataDescriptor()));
		}
	}
	//Get DataAccessor of base image
	FactoryResource<DataRequest> pRequest1;
	DataAccessor pSrcAcc1 = image1->getDataAccessor(pRequest1.release());
	std::vector<DataAccessor> pSrcAcc;
	
	//Get DataAccessors of other input images
	for (std::vector<RasterElement*>::iterator it = images.begin(); it != images.end(); ++it){
		FactoryResource<DataRequest> pRequest;
		pSrcAcc.push_back((*it)->getDataAccessor(pRequest.release()));
	}

	//Check whether output width and height are filled in
	if(x_out->text().isNull() || y_out->text().isNull() || x_out->text().isEmpty() || y_out->text().isEmpty())
	{
		pProgress->updateProgress("No output size specified.", 100, ERRORS);
		return false;
	}

	//Check whether dropsize is filled in and valid
	if(dropsize->text().isNull() || dropsize->text().isEmpty() || dropsize->text().toDouble() < 0 || dropsize->text().toDouble() > 1)
	{
		pProgress->updateProgress("No valid dropsize specified.", 100, ERRORS);
		return false;
	}

	//Create the output RasterElement
	ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(image1->getName() + "_Drizzled", y_out->text().toDouble(), x_out->text().toDouble(), pDesc1->getDataType()));

	//Check whether creation of new RasterElement succeeded
	if (pResultCube.get() == NULL){
		std::string msg = "A raster cube could not be created.";
		pStep->finalize(Message::Failure, msg);
		return false;
	}

	//Get DataAccessor and RasterDataDescriptor of output RasterElement
	FactoryResource<DataRequest> pResultRequest;
	pResultRequest->setWritable(true);
	DataAccessor pDestAcc = pResultCube->getDataAccessor(pResultRequest.release());
	RasterDataDescriptor* pDestDesc = static_cast<RasterDataDescriptor*>(pResultCube->getDataDescriptor());

	//Get GCPs of input image via GUI
	GcpList * GCPs = NULL;

	std::vector<DataElement*> pGcpLists = pModel->getElements(pSrcAcc1->getAssociatedRasterElement(), TypeConverter::toString<GcpList>());

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

	//Compensation of GCPs for increase in resolution of output image 
	std::list<GcpPoint> pNewGcpList = GCPs->getSelectedPoints();

	for (std::list<GcpPoint>::iterator it = (pNewGcpList.begin()); it != pNewGcpList.end(); ++it)
	{
		(*it).mPixel.mX *= (x_out->text().toDouble() / pDesc1->getColumnCount());
		(*it).mPixel.mY *= (y_out->text().toDouble() / pDesc1->getRowCount());
	}

	//Create new GCP list for output RasterElement
	GcpList* newGCPList = static_cast<GcpList*>(pModel->createElement("Corner coordinates","GcpList",pResultCube.get()));
	//Add compensated GCPs to new list
	newGCPList->addPoints(pNewGcpList);

	//Set GeoreferenceDescriptor for output RasterElement
	pDestDesc->setGeoreferenceDescriptor(pDesc1->getGeoreferenceDescriptor());
	GeoreferenceDescriptor *pDestGeoDesc = pDestDesc->getGeoreferenceDescriptor();

	//Create view
	Service<DesktopServices> pDesktop;
	SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pResultCube->getName(), SPATIAL_DATA_WINDOW));
	SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();

	//Check whether creation of view was successfull
	if (pView == NULL){
		std::string msg = "Unable to create view.";
		pStep->finalize(Message::Failure, msg);
		pProgress->updateProgress(msg, 0, ERRORS);
		return false;
	}

	//Georeference the output image using the Georeference Plugin
	const std::string &plugInName = pDestGeoDesc->getGeoreferencePlugInName();
	pStep->addProperty("PluginName", plugInName);
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


	bool overlapped = false;
	int num_overlap_images;
	double drop = dropsize->text().toDouble();
	
	//Set destination image to top left pixel.
	pDestAcc->toPixel(0,0);
	//Drizzle images onto destination image.
	for (unsigned int row = 0; row < pDestDesc->getRowCount(); ++row){ 
		pProgress->updateProgress("Calculating result", row * 100 / pDestDesc->getRowCount(), NORMAL);
		if (!pDestAcc.isValid())
		{
			std::string msg = "Unable to access the cube data.";
			pStep->finalize(Message::Failure, msg);
			pProgress->updateProgress(msg, 0, ERRORS);
			return false;
		}

		for (unsigned int col = 0; col < pDestDesc->getColumnCount(); ++col)
		{
			//Drizzle base image
			switchOnEncoding(pDestDesc->getDataType(), Drizzle, pDestAcc->getColumn(), pDestAcc, pSrcAcc1, row, col, pDestDesc->getRowCount(), pDestDesc->getColumnCount(), drop, &overlapped);
			num_overlap_images=1;
			//Drizzle other images
			for (int i=0; i<images.size();i++){
				overlapped=false;
				switchOnEncoding(pDestDesc->getDataType(), Drizzle, pDestAcc->getColumn(), pDestAcc, pSrcAcc[i], row, col, pDestDesc->getRowCount(), pDestDesc->getColumnCount(), drop, &overlapped);
				if(overlapped) num_overlap_images++;
			}
			//Divide output pixel by the number of input image overlapping with that particular pixel 
			switchOnEncoding(pDestDesc->getDataType(), Divide, pDestAcc->getColumn(), num_overlap_images);
			pDestAcc->nextColumn();
		}
		pDestAcc->nextRow();
	}

	//Clear vectors
	images.clear();
	pDesc.clear();
	pSrcAcc.clear();
	pGcpLists.clear();

	//Output destination RasterElement
	pView->setPrimaryRasterElement(pResultCube.get());
	pView->createLayer(RASTER, pResultCube.get());
	pView->createLayer(GCP_LAYER,newGCPList,"Corner Coordinates");
	pResultCube.release();

	pStep->finalize();
	pProgress->updateProgress("Done", 100, NORMAL);
	this->accept();
	return true;
}