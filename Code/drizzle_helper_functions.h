/********************************************//*
* 
* @file: drizzle_helper_functions.h
*
* The information in this file is
* Copyright(c) 2015 Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
***********************************************/

#ifndef drizzle_helper_functions_H
#define drizzle_helper_functions_H

#include "LocationType.h"

#include <vector>

/**
*
* Class containing functions used by the Drizzle algorithm for both
* video and image input.
*/
class drizzle_helper_functions
{

public:

	/**
	* Calculates the crossproduct for Locationtypes.
	*
	* @param a first input Locationtype
	* @param b second input Locationtype
	* @return The crossproduct of Locationtype a and Locationtype b
	*/
	static inline double crossprod(LocationType a, LocationType b);
	
	/**
	* Determines whether Locationtype c lies on the left
	* of the line determined by a->b
	*
	* @param a first input Locationtype which determines line
	* @param b second input Locationtype which determines line
	* @param c input Locationtype to determines its location relative to a->b.
	*		
	* @return 1 if c lies left of a->b, -1 if c lies right of a->b, 0 if c lies on a->b.
	*/
	static int left_of(LocationType a, LocationType b, LocationType c);
	
	/**
	* Determines intersection between two lines each determined by two Locationtypes.
	*
	* @param x1 first input Locationtype which determines line 1
	* @param x2 second input Locationtype which determines line 1
	* @param y1 first input Locationtype which determines line 2
	* @param y2 second input Locationtype which determines line 2
	* @param result Locationtype which will hold the intersection point
	*		
	* @return 1 if intersection found, 0 if no intersection found.
	*/
	static int line_intersect(LocationType x1, LocationType x2, LocationType y1, LocationType y2, LocationType *result);
	
	/**
	* Clips polygon determined by a vector of Locationtypes relative to edge determined by two Locationtypes
	*
	* @param sub polygon determined by vector of Locationtypes
	* @param x0 first Locationtype which determines the clipping edge
	* @param x1 second Locationtype which determines the clipping edge
	* @param left Integer corresponding to result of left_of.
	* @param res Locationtype which will hold the clipped polygon
	*/
	static void poly_edge_clip(std::vector<LocationType> sub, LocationType x0, LocationType x1, int left, std::vector<LocationType>* res);

};
#endif