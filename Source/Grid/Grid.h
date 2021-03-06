#ifndef GRID_H
#define GRID_H

#include <cstdlib>
#include <vector>
#include <string>
#include <cstring>

#include "Assert.h"
#include "FieldValue.h"
#include "GridCoordinate3D.h"
#include "Settings.h"

/**
 * Type of vector of points in grid.
 */
typedef std::vector<FieldValue> VectorFieldValues;

/**
 * Non-parallel grid class.
 */
template <class TCoord>
class Grid
{
protected:

  /**
   * Size of the grid. For parallel grid - size of current node plus size of buffers.
   */
  TCoord size;

  /**
   * Vector of points in grid.
   */
  std::vector<VectorFieldValues *> gridValues;

  /**
   * Name of the grid.
   */
  std::string gridName;

  /*
   * TODO: add debug uninitialized flag
   */

protected:

  static bool isLegitIndex (const TCoord &, const TCoord &);
  static grid_coord calculateIndexFromPosition (const TCoord &, const TCoord &);

private:

  Grid<TCoord> & operator = (const Grid<TCoord> &);
  Grid (const Grid<TCoord> &);

protected:

  bool isLegitIndex (const TCoord &) const;

public:

  Grid (const TCoord&, int, const char * = "unnamed");
  Grid (int, const char * = "unnamed");
  virtual ~Grid ();

  const TCoord & getSize () const;
  virtual TCoord getTotalPosition (const TCoord &) const;
  virtual TCoord getTotalSize () const;
  virtual TCoord getRelativePosition (const TCoord &) const;

  virtual TCoord getComputationStart (const TCoord &) const;
  virtual TCoord getComputationEnd (const TCoord &) const;
  TCoord calculatePositionFromIndex (grid_coord) const;
  grid_coord calculateIndexFromPosition (const TCoord &) const;

  void setFieldValue (const FieldValue &, const TCoord &, int);
  void setFieldValue (const FieldValue &, grid_coord, int);
  FieldValue * getFieldValue (const TCoord &, int);
  FieldValue * getFieldValue (grid_coord, int);

  virtual FieldValue * getFieldValueByAbsolutePos (const TCoord &, int);
  virtual FieldValue * getFieldValueOrNullByAbsolutePos (const TCoord &, int);

  virtual FieldValue * getFieldValueCurrentAfterShiftByAbsolutePos (const TCoord &);
  virtual FieldValue * getFieldValueOrNullCurrentAfterShiftByAbsolutePos (const TCoord &);
  virtual FieldValue * getFieldValuePreviousAfterShiftByAbsolutePos (const TCoord &);
  virtual FieldValue * getFieldValueOrNullPreviousAfterShiftByAbsolutePos (const TCoord &);

  virtual TCoord getChunkStartPosition () const;

  virtual bool isBufferLeftPosition (const TCoord & pos) const
  {
    return false;
  }

  virtual bool isBufferRightPosition (const TCoord & pos) const
  {
    return false;
  }

  void shiftInTime ();

  const char * getName () const;

  void initialize (const FieldValue &);

  FieldValue * getRaw (int);

  int getCountStoredSteps () const
  {
    return gridValues.size ();
  }

  void copy (const Grid<TCoord> *grid)
  {
    ASSERT (size == grid->size);
    ASSERT (gridValues.size () == grid->gridValues.size ());

    for (int i = 0; i < gridValues.size (); ++i)
    {
      ASSERT (gridValues[i]->size () == grid->gridValues[i]->size ());
      ASSERT (gridValues[i]->capacity () == grid->gridValues[i]->capacity ())

      memcpy (&(*gridValues[i])[0], &(*grid->gridValues[i])[0], grid->gridValues[i]->size () * sizeof (FieldValue));
    }
  }
}; /* Grid */

/**
 * Constructor of grid
 */
template <class TCoord>
Grid<TCoord>::Grid (const TCoord &s, /**< size of grid */
                    int storedSteps, /**< number of steps in time for which to store grid values */
                    const char *name) /**< name of grid */
  : size (s)
  , gridValues (storedSteps)
  , gridName (name)
{
  ASSERT (storedSteps > 0);

  for (int i = 0; i < gridValues.size (); ++i)
  {
    gridValues[i] = new VectorFieldValues (size.calculateTotalCoord ());
  }

  DPRINTF (LOG_LEVEL_STAGES_AND_DUMP, "New grid '%s' with %lu stored steps and raw size: %llu.\n",
    gridName.data (), gridValues.size (), (unsigned long long)size.calculateTotalCoord ());
} /* Grid<TCoord>::Grid */

