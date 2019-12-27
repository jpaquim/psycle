#if !defined(nameVIEW)
#define nameVIEW

#include <windows.h>
#include <uicomponent.h>

typedef struct {
   psy_ui_Component component;   
} nameView;

void InitnameView(nameView* nameview, psy_ui_Component* parent);

#endif