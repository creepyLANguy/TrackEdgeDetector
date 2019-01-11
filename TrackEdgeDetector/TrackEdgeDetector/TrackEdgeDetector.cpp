#include "bmp/EasyBMP.h"
#include "Definitions.h"
#include <vector>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

const int start_x_p1 = 423;
const int start_y_p1 = 38;

const int start_x_p2 = start_x_p1;
const int start_y_p2 = 37;

int currentDirectionShifterIndex = W;

const RGBApixel colour_visited = { 0, 0, 255, 0 }; //struct order is BGRA for some reason :/

const char* kSourceName = "source.bmp";
const char* kOutputName = "edges.bmp";

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

inline bool IsBlack(const RGBApixel& p)
{
  return (p.Red + p.Green + p.Blue + p.Alpha) == BLACK;
}

///////////////////////////////////////////////////////////////////////////////

bool IsValidSample(const RGBApixel& p1, const RGBApixel& p2)
{
  if ((p1 == colour_visited) || (p2 == colour_visited))
  {
    return false;;
  }

  int found_black = 0;
  int found_non_black = 0;

  if (IsBlack(p1) == true)
  {
    ++found_black;
  }
  else
  {
    ++found_non_black;
  }

  if (IsBlack(p2) == true)
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

  for (int iterCount = 0; iterCount < LAST; ++iterCount)
  {
    if (index == LAST)
    {
      index = 0;
    }

    Mask mask_temp = mask;

    const DirectionShifter d = directionMap[index];
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

    if (isShiftedOnWhite == true)
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

void DrawEdgeToOutputAndSave(const bool saveAllIterations = false)
{
  output.SetSize(source.TellWidth(), source.TellHeight());

  int iterations = 0;
  for (const auto i : edges)
  {
    output.SetPixel(i.first, i.second, colour_visited);

    if (saveAllIterations == true)
    {
      char buff[16] = {0};
      _itoa_s(iterations, buff, 10);
      strcat_s(buff, 16, ".bmp");
      output.WriteToFile(buff);
    }
    ++iterations;
  }

  output.WriteToFile(kOutputName);
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
  source.ReadFromFile(kSourceName);

  SetFirstBlackPixel();

  do
  {
    edges.push_back(make_pair(p->x, p->y));
    source.SetPixel(p->x, p->y, colour_visited);
  } 
  while (GetNextPoint() == true);
  
  DrawEdgeToOutputAndSave(true);

  delete p;
}

///////////////////////////////////////////////////////////////////////////////
