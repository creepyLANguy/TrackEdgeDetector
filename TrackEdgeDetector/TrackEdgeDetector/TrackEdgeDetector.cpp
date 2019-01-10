#include "bmp/EasyBMP.h"
#include <vector>
using namespace std;

const int BLACK = 0;
const int WHITE = 255 * 3; //R+G+B , ignoring A

const int start_x_p1 = 300;
const int start_y_p1 = 38;
const int start_colour_p1 = BLACK;

const int start_x_p2 = 300;
const int start_y_p2 = 37;
const int start_colour_p2 = WHITE;

struct Pixel
{
  Pixel (const int& x_in, const int& y_in, const int& colour_in)
  : x(x_in), y(y_in), colour(colour_in)
  {}

  int x, y;
  int colour;
};

struct Mask
{
  Mask (const Pixel& p1_in, const Pixel& p2_in)
  : p1(p1_in), p2(p2_in)
  {}

  Pixel p1, p2;
};

struct DirectionShifter
{
  DirectionShifter(const int& x_in, const int& y_in)
  : x(x_in), y(y_in)
  {}

  int x, y;
};

enum Direction
{
  W  = 0,
  NW = 1,
  N  = 2,
  NE = 3,
  E  = 4,
  SE = 5,
  S  = 6,
  SW = 7
};
const vector<DirectionShifter> directionMap =
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
int currentDirectionShifterIndex = W;
const int directionCount = 8;

//vertical mask with the first legitimate edge.
Mask mask = 
{ 
  Pixel(start_x_p1, start_y_p1, start_colour_p1),
  Pixel(start_x_p2, start_y_p2, start_colour_p2)
};

//Store all edge points that we find in here.
vector<pair<int, int>> edges;

BMP source, output;

RGBApixel colour_visited;

Pixel* p;

///////////////////////////////////////////////////////////////////////////////

bool IsValidSample(const RGBApixel& p1, const RGBApixel& p2)
{
  int found_black = 0;
  int found_white = 0;

  const int p1_colour = p1.Red + p1.Green + p1.Blue + p1.Alpha;
  const int p2_colour = p2.Red + p2.Green + p2.Blue + p2.Alpha;

  if (p1_colour == BLACK)
  {
    ++found_black;
  }
  if (p2_colour == BLACK)
  {
    ++found_black;
  }
  
  if (p1_colour == WHITE)
  {
    ++found_white;
  }
  if (p2_colour == WHITE)
  {
    ++found_white;
  }

  if  (
      (p2.Red == colour_visited.Red) &&
      (p2.Green == colour_visited.Green) &&
      (p2.Blue == colour_visited.Blue) &&
      (p2.Alpha == colour_visited.Alpha) 
      )
  {
    return false;;
  }
  if (
    (p1.Red == colour_visited.Red) &&
    (p1.Green == colour_visited.Green) &&
    (p1.Blue == colour_visited.Blue) &&
    (p1.Alpha == colour_visited.Alpha)
    )
  {
    return false;
  }

  if ((found_black == 1) && (found_white == 1))
  {
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool ReorientMask(Mask& mask)
{
  bool isShiftedOnWhite = false;

  if (currentDirectionShifterIndex == N)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirectionShifterIndex == S)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirectionShifterIndex == E)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirectionShifterIndex == W)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirectionShifterIndex == NE)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirectionShifterIndex == SE)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirectionShifterIndex == NW)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirectionShifterIndex == SW)
  {
    isShiftedOnWhite = true;
  }

  return isShiftedOnWhite;
}

///////////////////////////////////////////////////////////////////////////////

bool DoCheck(Mask& mask_temp, const int& index)
{
  const RGBApixel temp_1 = source.GetPixel(mask_temp.p1.x, mask_temp.p1.y);
  const RGBApixel temp_2 = source.GetPixel(mask_temp.p2.x, mask_temp.p2.y);

  if (IsValidSample(temp_1, temp_2))
  {
    mask_temp.p1.colour = temp_1.Red + temp_1.Blue + temp_1.Green;
    mask_temp.p2.colour = temp_2.Red + temp_2.Blue + temp_2.Green;

    currentDirectionShifterIndex = index;

    mask.p1.x = mask_temp.p1.x;
    mask.p1.y = mask_temp.p1.y;
    mask.p2.x = mask_temp.p2.x;
    mask.p2.y = mask_temp.p2.y;
    mask.p1.colour = mask_temp.p1.colour;
    mask.p2.colour = mask_temp.p2.colour;

    if (mask.p1.colour == BLACK)
    {
      p->x = mask.p1.x;
      p->y = mask.p1.y;
    }
    else
    {
      p->x = mask.p2.x;
      p->y = mask.p2.y;
    }

    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool GetEdge()
{
  int index = currentDirectionShifterIndex;
  const auto d = directionMap.at(index);

  for (int i = 0; i < directionCount; ++i)
  {
    if (index == directionCount)
    {
      index = 0;
    }

    Mask mask_temp = 
    { 
      Pixel(mask.p1.x, mask.p1.y, mask.p1.colour), 
      Pixel(mask.p2.x, mask.p2.y, mask.p2.colour) 
    };

    mask_temp.p1.x += d.x;
    mask_temp.p1.y += d.y;
    mask_temp.p2.x += d.x;
    mask_temp.p2.y += d.y;

    if (DoCheck(mask_temp, index) == true)
    {
      return true;
    }

    ++index;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool GetNextPoint()
{
  const bool hasFoundEdge = GetEdge();
  if (hasFoundEdge == false)
  {
    Mask mask_temp =
    {
      Pixel(mask.p1.x, mask.p1.y, mask.p1.colour),
      Pixel(mask.p2.x, mask.p2.y, mask.p2.colour)
    };

    //Seems we need to filp the orientation of the mask. 
    const bool isShiftedOnWhite = ReorientMask(mask_temp);

    if (isShiftedOnWhite)
    {
      //If we shift on white, we should check the new pos as it hits a single black. 
      if (DoCheck(mask_temp, currentDirectionShifterIndex) == true)
      {
        return true;
      }
    }

    return GetEdge();
  }
  
  return hasFoundEdge;
}

///////////////////////////////////////////////////////////////////////////////

void DrawEdgeToOutputAndSave()
{
  output.SetSize(source.TellWidth(), source.TellHeight());

  for (const auto i : edges)
  {
    RGBApixel p = { 0 };
    p.Red = 255;
    output.SetPixel(i.first, i.second, p);
  }

  output.WriteToFile("edges.bmp");
}

///////////////////////////////////////////////////////////////////////////////

int main()
{
  colour_visited.Red = 255;
  colour_visited.Green = 0; 
  colour_visited.Blue = 0; 
  colour_visited.Alpha = 0;


  source.ReadFromFile("source.bmp");

  if (start_colour_p1 == BLACK)
  {
    p = new Pixel(start_x_p1, start_y_p1, start_colour_p1);
  }
  else
  {
    p = new Pixel(start_x_p2, start_y_p2, start_colour_p2);
  }
    
  
  do 
  {
    edges.push_back(make_pair(p->x, p->y));
    source.SetPixel(p->x, p->y, colour_visited);

  } while (GetNextPoint() == true);


  DrawEdgeToOutputAndSave();

  return 0;
}

