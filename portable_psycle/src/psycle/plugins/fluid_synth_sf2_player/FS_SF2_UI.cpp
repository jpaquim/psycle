#include <project.private.hpp>
#include "fs_sf2_ui.hpp"

IMPLEMENT_DYNAMIC(FS_SF2_UI, CDialog)
FS_SF2_UI::FS_SF2_UI(CWnd* pParent /*=NULL*/)
	: CDialog(FS_SF2_UI::IDD, pParent)
{

}

FS_SF2_UI::~FS_SF2_UI()
{
}

BEGIN_MESSAGE_MAP(FS_SF2_UI, CDialog)
END_MESSAGE_MAP()
