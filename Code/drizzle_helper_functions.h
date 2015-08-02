/*
* The information in this file is
* Copyright(c) 2015, Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef drizzle_helper_functions_H
#define drizzle_helper_functions_H

#include "LocationType.h"

#include <vector>

class drizzle_helper_functions
{

public:
	
	static int ccw(LocationType a, LocationType b, LocationType c);
	static double distance(LocationType a, LocationType b);
	static bool POLAR_ORDER(LocationType a, LocationType b);
	static inline double crossprod(LocationType a, LocationType b);
	static inline double dotprod(LocationType a, LocationType b);
	static int left_of(LocationType a, LocationType b, LocationType c);
	static int line_intersect(LocationType x1, LocationType x2, LocationType y1, LocationType y2, LocationType *result);
	static void poly_edge_clip(std::vector<LocationType> sub, LocationType x0, LocationType x1, int left, std::vector<LocationType>* res);

};
#endif