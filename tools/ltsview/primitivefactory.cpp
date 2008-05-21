// Author(s): Bas Ploeger and Carst Tankink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file primitivefactory.cpp
/// \brief Add your file description here.

#include "primitivefactory.h"
#include <cmath>

using namespace Utils;

/* Primitive Factory -------------------------------------------------------- */

PrimitiveFactory::PrimitiveFactory(Settings* ss) {
  disc = -1;
  sphere = -1;
  hemisphere = -1;
  simple_sphere = -1;
  settings = ss;
  settings->subscribe(Quality,this);
	settings->subscribe(BranchTilt,this);
  
  cos_theta = NULL;
  sin_theta = NULL;
  coneDB = new ConeDB();
  update_geom_tables();
}

PrimitiveFactory::~PrimitiveFactory() {
  delete coneDB;
  for (unsigned int i = 0; i < primitives.size(); ++i) {
    delete primitives[i];
  }
  primitives.clear();
  if (cos_theta != NULL) {
    free(cos_theta);
    cos_theta = NULL;
  }
  if (sin_theta != NULL) {
    free(sin_theta);
    sin_theta = NULL;
  }
}

void PrimitiveFactory::drawPrimitive(int p) {
  primitives[p]->draw();
}

void PrimitiveFactory::drawSimpleSphere() {
  make_simple_sphere();
  primitives[simple_sphere]->draw();
}

int PrimitiveFactory::makeTruncatedCone(float r,bool topClosed,bool bottomClosed) {
  int result = make_ring(r);
  if (topClosed || bottomClosed) {
    P_Ring *ring = dynamic_cast<P_Ring*>(primitives[result]);
    result = coneDB->findTruncatedCone(r,topClosed,bottomClosed);
    if (result == -1) {
      make_disc();
      P_TruncatedCone *p = new P_TruncatedCone(ring,
					dynamic_cast<P_Disc*>(primitives[disc]),topClosed,bottomClosed);
      result = primitives.size();
      coneDB->addTruncatedCone(r,topClosed,bottomClosed,result);
      primitives.push_back(p);
    }
  }
  return result;
}

int PrimitiveFactory::makeObliqueCone(float a,float r,float s) {
	int result = coneDB->findObliqueCone(a,r,s);
	if (result == -1) {
		P_ObliqueCone *p = new P_ObliqueCone(a,r,s);
	  p->reshape(settings->getInt(Quality),cos_theta,sin_theta,
							 deg_to_rad(float(settings->getInt(BranchTilt))));
		result = primitives.size();
		primitives.push_back(p);
		oblq_cones.push_back(p);
		coneDB->addObliqueCone(a,r,s,result);
	}
	return result;
}

int PrimitiveFactory::makeHemisphere() {
  if (hemisphere == -1) {
    P_Hemisphere *p = new P_Hemisphere();
    p->reshape(settings->getInt(Quality),cos_theta,sin_theta);
    hemisphere = primitives.size();
    primitives.push_back(p);
  }
  return hemisphere;
}

int PrimitiveFactory::makeSphere() {
  if (sphere == -1) {
    P_Sphere *p = new P_Sphere();
    p->reshape(settings->getInt(Quality),cos_theta,sin_theta);
    sphere = primitives.size();
    primitives.push_back(p);
  }
  return sphere;
}

void PrimitiveFactory::notify(SettingID s) {
	switch (s) {
		case Quality:
			update_geom_tables();
			update_primitives();
			update_oblique_cones();
			break;
		case BranchTilt:
			update_oblique_cones();
			break;
		default:
			break;
	}
}
  
void PrimitiveFactory::update_geom_tables() {
  int qlt = settings->getInt(Quality);
  cos_theta = (float*)realloc(cos_theta,2*qlt*sizeof(float));
  sin_theta = (float*)realloc(sin_theta,2*qlt*sizeof(float));
  
  float d_theta = PI / qlt;
  float theta = 0.0f;
  for (int i = 0; i < 2*qlt; ++i) {
    cos_theta[i] = cos(theta);
    sin_theta[i] = sin(theta);
    theta += d_theta;
  }
}

void PrimitiveFactory::update_primitives() {
  int qlt = settings->getInt(Quality);
  for (unsigned int i = 0; i < primitives.size(); ++i) {
    primitives[i]->reshape(qlt,cos_theta,sin_theta);
  }
}

void PrimitiveFactory::update_oblique_cones() {
	int qlt = settings->getInt(Quality);
	float obt = deg_to_rad(float(settings->getInt(BranchTilt)));
	for (unsigned int i = 0; i < oblq_cones.size(); ++i) {
		oblq_cones[i]->reshape(qlt,cos_theta,sin_theta,obt);
	}
}

int PrimitiveFactory::make_ring(float r) {
  int result = coneDB->findTruncatedCone(r,false,false);
  if (result == -1) {
    P_Ring *p = new P_Ring(r);
    p->reshape(settings->getInt(Quality),cos_theta,sin_theta);
    result = primitives.size();
    primitives.push_back(p);
    coneDB->addTruncatedCone(r,false,false,result);
  }
  return result;
}

void PrimitiveFactory::make_disc() {
  if (disc == -1) {
    P_Disc *p = new P_Disc();
    p->reshape(settings->getInt(Quality),cos_theta,sin_theta);
    disc = primitives.size();
    primitives.push_back(p);
  }
}

void PrimitiveFactory::make_simple_sphere() {
  if (simple_sphere == -1) {
    P_SimpleSphere *p = new P_SimpleSphere();
    simple_sphere = primitives.size();
    primitives.push_back(p);
  }
}