/**
 * Constructor of grid without size
 */
template <class TCoord>
Grid<TCoord>::Grid (int storedSteps, /**< number of steps in time for which to store grid values */
                    const char *name) /**< name of grid */
  : gridValues (storedSteps)
  , gridName (name)
{
  ASSERT (storedSteps > 0);

  for (int i = 0; i < gridValues.size (); ++i)
  {
    gridValues[i] = NULLPTR;
  }

  DPRINTF (LOG_LEVEL_STAGES_AND_DUMP, "New grid '%s' with %lu stored steps without size.\n",
    gridName.data (), gridValues.size ());
} /* Grid<TCoord>::Grid */

/**
 * Destructor of grid. Should delete all field point values
 */
template <class TCoord>
Grid<TCoord>::~Grid ()
{
  for (int i = 0; i < gridValues.size (); ++i)
  {
    delete gridValues[i];
    gridValues[i] = NULLPTR;
  }
} /* Grid<TCoord>::~Grid */

/**
 * Check whether position is appropriate to get/set value from
 *
 * @return flag whether position is appropriate to get/set value from
 */
template <class TCoord>
bool
Grid<TCoord>::isLegitIndex (const TCoord& position) const /**< coordinate in grid */
{
  return isLegitIndex (position, size);
} /* Grid<TCoord>::isLegitIndex */

/**
 * Calculate one-dimensional coordinate from N-dimensional position
 *
 * @return one-dimensional coordinate from N-dimensional position
 */
template <class TCoord>
grid_coord
Grid<TCoord>::calculateIndexFromPosition (const TCoord& position) const /**< coordinate in grid */
{
  return calculateIndexFromPosition (position, size);
} /* Grid<TCoord>::calculateIndexFromPosition */

/**
 * Get size of the grid
 *
 * @return size of the grid
 */
template <class TCoord>
const TCoord &
Grid<TCoord>::getSize () const
{
  return size;
} /* Grid<TCoord>::getSize */

/**
 * Get last coordinate until which to perform computations at current step
 *
 * @return last coordinate until which to perform computations at current step
 */
template <class TCoord>
TCoord
Grid<TCoord>::getComputationEnd (const TCoord & diffPosEnd) const
{
  return getSize () - diffPosEnd;
} /* Grid<TCoord>::getComputationEnd () */

/**
 * Set field value at coordinate in grid
 */
template <class TCoord>
void
Grid<TCoord>::setFieldValue (const FieldValue & value, /**< field point value */
                             const TCoord & position, /**< coordinate in grid */
                             int time_step_back) /**< index of previous time step, starting from current (0) */
{
  ASSERT (isLegitIndex (position));
  ASSERT (time_step_back < gridValues.size ());

  grid_coord coord = calculateIndexFromPosition (position);

  setFieldValue (value, coord, time_step_back);
} /* Grid<TCoord>::setFieldValue */

/**
 * Set field value at coordinate in grid
 */
template <class TCoord>
void
Grid<TCoord>::setFieldValue (const FieldValue & value, /**< field point value */
                             grid_coord coord, /**< index in grid */
                             int time_step_back) /**< index of previous time step, starting from current (0) */
{
  ASSERT (coord >= 0 && coord < size.calculateTotalCoord ());
  ASSERT (time_step_back < gridValues.size ());

  (*gridValues[time_step_back])[coord] = value;
} /* Grid<TCoord>::setFieldValue */

/**
 * Get field value at coordinate in grid
 *
 * @return field value
 */
template <class TCoord>
FieldValue *
Grid<TCoord>::getFieldValue (const TCoord &position, /**< coordinate in grid */
                             int time_step_back) /**< index of previous time step, starting from current (0) */
{
  ASSERT (isLegitIndex (position));
  ASSERT (time_step_back < gridValues.size ());

  grid_coord coord = calculateIndexFromPosition (position);

  return getFieldValue (coord, time_step_back);
} /* Grid<TCoord>::getFieldValue */

/**
 * Get field point value at coordinate in grid
 *
 * @return field point value
 */
template <class TCoord>
FieldValue *
Grid<TCoord>::getFieldValue (grid_coord coord, /**< index in grid */
                             int time_step_back) /**< index of previous time step, starting from current (0) */
{
  ASSERT (coord >= 0 && coord < size.calculateTotalCoord ());
  ASSERT (time_step_back < gridValues.size ());

  return &(*gridValues[time_step_back])[coord];
} /* Grid<TCoord>::getFieldValue */

