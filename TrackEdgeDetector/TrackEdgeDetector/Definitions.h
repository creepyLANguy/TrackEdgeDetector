#ifndef DEFINITIONS_H
#define DEFINITIONS_H

///////////////////////////////////////////////////////////////////////////////

inline bool operator==(const RGBApixel& lhs, const RGBApixel& rhs)
{
  return  
  (lhs.Red    == rhs.Red)   &&
  (lhs.Green  == rhs.Green) &&
  (lhs.Blue   == rhs.Blue)  &&
  (lhs.Alpha  == rhs.Alpha); 
}

///////////////////////////////////////////////////////////////////////////////

const int BLACK = 0;

///////////////////////////////////////////////////////////////////////////////

struct Pixel
{
  Pixel(const int& x_in, const int& y_in)
    : x(x_in), y(y_in)
  {}

  int x, y;
};

///////////////////////////////////////////////////////////////////////////////

struct Mask
{
  Mask(const Pixel& p1_in, const Pixel& p2_in)
    : p1(p1_in), p2(p2_in)
  {}

  Pixel p1, p2;
};

///////////////////////////////////////////////////////////////////////////////

struct DirectionShifter
{
  DirectionShifter(const int& x_in, const int& y_in)
    : x(x_in), y(y_in)
  {}

  int x, y;
};

enum Direction
{
  W   = 0,
  NW  = 1,
  N   = 2,
  NE  = 3,
  E   = 4,
  SE  = 5,
  S   = 6,
  SW  = 7,
  LAST
};

const DirectionShifter directionMap[LAST] =
{
  DirectionShifter(-1, +0),  //  W  = 0,
  DirectionShifter(-1, -1),  //  NW = 1,
  DirectionShifter(+0, -1),  //  N  = 2,
  DirectionShifter(+1, -1),  //  NE = 3,
  DirectionShifter(+1, +0),  //  E  = 4,
  DirectionShifter(+1, +1),  //  SE = 5,
  DirectionShifter(+0, +1),  //  S  = 6,
  DirectionShifter(-1, +1)   //  SW = 7
};

///////////////////////////////////////////////////////////////////////////////

#endif // DEFINITIONS_H
