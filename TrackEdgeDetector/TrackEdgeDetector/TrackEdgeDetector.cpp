#include "bmp/EasyBMP.h"
#include "Definitions.h"
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

///////////////////////////////////////////////////////////////////////////////

vector<StartingSet> startingSets =
{
  //source_simple.bmp
  //StartingSet(Pixel(375, 43), Pixel(375, 42), W), 
  
  //source.bmp
  StartingSet(Pixel(440, 69), Pixel(440, 70), E), 
  StartingSet(Pixel(641, 141), Pixel(641, 140), E) 
  
  //suzuka_full.bmp
  //StartingSet(Pixel(3047, 15521), Pixel(3047, 15520), E), 
  //StartingSet(Pixel(3228, 15733), Pixel(3228, 15734), E) 

  //suzuka_src_fiftypercent.bmp
  //StartingSet(Pixel(1505, 7773), Pixel(1505, 7772), W), 
  //StartingSet(Pixel(1612, 7867), Pixel(1612, 7868), W), 
};

//const char* kSourceName = "source_simple.bmp";
const char* kSourceName = "source.bmp";
//const char* kSourceName = "suzuka_full.bmp";
//const char* kSourceName = "suzuka_src_fiftypercent.bmp";

const char* kOutputName = "edges.bmp";
const char* kCompositeName = "composite.bmp";
const string kFolderName = to_string(GetTickCount());

//struct order is BGRA for some reason :/
const RGBApixel colour_visited = { 255, 200, 0, 0 };

///////////////////////////////////////////////////////////////////////////////

Mask* mask_global = nullptr;

//Store all edge points that we find in here.
vector<pair<int, int>> edges;

BMP source;

Pixel* pixel_global = nullptr;

Direction currentDirection_global;

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

int GetRelativePositionIndex(const Pixel& origin, const Pixel& slave)
{  
  const int diff_x = slave.x - origin.x;
  const int diff_y = slave.y - origin.y;

  for (int i = 0; i < 4; ++i)
  {
    if  (
        (relativePosHelper[i].x == diff_x) && 
        (relativePosHelper[i].y == diff_y)
        )
    {
      return i;
    }
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////

bool DoCheck(Mask& mask_temp)
{
  const RGBApixel p1 = source.GetPixel(mask_temp.p1.x, mask_temp.p1.y);
  const RGBApixel p2 = source.GetPixel(mask_temp.p2.x, mask_temp.p2.y);

  if (IsValidSample(p1, p2))
  {
    *mask_global = mask_temp;

    p1.Red + p1.Blue + p1.Green == BLACK 
    ? *pixel_global = mask_global->p1 
    : *pixel_global = mask_global->p2;
    
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool GetValidEdge(Mask& mask)
{
  Direction index = currentDirection_global;

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
      currentDirection_global = index;
      return true;
    }

    index = static_cast<Direction>(static_cast<int>(index) + 1);
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool TryFindEdgeWithNewMask(Mask& mask)
{
  const int relativePosIndex = GetRelativePositionIndex(mask.p1, mask.p2);

  //AL.
  //I'm commenting this out cos this condition should never be reached. 
  //HOWEVER, if buggy stuffs starts happening, add this check back in.
  /*
  if (relativePosIndex < 0)
  {
    return false;
  }
  */

  for (auto r : rotatormasks)
  {
    mask.p2.x = mask.p2.x + r[relativePosIndex].x;
    mask.p2.y = mask.p2.y + r[relativePosIndex].y;

    //We may have landed on a vald edge by simply rotating! :)
    if (DoCheck(mask) == true)
    {
      return true;
    }

    if (GetValidEdge(mask) == true)
    {
      return true;
    }
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool TryAndGetAValidEdgeWithAllOtherMasksAndRotations(Mask& mask)
{
  vector<Mask> tempMasks =
  {
    Mask(Pixel(mask.p1.x, mask.p1.y), Pixel(mask.p2.x, mask.p2.y)),
    Mask(Pixel(mask.p2.x, mask.p2.y), Pixel(mask.p1.x, mask.p1.y))
  };
  for (auto m : tempMasks)
  {
    if (TryFindEdgeWithNewMask(m) == true)
    {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool GetNextPoint(Mask& mask)
{
  if (GetValidEdge(mask) == true)
  {
    return true;
  }

  //No valid edges found for current mask.
  return TryAndGetAValidEdgeWithAllOtherMasksAndRotations(mask);

}

///////////////////////////////////////////////////////////////////////////////

void DrawEdgesAndSave(BMP& canvas, const char* filename, const bool saveAllIterations = false)
{
  string fullPath = kFolderName + "/" + filename;

  int iterations = 0;
  for (const auto i : edges)
  {
    canvas.SetPixel(i.first, i.second, colour_visited);

    if (saveAllIterations == true)
    {
      string path_bmp = kFolderName + "/" + to_string(iterations) + ".bmp";
      canvas.WriteToFile(path_bmp.c_str());
    }
    ++iterations;
  }

  canvas.WriteToFile(fullPath.c_str());
}

///////////////////////////////////////////////////////////////////////////////

void SetFirstBlackPixel(StartingSet& set)
{
  const RGBApixel sample = source.GetPixel(set.p1.x, set.p1.y);
  
  sample.Red + sample.Green + sample.Blue == BLACK 
  ? pixel_global = new Pixel(set.p1.x, set.p1.y) 
  : pixel_global = new Pixel(set.p2.x, set.p2.y);
}

///////////////////////////////////////////////////////////////////////////////

void main()
{  
  CreateDirectoryA(kFolderName.c_str(), nullptr);

  source.ReadFromFile(kSourceName);
  
  BMP source_copy = source;

  for (auto startingset : startingSets)
  {
    mask_global = new Mask
    (
      Pixel(startingset.p1.x, startingset.p1.y), 
      Pixel(startingset.p2.x, startingset.p2.y)
    );

    SetFirstBlackPixel(startingset);

    currentDirection_global = startingset.direction;

    do
    {
      edges.push_back(make_pair(pixel_global->x, pixel_global->y));
      source.SetPixel(pixel_global->x, pixel_global->y, colour_visited);
    } 
    while (GetNextPoint(*mask_global) == true);

    delete mask_global;
    delete pixel_global;
  }


  BMP output;
  output.SetSize(source.TellWidth(), source.TellHeight());
  DrawEdgesAndSave(output, kOutputName, false);


  DrawEdgesAndSave(source_copy, kCompositeName, false);
}

///////////////////////////////////////////////////////////////////////////////

