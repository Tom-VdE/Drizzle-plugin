/********************************************//*
* 
* @file: DrizzleVideo_GUI.cpp
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
#include "DrizzleVideo_GUI.h"
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
#include <Qt/qfiledialog.h>
#include <Qt/qdir.h>

#include <stdio.h>


#include <opencv\cv.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\opencv_modules.hpp>
#include <opencv2\nonfree\nonfree.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace cv;

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
	* @param num_overlap_images Pointer to double which holds the number of source images overlapping with the destination pixel.
	*/
	void DrizzleVideo(T* pData, DataAccessor pDestAcc, DataAccessor pSrcAcc, unsigned int row, unsigned int col, unsigned int rowSize, unsigned int colSize, double drop, double* num_overlap_images)
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

		//Set indicator for whether current output pixel and input image overlapped to FALSE
		bool overlapped = false;
		//Set temporary output pixel value to zero
		unsigned long temp = 0;

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

							//Total area for geographical coordinate:
							//double totalarea = (((ptly1*pblx1)+(pbly1*pbrx1)+(pbry1*ptrx1)+(ptry1*ptlx1))-((ptlx1*pbly1)+(pblx1*pbry1)+(pbrx1*ptry1)+(ptrx1*ptly1)))/2;

							//Total area for relative coordinate
							//double totalarea = (((tlsrcrow*blsrcrow)+(blsrcrow*brsrccol)+(brsrcrow*trsrccol)+(trsrcrow*tlsrccol))-((tlsrccol*blsrcrow)+(blsrccol*brsrcrow)+(brsrccol*trsrcrow)+(trsrccol*tlsrcrow)))/2;

							//Get source pixel value
							pSrcAcc->toPixel(srcrow, srccol);
							VERIFYNRV(pSrcAcc.isValid());
							T srcpixel = *reinterpret_cast<T*>(pSrcAcc->getColumn());

							//Add weighted source pixel to temporary destination pixel
							temp += area*srcpixel;

							//Set overlapped true to be able to determine the number of overlapping images for each destination pixel
							overlapped = true;

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
		//Divide output pixel value by total number of overlapping input images with that particular output pixel
		if(*num_overlap_images!=0){
			*pData = static_cast<T>(static_cast<double>(*pData) * (*num_overlap_images)/(*num_overlap_images+1));
		}
		*pData += static_cast<T>(temp / ((*num_overlap_images+1.0)));

		//Determine number of overlapping images
		if(overlapped) (*num_overlap_images)++;
		//ipoints.clear();
	}
};

namespace
{
	template<typename T>
	/**
	* Function to copy one pixel of an IplImage to a pixel of a RasterElement
	*
	* @param pData Pixel of RasterElement
	* @param frame_pixel Pixel of IplImage
	*/
	void IplImagetoRaster(T* pData, double frame_pixel)
	{
		*pData = static_cast<T>(frame_pixel);
	}
};

/**
* Function to allocate memory for an IplImage
*
* @param IplImage Allocated memory
* @param CvSize Size of image
* @param depth Depth of image
* @param channels Numbers of channels of the image
*/
inline static void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
{
	if ( *img != NULL )	return;

	*img = cvCreateImage( size, depth, channels );
	if ( *img == NULL )
	{
		QMessageBox::critical(NULL, "Drizzle: Memory allocation", "Memory allocation failed", "Back" );
		return;
	}

}

