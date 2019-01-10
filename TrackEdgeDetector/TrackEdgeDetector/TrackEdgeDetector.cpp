#include "bmp/EasyBMP.h"
#include <vector>
using namespace std;

const int BLACK = 0;
const int WHITE = 255 * 3; //R+G+B , ignoring A

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
int currentDirectionShifterIndex = 0;
const int directionCount = 8;


//The first black point on the very edge, where we will start crawling from. 
const Pixel start = { 235,272, BLACK };

//vertical mask with the starting black point as the bottom pixel. 
Mask mask = { Pixel(start.x, start.y-1, WHITE), Pixel(start.x, start.y, start.colour) };

//Store all edge points that we find in here.
vector<Pixel> edges;

BMP source, output;

RGBApixel colour_visited;

///////////////////////////////////////////////////////////////////////////////

bool IsValidSample(const RGBApixel& p1, const RGBApixel& p2)
{
  int found_black = 0;
  int found_white = 0;
  int found_visited = 0;

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
    ++found_visited;
  }
  if (
    (p1.Red == colour_visited.Red) &&
    (p1.Green == colour_visited.Green) &&
    (p1.Blue == colour_visited.Blue) &&
    (p1.Alpha == colour_visited.Alpha)
    )
  {
    ++found_visited;
  }

  if ((found_black == 1) && (found_white == 1) && (found_visited == 0))
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

bool DoCheck(Mask& mask_temp, const int& index, Pixel& p)
{
  const RGBApixel temp_p1 = source.GetPixel(mask_temp.p1.x, mask_temp.p1.y);
  const RGBApixel temp_p2 = source.GetPixel(mask_temp.p2.x, mask_temp.p2.y);

  mask_temp.p1.colour = temp_p1.Red + temp_p1.Blue + temp_p1.Green;
  mask_temp.p2.colour = temp_p2.Red + temp_p2.Blue + temp_p2.Green;

  if (IsValidSample(temp_p1, temp_p2))
  {
    currentDirectionShifterIndex = index;

    mask.p1.x = mask_temp.p1.x;
    mask.p1.y = mask_temp.p1.y;
    mask.p2.x = mask_temp.p1.x;
    mask.p2.y = mask_temp.p1.y;
    mask.p1.colour = mask_temp.p1.colour;
    mask.p2.colour = mask_temp.p2.colour;

    if (mask.p1.colour == BLACK)
    {
      p.x = mask.p1.x;
      p.y = mask.p1.y;
    }
    else
    {
      p.x = mask.p2.x;
      p.y = mask.p2.y;
    }

    return true;
  }

  return false;

}

///////////////////////////////////////////////////////////////////////////////

bool GetEdgeWithMask(Mask& mask_temp, Pixel& p)
{
  int index = currentDirectionShifterIndex;
  const auto d = directionMap.at(index);
  for (int i = 0; i < directionCount; ++i)
  {
    if (index == directionCount)
    {
      index = 0;
    }

    mask_temp.p1.x += d.x;
    mask_temp.p1.y += d.y;
    mask_temp.p2.x += d.x;
    mask_temp.p2.y += d.y;

    if (DoCheck(mask_temp, index, p) == true)
    {
      return true;
      break;
    }

    ++index;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool GetNextPoint(Pixel& p)
{
  Mask mask_temp = { Pixel(mask.p1.x, mask.p1.y, mask.p1.colour), Pixel(mask.p2.x, mask.p2.y, mask.p2.colour) };

  if (GetEdgeWithMask(mask_temp, p) == false)
  {
    //Seems we need to filp the orientation of the mask. 
    const bool isShiftedOnWhite = ReorientMask(mask_temp);

    if (isShiftedOnWhite)
    {
      if (DoCheck(mask_temp, currentDirectionShifterIndex, p) == true)
      {
        return true;
      }
    }

    return GetEdgeWithMask(mask_temp, p);
  }
  
  return false;
}

///////////////////////////////////////////////////////////////////////////////

void SaveOutput()
{
  output.SetSize(source.TellWidth(), source.TellHeight());
  output.SetBitDepth(1);
  cout << "writing output... " << endl;
  output.WriteToFile("edges.bmp");
}

///////////////////////////////////////////////////////////////////////////////

void DrawEdgeToOutput()
{
  for (auto i : edges)
  {
    const RGBApixel p = { 0 };
    output.SetPixel(i.x, i.y, p);
  }
}

///////////////////////////////////////////////////////////////////////////////

int main()
{
  colour_visited.Red = 255;
  colour_visited.Green = 0; 
  colour_visited.Blue = 0; 
  colour_visited.Alpha = 0;


  source.ReadFromFile("source.bmp");

  Pixel runner (start.x, start.y, start.colour);
  
  do 
  {
    edges.push_back(runner);
    source.SetPixel(runner.x, runner.y, colour_visited);

  } while (GetNextPoint(runner) == true);


  DrawEdgeToOutput();

  SaveOutput();

  return 0;
}

