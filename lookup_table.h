
#ifndef _LOOKUP_TABLE_H_
#define _LOOKUP_TABLE_H_

class LookupTable
{
 public:
  LookupTable(double *points, int num_points);
  double getValue(double x);

 protected:

  // Note: Actual array size is 2*m_nPoints;
  double *m_Points;
  int m_nPoints;
};

#endif // _LOOKUP_TABLE_H_
