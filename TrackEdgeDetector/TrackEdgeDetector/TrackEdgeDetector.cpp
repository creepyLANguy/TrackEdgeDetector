#include "bmp/EasyBMP.h"
#include "Definitions.h"
#include <string>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

///////////////////////////////////////////////////////////////////////////////

vector<StartingSet> startingSets =
{
  //spliced
  StartingSet(Pixel(3091, 3431), Pixel(3091, 3430), E), StartingSet(Pixel(3039, 3573), Pixel(3039, 3574), E),
};

string sourceName = "spliced.bmp";

const char* kOutputName     = "edges.bmp";
const char* kCompositeName  = "composite.bmp";
const int tickCount = abs(static_cast<int>(GetTickCount()));
const string kFolderName    = to_string(tickCount);

//struct order is BGRA for some reason :/
const RGBApixel colour_visited = { 0, 0, 255, 0 }; //red

///////////////////////////////////////////////////////////////////////////////

vector<vector<pair<int, int>>> edges_collection;

BMP source;

Direction currentDirection_global;

Pixel* pixel_global = nullptr;

Mask* mask_global = nullptr;

///////////////////////////////////////////////////////////////////////////////

bool IsValidSample(const RGBApixel& p1, const RGBApixel& p2)
{
  if ((p1 == colour_visited) || (p2 == colour_visited))
  {
    return false;;
  }

  int found_black = 0;
  int found_non_black = 0;

  if (p1 == blackPixel)
  {
    ++found_black;
  }
  else
  {
    ++found_non_black;
  }

  if (p2 == blackPixel)
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

    p1 == blackPixel 
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

void DrawEdgesAndSave
(
  BMP& canvas, 
  const char* filename, 
  const bool saveAllIterations = false,
  const RGBApixel& colourconst = blackPixel
)
{
  string fullPath = kFolderName + "/" + filename;

  int iterations = 0;
  for (auto edges : edges_collection)
  {
    for (const auto i : edges)
    {
      canvas.SetPixel(i.first, i.second, colourconst);

      if (saveAllIterations == true)
      {
        string path_bmp = kFolderName + "/" + to_string(iterations) + ".bmp";
        canvas.WriteToFile(path_bmp.c_str());
      }
      ++iterations;
    }
  }

  canvas.WriteToFile(fullPath.c_str());
}

///////////////////////////////////////////////////////////////////////////////

void SetFirstBlackPixel(StartingSet& set)
{
  const RGBApixel sample = source.GetPixel(set.p1.x, set.p1.y);
  
  sample == blackPixel 
  ? pixel_global = new Pixel(set.p1.x, set.p1.y) 
  : pixel_global = new Pixel(set.p2.x, set.p2.y);
}

///////////////////////////////////////////////////////////////////////////////

void InitialiseEdgesCollection()
{
  for (auto i : startingSets)
  {
    edges_collection.emplace_back(vector<pair<int, int>>());
  }
}

///////////////////////////////////////////////////////////////////////////////

void WriteEdgesToTextFiles()
{
  int collectionNumber = 1;
  for (auto edgecollection : edges_collection)
  {
    ofstream movefile(kFolderName + "/" + to_string(collectionNumber) + ".txt");
    
    for(const auto p : edgecollection)
    {
      movefile << to_string(p.first) << " " << to_string(p.second) << "\n";
    }
    movefile.close();
    ++collectionNumber;
  }
}

///////////////////////////////////////////////////////////////////////////////

void ReInitialiseStartingSets(char* argv[])
{
  startingSets.clear();

  int i = 1;

  //DOING THIS STUPIDLY COS HAVING STRANGE ISSUE WITH CMD ARGS
  const int m1x1 = atoi(argv[++i]);
  const int m1y1 = atoi(argv[++i]);
  const int m1x2 = atoi(argv[++i]);
  const int m1y2 = atoi(argv[++i]);

  const int m2x1 = atoi(argv[++i]);
  const int m2y1 = atoi(argv[++i]);
  const int m2x2 = atoi(argv[++i]);
  const int m2y2 = atoi(argv[++i]);


  startingSets.emplace_back(
    //StartingSet(Pixel(atoi(argv[++i]), atoi(argv[++i])), Pixel(atoi(argv[++i]), atoi(argv[++i])), FIRST));
    StartingSet(Pixel(m1x1, m1y1), Pixel(m1x2, m1y2), FIRST));
  startingSets.emplace_back(
    //StartingSet(Pixel(atoi(argv[++i]), atoi(argv[++i])), Pixel(atoi(argv[++i]), atoi(argv[++i])), FIRST));
    StartingSet(Pixel(m2x1, m2y1), Pixel(m2x2, m2y2), FIRST));
}

///////////////////////////////////////////////////////////////////////////////

void ShowError()
{
  wstring msg = L"Could not find : ";
  msg += wstring(sourceName.begin(), sourceName.end());

  MessageBox(
    nullptr,
    msg.c_str(),
    L"Error",
    MB_ICONEXCLAMATION
  );
}

///////////////////////////////////////////////////////////////////////////////

int main(const int argc, char* argv[])
{  
  //AL.
  //For debugging!
  #ifdef _DEBUG
  MessageBox(nullptr,L"Attach",L"",0);
  #endif
  for (int i = 0; i < argc; ++i)
  {
    cout << argv[i] << "\n";
  }
  cout << "\nDetecting Edges...";
  //

  if (argc > 1)
  {
    sourceName = argv[1];
    ReInitialiseStartingSets(argv);
  }

  InitialiseEdgesCollection();


  if (source.ReadFromFile(sourceName.c_str()) == false)
  {
    ShowError();
    return -1;
  }

  CreateDirectoryA(kFolderName.c_str(), nullptr);
  
  BMP source_copy = source;

  int currentSetIndex = 0;
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
      edges_collection[currentSetIndex].emplace_back(pixel_global->x, pixel_global->y);
      source.SetPixel(pixel_global->x, pixel_global->y, colour_visited);
    } 
    while (GetNextPoint(*mask_global) == true);

    delete mask_global;
    delete pixel_global;

    ++currentSetIndex;
  }

  WriteEdgesToTextFiles();

  BMP output;
  output.SetSize(source.TellWidth(), source.TellHeight());
  DrawEdgesAndSave(output, kOutputName, false);

  DrawEdgesAndSave(source_copy, kCompositeName, false, colour_visited);

  return tickCount;
}

///////////////////////////////////////////////////////////////////////////////

