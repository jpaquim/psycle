#include "atlddx.h"


#define DDX_COMBO_INDEX(nID, var) \
 if(nCtlID == (UINT)-1 || nCtlID == nID) \
 { \
  if(!DDX_Combo_Index(nID, var, TRUE, bSaveAndValidate)) \
   return FALSE; \
 }


