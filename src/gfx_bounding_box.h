#ifndef _GFX_BOUNDING_BOX_H_
#define _GFX_BOUNDING_BOX_H_
#include "gfx_transform_matrix.h"

class BoundingBox {
 private:
  Vector lx,ly,lz;
  Vector mx,my,mz;

 public:
  BoundingBox (Vector LX, Vector MX,Vector LY,Vector MY,Vector LZ,Vector MZ);
  void Transform (Matrix t);
  void Transform (const Vector &p, const Vector &q, const Vector &r);
  void Transform (const Vector &translate);
  Vector Center (){
    return Vector (.16666666666666666F*((mx+lx)+(my+ly)+(mz+lz)));
  }
  float ZCenter () {
    return .166666666666666666F*(mx.k+lx.k+my.k+ly.k+mz.k+lz.k);
  }
  bool Within (Vector query,float err);
};

#endif
