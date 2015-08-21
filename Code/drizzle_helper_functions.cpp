/********************************************//*
* 
* @file: drizzle_helper_functions.cpp
*
* The information in this file is
* Copyright(c) 2015 Tom Van den Eynde
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
***********************************************/

#include "LocationType.h"
#include "drizzle_helper_functions.h"


inline double drizzle_helper_functions::crossprod(LocationType a, LocationType b){
	return a.mX*b.mY - a.mY*b.mX;
}

int drizzle_helper_functions::left_of(LocationType a, LocationType b, LocationType c)
{
	LocationType tmp1 = b - a;
	LocationType tmp2 = c - b;
	double x = tmp1.mX*tmp2.mY - tmp1.mY*tmp2.mX;
	if(x<0) return -1;
	if(x>0) return 1;
	return 0;
}

int drizzle_helper_functions::line_intersect(LocationType x1, LocationType x2, LocationType y1, LocationType y2, LocationType *result){
	LocationType dx, dy, d;
	dx = x2 - x1;
	dy = y2 - y1;
	d = x1 - y1;
	double dyx = crossprod(dy,dx);
	if(!dyx) return 0;
	dyx = crossprod(d, dx)/dyx;
	if(dyx <= 0 || dyx >= 1) return 0;
	result->mX = y1.mX + dyx * dy.mX;
	result->mY = y1.mY + dyx * dy.mY;
	return 1;
}

void drizzle_helper_functions::poly_edge_clip(std::vector<LocationType> sub, LocationType x0, LocationType x1, int left, std::vector<LocationType>* res)
{
	int i, side0, side1;
	LocationType tmp;
	LocationType v0 = sub[sub.size()-1], v1;
	res->clear();
 
	side0 = left_of(x0, x1, v0);

	if (side0 != -left) res->push_back(v0);
 
	for (i = 0; i < sub.size(); i++) {
		v1 = sub[i];
		side1 = left_of(x0, x1, v1);
		if (side0 + side1 == 0 && side0)
			// last point and current point span the edge
			if (line_intersect(x0, x1, v0, v1, &tmp)) res->push_back(tmp);
		if (i == sub.size()-1) break;
		if (side1 != -left) res->push_back(v1);
		v0 = v1;
		side0 = side1;
	}
}