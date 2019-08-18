#if !defined(nameVIEW)
#define nameVIEW

#include <windows.h>
#include <uicomponent.h>

typedef struct {
   ui_component component;   
} nameView;

void InitnameView(nameView* nameview, ui_component* parent);

#endif