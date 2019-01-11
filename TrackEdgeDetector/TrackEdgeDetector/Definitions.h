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
  Pixel(const int& x, const int& y)
  : x(x), y(y)
  {}

  int x, y;
};

///////////////////////////////////////////////////////////////////////////////

struct Mask
{
  Mask(const Pixel& p1, const Pixel& p2)
  : p1(p1), p2(p2)
  {}

  Pixel p1, p2;
};

///////////////////////////////////////////////////////////////////////////////

struct DirectionShifter
{
  DirectionShifter(const int& x, const int& y)
  : x(x), y(y)
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

  FIRST = 0,
  LAST  = 8
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

struct StartingSet
{
  StartingSet(const Pixel& p1, const Pixel& p2, const Direction& direction)
  : p1(p1), p2(p2), direction(direction)
  {}

  Pixel p1, p2;
  Direction direction;
};

///////////////////////////////////////////////////////////////////////////////

#endif // DEFINITIONS_H