DrizzleVideo_GUI::DrizzleVideo_GUI(QWidget* Parent): QDialog(Parent)
{
	this->setWindowTitle("Drizzle algorithm");
	setModal(FALSE);

	//WIDGETS
	Apply = new QPushButton( "applyButton", this );
	Apply->setText("Drizzle!");

	Cancel = new QPushButton( "cancelButton", this );
	Cancel->setText("Cancel");

	Browse = new QPushButton( "browseButton", this );
	Browse->setText("Browse");

	Video = new QLabel("Select input video", this);
	Size = new QLabel("Input frame size:", this);
	NumImages = new QLabel(" / ", this);

	Dir = new QLineEdit(this);

	x_out_text = new QLabel("Output size (x)");
	y_out_text = new QLabel("Output size (y)");
	dropsize_text = new QLabel("Dropsize");
	num_images_text = new QLabel("Number of frames:");
	x_out = new QLineEdit(this);
	y_out = new QLineEdit(this);
	dropsize = new QLineEdit(this);
	num_images = new QLineEdit(this);

	//LAYOUT

	QGridLayout* pLayout = new QGridLayout(this);
	
	pLayout->addWidget( Video, 0, 0, 1, 3);

	pLayout->addWidget( Dir, 1, 0, 1, 2 );
	pLayout->addWidget(Browse, 1, 2, 1, 1);

	pLayout->addWidget( Size, 2, 0, 1, 3);

	pLayout->addWidget(num_images_text, 3, 0, 1, 1);
	pLayout->addWidget(num_images, 3, 1, 1, 1);
	pLayout->addWidget(NumImages,3,2,1,1);

	pLayout->addWidget( x_out_text,4,0);
	pLayout->addWidget( y_out_text,4,1);
	pLayout->addWidget( dropsize_text,4,2);
	pLayout->addWidget( x_out,5,0);
	pLayout->addWidget( y_out,5,1);
	pLayout->addWidget( dropsize,5,2);

	pLayout->addWidget(Cancel, 6, 2,1,3);
	pLayout->addWidget(Apply, 6, 0,1,1);

	//Call init() for the necessary initialisations
	init();
}

DrizzleVideo_GUI::~DrizzleVideo_GUI()
{

}

void DrizzleVideo_GUI::init()
{
	//Initialize buttons
	connect(Cancel, SIGNAL(clicked()), this, SLOT(reject()));
	connect(Apply, SIGNAL(clicked()), this, SLOT(PerformDrizzle()));
	connect(Browse, SIGNAL(clicked()), this, SLOT(browse()));
	connect(Dir, SIGNAL(textChanged(const QString &)), this, SLOT(updateInfo()));

	//Fix size of GUI
	this->layout()->setSizeConstraint( QLayout::SetFixedSize );
}

void DrizzleVideo_GUI::updateInfo()
{
	//Update size and number of frames when input video changes
	CvCapture* input_video = cvCreateFileCapture(Dir->text().toStdString().c_str());
	if (input_video != NULL)
	{
		int frame_height = (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_HEIGHT );
		int frame_width = (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_WIDTH );

		Size->setText("Input frame size:\t"+ QString::number(frame_width) + "x" + QString::number(frame_height));

		int number_of_frames = (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_COUNT );

		NumImages->setText(" / " +QString::number(number_of_frames));
	}
	else
	{
		Size->setText("Input frame size:");
		NumImages->setText(" / ");
	}
}

void DrizzleVideo_GUI::browse()
{
	fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Files (*.mp4)"));
    Dir->setText(fileName);
}

void DrizzleVideo_GUI::closeGUI(){
	this->reject();
}