/**
 * Get field value at relative coordinate in grid
 *
 * @return field value
 */
template <class TCoord>
FieldValue *
Grid<TCoord>::getFieldValueByAbsolutePos (const TCoord &relPosition, /**< relative coordinate in grid */
                                          int time_step_back) /**< index of previous time step, starting from current (0) */
{
  return getFieldValue (relPosition, time_step_back);
} /* Grid<TCoord>::getFieldValueByAbsolutePos */

/**
 * Get field value at relative coordinate in grid or null
 *
 * @return field value or null
 */
template <class TCoord>
FieldValue *
Grid<TCoord>::getFieldValueOrNullByAbsolutePos (const TCoord &relPosition, /**< relative coordinate in grid */
                                                int time_step_back) /**< index of previous time step, starting from current (0) */
{
  return getFieldValueByAbsolutePos (relPosition, time_step_back);
} /* Grid<TCoord>::getFieldValueOrNullByAbsolutePos */

template <class TCoord>
FieldValue *
Grid<TCoord>::getFieldValueCurrentAfterShiftByAbsolutePos (const TCoord &relPosition) /**< relative coordinate in grid */
{
  return getFieldValue (relPosition, 1);
}

template <class TCoord>
FieldValue *
Grid<TCoord>::getFieldValueOrNullCurrentAfterShiftByAbsolutePos (const TCoord &relPosition) /**< relative coordinate in grid */
{
  return getFieldValueCurrentAfterShiftByAbsolutePos (relPosition);
}

template <class TCoord>
FieldValue *
Grid<TCoord>::getFieldValuePreviousAfterShiftByAbsolutePos (const TCoord &relPosition) /**< relative coordinate in grid */
{
  if (gridValues.size () > 2)
  {
    return getFieldValue (relPosition, 2);
  }
  else
  {
    return getFieldValue (relPosition, 0);
  }
}

template <class TCoord>
FieldValue *
Grid<TCoord>::getFieldValueOrNullPreviousAfterShiftByAbsolutePos (const TCoord &relPosition) /**< relative coordinate in grid */
{
  return getFieldValuePreviousAfterShiftByAbsolutePos (relPosition);
}

/**
 * Get total position in grid. Is equal to position in non-parallel grid
 *
 * @return total position in grid
 */
template <class TCoord>
TCoord
Grid<TCoord>::getTotalPosition (const TCoord & pos) const /**< position in grid */
{
  return pos;
} /* Grid<TCoord>::getTotalPosition */

/**
 * Get total size of grid. Is equal to size in non-parallel grid
 *
 * @return total size of grid
 */
template <class TCoord>
TCoord
Grid<TCoord>::getTotalSize () const
{
  return getSize ();
} /* Grid<TCoord>::getTotalSize */

/**
 * Get relative position in grid. Is equal to position in non-parallel grid
 *
 * @return relative position in grid
 */
template <class TCoord>
TCoord
Grid<TCoord>::getRelativePosition (const TCoord & pos) const /**< position in grid */
{
  return pos;
} /* gGrid<TCoord>::getRelativePosition */

/**
 * Get name of grid
 *
 * @return name of grid
 */
template <class TCoord>
const char *
Grid<TCoord>::getName () const
{
  return gridName.c_str ();
} /* Grid<TCoord>::getName */

/**
 * Initialize current grid field values with default values
 */
template <class TCoord>
void
Grid<TCoord>::initialize (const FieldValue & cur)
{
  ASSERT (gridValues.size () > 0);

  for (grid_coord i = 0; i < gridValues[0]->size (); ++i)
  {
    (*gridValues[0])[i] = cur;
  }
} /* Grid<TCoord>::initialize */

template <class TCoord>
FieldValue *
Grid<TCoord>::getRaw (int time_step_back)
{
  ASSERT (time_step_back < gridValues.size ());

  return &(*gridValues[time_step_back])[0];
}

/**
 * Replace previous time layer with current and so on
 */
template <class TCoord>
void
Grid<TCoord>::shiftInTime ()
{
  /*
   * Reuse oldest grid as new current
   */
  ASSERT (gridValues.size () > 0);

  VectorFieldValues *oldest = gridValues[gridValues.size () - 1];

  for (int i = gridValues.size () - 1; i >= 1; --i)
  {
    gridValues[i] = gridValues[i - 1];
  }

  gridValues[0] = oldest;
} /* Grid<TCoord>::shiftInTime */

#endif /* GRID_H */
