#include "bmp/EasyBMP.h"
#include "Definitions.h"
#include <vector>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

vector<StartingSet> startingSets =
{
  //StartingSet(Pixel(360, 38), Pixel(360, 37), W),
  StartingSet(Pixel(375, 43), Pixel(375, 42), W),
  StartingSet(Pixel(260, 119), Pixel(260, 120), E)
};

const RGBApixel colour_visited = { 0, 0, 255, 0 }; //struct order is BGRA for some reason :/

const char* kSourceName = "source.bmp";
const char* kOutputName = "edges.bmp";
const char* kCompositeName = "composite.bmp";

///////////////////////////////////////////////////////////////////////////////

Mask* mask = nullptr;

//Store all edge points that we find in here.
vector<pair<int, int>> edges;

BMP source, output;

Pixel* p = nullptr;

Direction currentDirection;

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

  if (currentDirection == N)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirection == S)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirection == E)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirection == W)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirection == NE)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirection == SE)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirection == NW)
  {
    isShiftedOnWhite = true;
  }
  else if (currentDirection == SW)
  {
    isShiftedOnWhite = true;
  }

  return isShiftedOnWhite;
}

///////////////////////////////////////////////////////////////////////////////

bool DoCheck(Mask& mask_temp)
{
  const RGBApixel temp_1 = source.GetPixel(mask_temp.p1.x, mask_temp.p1.y);
  const RGBApixel temp_2 = source.GetPixel(mask_temp.p2.x, mask_temp.p2.y);

  if (IsValidSample(temp_1, temp_2))
  {
    *mask = mask_temp;

    if ((temp_1.Red + temp_1.Blue + temp_1.Green) == BLACK)
    {
      *p = mask->p1;
    }
    else
    {
      *p = mask->p2;
    }

    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool GetEdge(Mask& mask)
{
  Direction index = currentDirection;

  for (int iterCount = 0; iterCount < LAST; ++iterCount)
  {
    if (index == LAST)
    {
      index = FIRST;
    }

    Mask mask_temp = mask;

    const DirectionShifter d = directionMap[index];
    mask_temp.p1.x += d.x;
    mask_temp.p1.y += d.y;
    mask_temp.p2.x += d.x;
    mask_temp.p2.y += d.y;

    if (DoCheck(mask_temp) == true)
    {
      currentDirection = index;
      return true;
    }

    index = static_cast<Direction>(static_cast<int>(index) + 1);
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool GetNextPoint(Mask mask)
{
  const bool hasFoundEdge = GetEdge(mask);
  if (hasFoundEdge == false)
  {
    Mask mask_temp = mask;

    //Seems we need to filp the orientation of the mask. 
    const bool isShiftedOnWhite = ReorientMask(mask_temp);

    if (isShiftedOnWhite == true)
    {
      //If we shift on white, we should check the new pos as it hits a single black. 
      if (DoCheck(mask_temp) == true)
      {
        return true;
      }
    }

    return GetEdge(mask);
  }
  
  return hasFoundEdge;
}

///////////////////////////////////////////////////////////////////////////////

void DrawEdgesToOutputAndSave(const bool saveAllIterations = false)
{
  output.SetSize(source.TellWidth(), source.TellHeight());

  int iterations = 0;
  for (const auto i : edges)
  {
    output.SetPixel(i.first, i.second, colour_visited);

    if (saveAllIterations == true)
    {
      char numbuff[16] = {0};
      _itoa_s(iterations, numbuff, 10);
      strcat_s(numbuff, 16, ".bmp");
      output.WriteToFile(numbuff);
    }
    ++iterations;
  }

  output.WriteToFile(kOutputName);
}

///////////////////////////////////////////////////////////////////////////////

void SetFirstBlackPixel(StartingSet& set)
{
  const RGBApixel sample = source.GetPixel(set.p1.x, set.p1.y);
  if ((sample.Red + sample.Green + sample.Blue) == BLACK)
  {
    p = new Pixel(set.p1.x, set.p1.y);
  }
  else
  {
    p = new Pixel(set.p2.x, set.p2.y);
  }  
}

///////////////////////////////////////////////////////////////////////////////

void main()
{
  source.ReadFromFile(kSourceName);

  for (auto startingset : startingSets)
  {
    mask = new Mask
    (
      Pixel(startingset.p1.x, startingset.p1.y), 
      Pixel(startingset.p2.x, startingset.p2.y)
    );

    SetFirstBlackPixel(startingset);

    currentDirection = startingset.direction;

    do
    {
      edges.push_back(make_pair(p->x, p->y));
      source.SetPixel(p->x, p->y, colour_visited);
    } while (GetNextPoint(*mask) == true);

    delete mask;
    delete p;
  }

  //DrawEdgeToOutputAndSave(true);
  DrawEdgesToOutputAndSave(false);

  source.WriteToFile(kCompositeName);
}

///////////////////////////////////////////////////////////////////////////////