bool DrizzleVideo_GUI::PerformDrizzle(){
	Service<ModelServices> pModel;
	StepResource pStep( "DrizzleVideo GUI", "app", "7743FFD5-C2DA-4AD5-B0F0-9D6AF2C01A86" );
	ProgressResource pProgress("ProgressBar");

	//Get input video from LineEdit
	CvCapture* input_video = cvCreateFileCapture(Dir->text().toStdString().c_str());

	//Check whether video input was succesfull
	if (input_video == NULL)
	{
		QMessageBox::critical( this, "Drizzle", "Video input failed'", "Back" );
		pStep->finalize( Message::Failure, "Video input failed!" );
		return false;
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

	//Check whether number of frames to be used is filled in
	if(num_images->text().isNull() || num_images->text().isEmpty())
	{
		pProgress->updateProgress("No number of frames specified.", 100, ERRORS);
		return false;
	}

	//New IplImage as buffer and for frame1
	static IplImage *frame = NULL, *frame1 = NULL, *frame1_1C = NULL;

	//Get size of frames
	CvSize frame_size;
	frame_size.height = (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_HEIGHT );
	frame_size.width = (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_WIDTH );

	//Get total number of frames in video
	long number_of_frames;
	cvSetCaptureProperty( input_video, CV_CAP_PROP_POS_AVI_RATIO, 1. );
	number_of_frames = (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_COUNT );

	//Check whether number of frames to be used is not larger than the total amount of frames
	if(num_images->text().toInt() > number_of_frames)
	{
		pProgress->updateProgress("Too many input frames selected!", 100, ERRORS);
		return false;
	}

	//Reset current frame to first
	cvSetCaptureProperty( input_video, CV_CAP_PROP_POS_FRAMES, 0. );
	long current_frame = 0;

	//Get frame from input video
	frame = cvQueryFrame(input_video);
	
	//Check whether getting frame was succesfull
	if(&frame == NULL)
	{
		QMessageBox::critical( this, "Drizzle",  "Error: unable to load frame.", "Back" );
		pStep->finalize( Message::Failure,  "Error: unable to load frame." );
		return false;
	}

	//Create new vectors containing corner coordinates of previous and first frame
	std::vector<Point2f> prev_frame_corners(4);
	std::vector<Point2f> start_frame_corners(4);

	//Inialise vector containing corner coordinates of start frame to fake coordinates
	start_frame_corners[0] = cvPoint(0,0);
	start_frame_corners[1] = cvPoint(0,1);
	start_frame_corners[2] = cvPoint(1,1);
	start_frame_corners[3] = cvPoint(1,0);

	//Create new RasterElement for output image and for current frame
	ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement("DrizzleVideo_output", y_out->text().toDouble(), x_out->text().toDouble(), INT1UBYTE));
	ModelResource<RasterElement> pFrameCube(RasterUtilities::createRasterElement("temp_frame", frame_size.height, frame_size.width,  INT1UBYTE));

	//Check whether creation of RasterElements was succesfull
	if (pResultCube.get() == NULL || pFrameCube.get() == NULL){
		std::string msg = "A raster cube could not be created.";
		pStep->finalize(Message::Failure, msg);
		return false;
	}

	//Get DataAccessor and RasterDataDescriptor of output RasterElement
	FactoryResource<DataRequest> pResultRequest;
	DataAccessor pDestAcc = pResultCube->getDataAccessor(pResultRequest.release());
	RasterDataDescriptor* pDestDesc = static_cast<RasterDataDescriptor*>(pResultCube->getDataDescriptor());

	//Get RasterDataDescriptor of frame RasterElement
	RasterDataDescriptor* pFrameDesc = static_cast<RasterDataDescriptor*>(pFrameCube->getDataDescriptor());

	//Create list containing corner coordinates
	std::list<GcpPoint> pNewGcpList(4);

	//Set corner coordinates of the output RasterElement
	std::list<GcpPoint>::iterator it = pNewGcpList.begin();
	it->mPixel = *(new LocationType(0, 0));
	it->mCoordinate = *(new LocationType(start_frame_corners[0].x, start_frame_corners[0].y));
	std::advance(it, 1);
	it->mPixel = *(new LocationType(0, y_out->text().toDouble()));
	it->mCoordinate = *(new LocationType(start_frame_corners[1].x, start_frame_corners[1].y));
	std::advance(it, 1);
	it->mPixel = *(new LocationType(x_out->text().toDouble(), y_out->text().toDouble()));
	it->mCoordinate = *(new LocationType(start_frame_corners[2].x, start_frame_corners[2].y));
	std::advance(it, 1);
	it->mPixel = *(new LocationType(x_out->text().toDouble(), 0));
	it->mCoordinate = *(new LocationType(start_frame_corners[3].x, start_frame_corners[3].y));
	GcpList* newGCPList = static_cast<GcpList*>(pModel->createElement("Corner coordinates","GcpList",pResultCube.get()));
	newGCPList->addPoints(pNewGcpList);

	//Get GeoreferenceDescriptor of output RasterElement
	GeoreferenceDescriptor *pDestGeoDesc = pDestDesc->getGeoreferenceDescriptor();
	//Set GeoreferencePlugin to be used
	pDestGeoDesc->setGeoreferencePlugInName("GCP Georeference");

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
	if (!plugInName.empty()){
		ExecutableResource geoPlugIn(plugInName);
		PlugInArgList& argList = geoPlugIn->getInArgList();
		argList.setPlugInArgValue(Executable::DataElementArg(), pResultCube.get());
		argList.setPlugInArgValue(Executable::ProgressArg(), pProgress.get());
		argList.setPlugInArgValueLoose(Georeference::GcpListArg(), newGCPList);
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

	//Output destination RasterElement
	pView->setPrimaryRasterElement(pResultCube.get());
	pView->createLayer(RASTER, pResultCube.get());
	
	//Set corner coordinates of the frame RasterElement
	it = pNewGcpList.begin();
	it->mPixel = *(new LocationType(0, 0));
	it->mCoordinate = *(new LocationType(start_frame_corners[0].x, start_frame_corners[0].y));
	std::advance(it, 1);
	it->mPixel = *(new LocationType(0, frame_size.height));
	it->mCoordinate = *(new LocationType(start_frame_corners[1].x, start_frame_corners[1].y));
	std::advance(it, 1);
	it->mPixel = *(new LocationType(frame_size.width, frame_size.height));
	it->mCoordinate = *(new LocationType(start_frame_corners[2].x, start_frame_corners[2].y));
	std::advance(it, 1);
	it->mPixel = *(new LocationType(frame_size.width, 0));
	it->mCoordinate = *(new LocationType(start_frame_corners[3].x, start_frame_corners[3].y));
	newGCPList = static_cast<GcpList*>(pModel->createElement("Corner coordinates","GcpList",pFrameCube.get()));
	newGCPList->addPoints(pNewGcpList);

	//Get GeoreferenceDescriptor of frame RasterElement
	GeoreferenceDescriptor *pFrameGeoDesc = pFrameDesc->getGeoreferenceDescriptor();
	//Set GeoreferencePlugin to be used
	pFrameGeoDesc->setGeoreferencePlugInName("GCP Georeference");

	//Georeference the frame using the Georeference Plugin
	if (!plugInName.empty()){
		ExecutableResource geoPlugIn(plugInName);
		PlugInArgList& argList = geoPlugIn->getInArgList();
		argList.setPlugInArgValue(Executable::DataElementArg(), pFrameCube.get());
		argList.setPlugInArgValue(Executable::ProgressArg(), pProgress.get());
		argList.setPlugInArgValueLoose(Georeference::GcpListArg(), newGCPList);
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

	//Create vectors containing RasterElements and DataAccessors for all frames of video
	std::vector<ModelResource<RasterElement>> rasters;
	std::vector<DataAccessor> accessors;

	//Get DataAccessor of frame RasterElement
	FactoryResource<DataRequest> pFrameRequest;
	DataAccessor pFrameAcc = pFrameCube->getDataAccessor(pFrameRequest.release());

	//Copy frame to gray scale and color IplImages
	allocateOnDemand( &frame1_1C, frame_size, IPL_DEPTH_8U, 1 );
	cvConvertImage(frame, frame1_1C);
	allocateOnDemand( &frame1, frame_size, IPL_DEPTH_8U, 3 );
	cvConvertImage(frame, frame1);

	//Set frame RasterElement to top left pixel.
	pFrameAcc->toPixel(0,0);
	//Copy IplImage to RasterElement
	for (unsigned int row = 0; row < pFrameDesc->getRowCount(); ++row){ 
		if (!pDestAcc.isValid())
		{
			std::string msg = "Unable to access the cube data.";
			pStep->finalize(Message::Failure, msg);
			pProgress->updateProgress(msg, 0, ERRORS);
			return false;
		}

		for (unsigned int col = 0; col < pFrameDesc->getColumnCount(); ++col)
		{
			switchOnEncoding(pFrameDesc->getDataType(), IplImagetoRaster, pFrameAcc->getColumn(), frame1_1C->imageData[row*frame1_1C->widthStep+col]);
			pFrameAcc->nextColumn();
		}
		pFrameAcc->nextRow();
	}

	//Add RasterElement and DataAccessor of first frame to vectors
	rasters.push_back(pFrameCube);
	accessors.push_back(pFrameAcc);

	//Initialise previous frame corners as the start frame corners
	prev_frame_corners[0] = start_frame_corners[0];
	prev_frame_corners[1] = start_frame_corners[1];
	prev_frame_corners[2] = start_frame_corners[2];
	prev_frame_corners[3] = start_frame_corners[3];

	//Set frame counter to one (one frame is already processed)
	int counter = 1;
	//Get number of frames to be used
	int num_frames = num_images->text().toInt();

	while(counter < num_frames)
	{
		//New IplImage  for frame2
		static IplImage *frame2_1C = NULL, *frame2 = NULL;

		//Copy previous frame to gray scale and color IplImages (frame1)
		allocateOnDemand( &frame1_1C, frame_size, IPL_DEPTH_8U, 1 );
		cvConvertImage(frame, frame1_1C);
		allocateOnDemand( &frame1, frame_size, IPL_DEPTH_8U, 3 );
		cvConvertImage(frame, frame1);

		//Get frame from input video
		frame = cvQueryFrame(input_video);
	
		//Check whether getting frame was succesfull
		if(&frame == NULL)
		{
			QMessageBox::critical( this, "Drizzle", "Error: unable to load frame.", "Back" );
			pStep->finalize( Message::Failure, "Error: unable to load frame." );
			return false;
		}

		//Copy current frame to gray scale and color IplImages (frame2)
		allocateOnDemand( &frame2_1C, frame_size, IPL_DEPTH_8U, 1 );
		cvConvertImage(frame, frame2_1C);
		allocateOnDemand( &frame2, frame_size, IPL_DEPTH_8U, 3 );
		cvConvertImage(frame, frame2);

		//SURF DETECTION AND DESCRIPTION
		std::vector< KeyPoint > frame1_features;
		std::vector< KeyPoint >  frame2_features;

		int minHessian = 600;
		SurfFeatureDetector detector(minHessian);

		detector.detect( frame1_1C, frame1_features );
		detector.detect( frame2_1C, frame2_features );

		SurfDescriptorExtractor extractor;

		cv::Mat frame1_descriptors, frame2_descriptors;

		extractor.compute(frame1_1C,frame1_features,frame1_descriptors);
		extractor.compute(frame2_1C,frame2_features,frame2_descriptors);

		FlannBasedMatcher matcher;
		std::vector<DMatch> matches;

		matcher.match(frame1_descriptors, frame2_descriptors, matches);

		double max_dist = 0; double min_dist = 100;

		for( int i = 0; i < frame1_descriptors.rows; i++ )
		{ double dist = matches[i].distance;
		if( dist < min_dist && dist != 0 ) min_dist = dist;
		if( dist > max_dist ) max_dist = dist;
		}

		std::vector< DMatch > good_matches;

		for( int i = 0; i < frame1_descriptors.rows; i++ )
		{ 
			//if( matches[i].distance < 100*min_dist )
			{ 
				good_matches.push_back( matches[i]); 
			}
		}

		cv::Mat img_matches;
		cv::Mat frame1_mat = cv::Mat(frame1);
		drawMatches( frame1_mat,  frame1_features,cv::Mat(frame2), frame2_features, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

		std::vector< Point2f > frame1_matches;
		std::vector< Point2f > frame2_matches;

		for( int i = 0; i < good_matches.size(); i++ )
		{
			frame1_matches.push_back( frame1_features[ good_matches[i].queryIdx ].pt );
			frame2_matches.push_back( frame2_features[ good_matches[i].trainIdx ].pt );
		}

		//Determine transformation matrix between matches
		Mat H = findHomography( frame2_matches, frame1_matches, CV_RANSAC );

		//Compensate for the difference between frame size and frame coordinates (width in pixel != width in coordinates)
		H.at<double>(0,2)/=frame_size.width;
		H.at<double>(1,2)/=frame_size.height;

		// Get the corners from the previous frame
		std::vector<Point2f> frame1_corners(4);
		frame1_corners[0] = prev_frame_corners[0];
		frame1_corners[1] = prev_frame_corners[1];
		frame1_corners[2] = prev_frame_corners[2];
		frame1_corners[3] = prev_frame_corners[3];

		//Get the corners of the current frame via the transformation matrix determined by SURF
		std::vector<Point2f> frame2_corners(4);
		perspectiveTransform( frame1_corners, frame2_corners, H);

		//Create new RasterElement for current frame
		std::string name("temp_frame_" + std::to_string(static_cast<long long>(counter)));
		ModelResource<RasterElement> pTempCube(RasterUtilities::createRasterElement(name, frame_size.height, frame_size.width, INT1UBYTE));
		pFrameCube = pTempCube;

		//Check whether RasterElement creation was succesfull
		if (pFrameCube.get() == NULL){
			std::string msg = "A raster cube could not be created.";
			pProgress->updateProgress(msg, 0, WARNING);
			pStep->finalize(Message::Failure, msg);
			return false;
		}

		//Get RasterDataDescriptor of current frame
		pFrameDesc = static_cast<RasterDataDescriptor*>(pFrameCube->getDataDescriptor());

		//Set corner coordinates of the current frame RasterElement
		it = pNewGcpList.begin();
		it->mPixel = *(new LocationType(0, 0));
		it->mCoordinate = *(new LocationType(frame2_corners[0].x, frame2_corners[0].y));
		std::advance(it, 1);
		it->mPixel = *(new LocationType(0, frame_size.height));
		it->mCoordinate = *(new LocationType(frame2_corners[1].x, frame2_corners[1].y));
		std::advance(it, 1);
		it->mPixel = *(new LocationType(frame_size.width, frame_size.height));
		it->mCoordinate = *(new LocationType(frame2_corners[2].x, frame2_corners[2].y));
		std::advance(it, 1);
		it->mPixel = *(new LocationType(frame_size.width, 0));
		it->mCoordinate = *(new LocationType(frame2_corners[3].x, frame2_corners[3].y));
		newGCPList = static_cast<GcpList*>(pModel->createElement("Corner coordinates","GcpList",pFrameCube.get()));
		newGCPList->addPoints(pNewGcpList);

		//Get GeoreferenceDescriptor of current frame RasterElement
		pFrameGeoDesc = pFrameDesc->getGeoreferenceDescriptor();
		//Set GeoreferencePlugin to be used
		pFrameGeoDesc->setGeoreferencePlugInName("GCP Georeference");

		//Georeference the frame using the Georeference Plugin
		if (!plugInName.empty()){
		ExecutableResource geoPlugIn(plugInName);
		PlugInArgList& argList = geoPlugIn->getInArgList();
		argList.setPlugInArgValue(Executable::DataElementArg(), pFrameCube.get());
		argList.setPlugInArgValue(Executable::ProgressArg(), pProgress.get());
		argList.setPlugInArgValueLoose(Georeference::GcpListArg(), newGCPList);
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
		
		//Get DataAccessor of the current frame RasterElement
		FactoryResource<DataRequest> pFrameRequest;
		pFrameAcc = pFrameCube->getDataAccessor(pFrameRequest.release());

		//Set frame RasterElement to top left pixel.
		pFrameAcc->toPixel(0,0);
		//Copy IplImage to RasterElement
		for (unsigned int row = 0; row < pFrameDesc->getRowCount(); ++row){ 
			if (!pFrameAcc.isValid())
			{
				std::string msg = "Unable to access the cube data.";
				pStep->finalize(Message::Failure, msg);
				pProgress->updateProgress(msg, 0, ERRORS);
				return false;
			}

			for (unsigned int col = 0; col < pFrameDesc->getColumnCount(); ++col)
			{
				switchOnEncoding(pFrameDesc->getDataType(), IplImagetoRaster, pFrameAcc->getColumn(), frame2_1C->imageData[row*frame2_1C->widthStep+col]);
				
				pFrameAcc->nextColumn();
			}
			pFrameAcc->nextRow();
		}
		//Add RasterElement and DataAccessor of current frame to vectors
		rasters.push_back(pFrameCube);
		accessors.push_back(pFrameAcc);

		//Set previous frame corners for use by next frame
		prev_frame_corners[0] = frame2_corners[0];
		prev_frame_corners[1] = frame2_corners[1];
		prev_frame_corners[2] = frame2_corners[2];
		prev_frame_corners[3] = frame2_corners[3];

		counter++;
	}
	double num_overlap_images;

	//Get dropsize from GUI
	double drop = dropsize->text().toDouble();

	//Reset destination image to top left pixel.
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
			num_overlap_images=0.0;
			for (int i=0; i<rasters.size();i++){
				switchOnEncoding(pDestDesc->getDataType(), DrizzleVideo, pDestAcc->getColumn(), pDestAcc, accessors[i], row, col, pDestDesc->getRowCount(), pDestDesc->getColumnCount(), drop, &num_overlap_images);
			}
			pDestAcc->nextColumn();
		}
		pDestAcc->nextRow();
	}

	//Release output RasterElement
	pResultCube.release();

	pStep->finalize();
	pProgress->updateProgress("Done", 100, NORMAL);
	this->accept();
	return true;
}