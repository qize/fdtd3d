#include "Grid.h"

#include <cmath>

extern const char* BufferPositionNames[];

#if defined (PARALLEL_BUFFER_DIMENSION_2D_XY) || defined (PARALLEL_BUFFER_DIMENSION_2D_YZ) || defined (PARALLEL_BUFFER_DIMENSION_2D_XZ)

void
Grid::CalculateGridSizeForNode (grid_coord& c1, int nodeGridSize1, grid_coord size1, grid_coord& c2, int nodeGridSize2, grid_coord size2)
{
  if ((processId + 1) % nodeGridSize1 != 0)
    c1 = size1 / nodeGridSize1;
  else
    c1 = size1 - (nodeGridSize1 - 1) * (size1 / nodeGridSize1);

  if ((processId + 1) % nodeGridSize2 != 0)
    c2 = size2 / nodeGridSize2;
  else
    c2 = size2 - (nodeGridSize2 - 1) * (size2 / nodeGridSize2);
}

void
Grid::FindProportionForNodeGrid (int& nodeGridSize1, int& nodeGridSize2, int& left)
{
  // Bad case, too many nodes left unused. Let's change proportion.
  bool find = true;
  bool direction = nodeGridSize1 > nodeGridSize2 ? true : false;
  while (find)
  {
    find = false;
    if (direction && nodeGridSize1 > 2)
    {
      find = true;
      --nodeGridSize1;
      nodeGridSize2 = totalProcCount / nodeGridSize1;
    }
    else if (!direction && nodeGridSize2 > 2)
    {
      find = true;
      --nodeGridSize2;
      nodeGridSize1 = totalProcCount / nodeGridSize2;
    }

    left = totalProcCount - nodeGridSize1 * nodeGridSize2;

    if (find && left == 0)
    {
      find = false;
    }
  }
}

void
Grid::NodeGridInitInner (FieldValue& overall1, FieldValue& overall2, int& nodeGridSize1, int& nodeGridSize2, int& left)
{
  FieldValue alpha = overall2 / overall1;
  FieldValue sqrtVal = ((FieldValue) (totalProcCount)) / alpha;
  sqrtVal = sqrt (sqrtVal);

  if (sqrtVal <= 1.0 || alpha*sqrtVal <= 1.0)
  {
    ASSERT_MESSAGE ("Unsupported number of nodes for 2D parallel buffers. Use 1D ones.");
  }

  sqrtVal = round (sqrtVal);

  nodeGridSize1 = (int) sqrtVal;
  nodeGridSize2 = totalProcCount / nodeGridSize1;

  left = totalProcCount - nodeGridSize1 * nodeGridSize2;

  if (left > 0)
  {
    // Bad case, too many nodes left unused. Let's change proportion.
    FindProportionForNodeGrid (nodeGridSize1, nodeGridSize2, left);
  }

  ASSERT (nodeGridSize1 > 1 && nodeGridSize2 > 1);
}

#endif