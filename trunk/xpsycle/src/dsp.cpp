#include "dsp.h"


///\file
///\brief implementation file for psycle::host::Cubic.




namespace dsp
{
  //
  // #if defined PSYCLE__CONFIGURATION__RMS_VUS	
  //  int numRMSSamples=1;
  //  int countRMSSamples=0;
  //  double RMSAccumulatedLeft=0;
  //  double RMSAccumulatedRight=0;
  //  float previousRMSLeft=0;
  //  float previousRMSRight=0;
  // #endif

  int Cubic::_resolution;
  float Cubic::_aTable[CUBIC_RESOLUTION];
  float Cubic::_bTable[CUBIC_RESOLUTION];
  float Cubic::_cTable[CUBIC_RESOLUTION];
  float Cubic::_dTable[CUBIC_RESOLUTION];
  float Cubic::_lTable[CUBIC_RESOLUTION];

  Cubic::Cubic()
  {
    _resolution = CUBIC_RESOLUTION;
    // Initialize tables
    for(int i=0; i<_resolution; i++)
    {
      float x = (float)i/(float)_resolution;
      _aTable[i] = float(-0.5*x*x*x +     x*x - 0.5*x);
      _bTable[i] = float( 1.5*x*x*x - 2.5*x*x         + 1);
      _cTable[i] = float(-1.5*x*x*x + 2.0*x*x + 0.5*x);
      _dTable[i] = float( 0.5*x*x*x - 0.5*x*x);
      _lTable[i] = x;
    }
   }
}
