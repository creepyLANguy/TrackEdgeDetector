#include "bmp/EasyBMP.h"
#include <vector>
#include "Definitions.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////

const int start_x_p1 = 423;
const int start_y_p1 = 38;

const int start_x_p2 = start_x_p1;
const int start_y_p2 = 37;

int currentDirectionShifterIndex = W;

const RGBApixel colour_visited = { 0, 0, 255, 0 }; //struct order is BGRA for some reason :/

///////////////////////////////////////////////////////////////////////////////

//mask on the first legitimate edge.
Mask mask = 
{ 
  Pixel(start_x_p1, start_y_p1),
  Pixel(start_x_p2, start_y_p2)
};

//Store all edge points that we find in here.
vector<pair<int, int>> edges;

BMP source, output;

Pixel* p = nullptr;

///////////////////////////////////////////////////////////////////////////////

bool IsValidSample(const RGBApixel& p1, const RGBApixel& p2)
{
  if ((p1 == colour_visited) || (p2 == colour_visited))
  {
    return false;;
  }

  const int p1_colour = p1.Red + p1.Green + p1.Blue + p1.Alpha;
  const int p2_colour = p2.Red + p2.Green + p2.Blue + p2.Alpha;

  int found_black = 0;
  int found_non_black = 0;

  if (p1_colour == BLACK)
  {
    ++found_black;
  }
  else
  {
    ++found_non_black;
  }

  if (p2_colour == BLACK)
  {
    ++found_black;
  }
  else
  {
    ++found_non_black;
  }

  if ((found_black > 1) || (found_non_black > 1))
  {
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

bool ReorientMask(Mask& temp_mask)
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
    currentDirectionShifterIndex = index;

    mask = mask_temp;

    if ((temp_1.Red + temp_1.Blue + temp_1.Green) == BLACK)
    {
      *p = mask.p1;
    }
    else
    {
      *p = mask.p2;
    }

    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool GetEdge()
{
  int index = currentDirectionShifterIndex;
  const DirectionShifter d = directionMap[index];

  for (int iterCount = 0; iterCount < LAST; ++iterCount)
  {
    if (index == LAST)
    {
      index = 0;
    }

    Mask mask_temp = mask;
    
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
    Mask mask_temp = mask;

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
    output.SetPixel(i.first, i.second, colour_visited);
  }

  output.WriteToFile("edges.bmp");
}

///////////////////////////////////////////////////////////////////////////////

void SetFirstBlackPixel()
{
  const RGBApixel sample = source.GetPixel(start_x_p1, start_y_p1);
  if ((sample.Red + sample.Green + sample.Blue) == BLACK)
  {
    p = new Pixel(start_x_p1, start_y_p1);
  }
  else
  {
    p = new Pixel(start_x_p2, start_y_p2);
  }  
}

///////////////////////////////////////////////////////////////////////////////

void main()
{
  source.ReadFromFile("source.bmp");

  SetFirstBlackPixel();
    
  do
  {
    edges.push_back(make_pair(p->x, p->y));
    source.SetPixel(p->x, p->y, colour_visited);
  } 
  while (GetNextPoint());

  DrawEdgeToOutputAndSave();

  delete p;
}

///////////////////////////////////////////////////////////////////////////////
