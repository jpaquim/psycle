#include "stdafx.h"
#include "Psycle.h"
#include "OutputDlg.h"
#include "MidiInput.h"
#include "Configuration.h"
#include "MidiInputDlg.h"
#include "Helpers.h"
///\file
///\brief implementation file for psycle::host::CMidiInputDlg.
namespace psycle
{
	namespace host
	{
		IMPLEMENT_DYNCREATE(CMidiInputDlg, CPropertyPage)

		CMidiInputDlg::CMidiInputDlg() : CPropertyPage(CMidiInputDlg::IDD)
		{
			//{{AFX_DATA_INIT(CMidiInputDlg)
				// NOTE: the ClassWizard will add member initialization here
			//}}AFX_DATA_INIT
		}

		CMidiInputDlg::~CMidiInputDlg()
		{
		}

		void CMidiInputDlg::DoDataExchange(CDataExchange* pDX)
		{
			CPropertyPage::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CMidiInputDlg)
			DDX_Control(pDX, IDC_MIDI_RECORD_VEL, m_midiRecordVelButton);
			DDX_Control(pDX, IDC_MIDI_TYPE_VEL, m_midiTypeVelComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_VEL, m_midiCommandVelEdit);
			DDX_Control(pDX, IDC_MIDI_FROM_VEL, m_midiFromVelEdit);
			DDX_Control(pDX, IDC_MIDI_TO_VEL, m_midiToVelEdit);
			DDX_Control(pDX, IDC_MIDI_RECORD_PIT, m_midiRecordPitButton);
			DDX_Control(pDX, IDC_MIDI_TYPE_PIT, m_midiTypePitComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_PIT, m_midiCommandPitEdit);
			DDX_Control(pDX, IDC_MIDI_FROM_PIT, m_midiFromPitEdit);
			DDX_Control(pDX, IDC_MIDI_TO_PIT, m_midiToPitEdit);
			DDX_Control(pDX, IDC_MIDI_RECORD_0, m_midiRecord0Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_0, m_midiMessage0Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_0, m_midiType0ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_0, m_midiCommand0Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_0, m_midiFrom0Edit);
			DDX_Control(pDX, IDC_MIDI_TO_0, m_midiTo0Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_1, m_midiRecord1Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_1, m_midiMessage1Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_1, m_midiType1ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_1, m_midiCommand1Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_1, m_midiFrom1Edit);
			DDX_Control(pDX, IDC_MIDI_TO_1, m_midiTo1Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_2, m_midiRecord2Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_2, m_midiMessage2Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_2, m_midiType2ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_2, m_midiCommand2Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_2, m_midiFrom2Edit);
			DDX_Control(pDX, IDC_MIDI_TO_2, m_midiTo2Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_3, m_midiRecord3Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_3, m_midiMessage3Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_3, m_midiType3ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_3, m_midiCommand3Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_3, m_midiFrom3Edit);
			DDX_Control(pDX, IDC_MIDI_TO_3, m_midiTo3Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_4, m_midiRecord4Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_4, m_midiMessage4Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_4, m_midiType4ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_4, m_midiCommand4Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_4, m_midiFrom4Edit);
			DDX_Control(pDX, IDC_MIDI_TO_4, m_midiTo4Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_5, m_midiRecord5Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_5, m_midiMessage5Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_5, m_midiType5ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_5, m_midiCommand5Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_5, m_midiFrom5Edit);
			DDX_Control(pDX, IDC_MIDI_TO_5, m_midiTo5Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_6, m_midiRecord6Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_6, m_midiMessage6Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_6, m_midiType6ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_6, m_midiCommand6Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_6, m_midiFrom6Edit);
			DDX_Control(pDX, IDC_MIDI_TO_6, m_midiTo6Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_7, m_midiRecord7Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_7, m_midiMessage7Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_7, m_midiType7ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_7, m_midiCommand7Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_7, m_midiFrom7Edit);
			DDX_Control(pDX, IDC_MIDI_TO_7, m_midiTo7Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_8, m_midiRecord8Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_8, m_midiMessage8Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_8, m_midiType8ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_8, m_midiCommand8Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_8, m_midiFrom8Edit);
			DDX_Control(pDX, IDC_MIDI_TO_8, m_midiTo8Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_9, m_midiRecord9Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_9, m_midiMessage9Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_9, m_midiType9ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_9, m_midiCommand9Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_9, m_midiFrom9Edit);
			DDX_Control(pDX, IDC_MIDI_TO_9, m_midiTo9Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_10, m_midiRecord10Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_10, m_midiMessage10Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_10, m_midiType10ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_10, m_midiCommand10Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_10, m_midiFrom10Edit);
			DDX_Control(pDX, IDC_MIDI_TO_10, m_midiTo10Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_11, m_midiRecord11Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_11, m_midiMessage11Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_11, m_midiType11ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_11, m_midiCommand11Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_11, m_midiFrom11Edit);
			DDX_Control(pDX, IDC_MIDI_TO_11, m_midiTo11Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_12, m_midiRecord12Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_12, m_midiMessage12Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_12, m_midiType12ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_12, m_midiCommand12Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_12, m_midiFrom12Edit);
			DDX_Control(pDX, IDC_MIDI_TO_12, m_midiTo12Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_13, m_midiRecord13Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_13, m_midiMessage13Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_13, m_midiType13ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_13, m_midiCommand13Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_13, m_midiFrom13Edit);
			DDX_Control(pDX, IDC_MIDI_TO_13, m_midiTo13Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_14, m_midiRecord14Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_14, m_midiMessage14Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_14, m_midiType14ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_14, m_midiCommand14Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_14, m_midiFrom14Edit);
			DDX_Control(pDX, IDC_MIDI_TO_14, m_midiTo14Edit);
			DDX_Control(pDX, IDC_MIDI_RECORD_15, m_midiRecord15Button);
			DDX_Control(pDX, IDC_MIDI_MESSAGE_15, m_midiMessage15Edit);
			DDX_Control(pDX, IDC_MIDI_TYPE_15, m_midiType15ComboBox);
			DDX_Control(pDX, IDC_MIDI_COMMAND_15, m_midiCommand15Edit);
			DDX_Control(pDX, IDC_MIDI_FROM_15, m_midiFrom15Edit);
			DDX_Control(pDX, IDC_MIDI_TO_15, m_midiTo15Edit);
			DDX_Control(pDX, IDC_MIDI_RAW, m_midiRawMcm);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CMidiInputDlg, CPropertyPage)
			//{{AFX_MSG_MAP(CMidiInputDlg)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CMidiInputDlg::OnInitDialog() 
		{
			CPropertyPage::OnInitDialog();

			// new midi stuff
			CString str;

			m_midiRecordVelButton.SetCheck(Global::pConfig->_midiRecordVel?1:0);
			m_midiTypeVelComboBox.AddString("cmd");
			m_midiTypeVelComboBox.AddString("ins");
			m_midiTypeVelComboBox.SetCurSel(Global::pConfig->_midiTypeVel?1:0);
			str.Format("%x", Global::pConfig->_midiCommandVel);
			m_midiCommandVelEdit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFromVel);
			m_midiFromVelEdit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiToVel);
			m_midiToVelEdit.SetWindowText(str);

			m_midiRecordPitButton.SetCheck(Global::pConfig->_midiRecordPit?1:0);
			m_midiTypePitComboBox.AddString("cmd");
			m_midiTypePitComboBox.AddString("twk");
			m_midiTypePitComboBox.AddString("tws");
			m_midiTypePitComboBox.AddString("ins");
			m_midiTypePitComboBox.AddString("mcm");
			m_midiTypePitComboBox.SetCurSel(Global::pConfig->_midiTypePit);
			str.Format("%x", Global::pConfig->_midiCommandPit);
			m_midiCommandPitEdit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFromPit);
			m_midiFromPitEdit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiToPit);
			m_midiToPitEdit.SetWindowText(str);

			m_midiRecord0Button.SetCheck(Global::pConfig->_midiRecord0?1:0);
			str.Format("%x", Global::pConfig->_midiMessage0);
			m_midiMessage0Edit.SetWindowText(str);
			m_midiType0ComboBox.AddString("cmd");
			m_midiType0ComboBox.AddString("twk");
			m_midiType0ComboBox.AddString("tws");
			m_midiType0ComboBox.AddString("ins");
			m_midiType0ComboBox.AddString("mcm");
			m_midiType0ComboBox.SetCurSel(Global::pConfig->_midiType0);
			str.Format("%x", Global::pConfig->_midiCommand0);
			m_midiCommand0Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom0);
			m_midiFrom0Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo0);
			m_midiTo0Edit.SetWindowText(str);

			m_midiRecord1Button.SetCheck(Global::pConfig->_midiRecord1?1:0);
			str.Format("%x", Global::pConfig->_midiMessage1);
			m_midiMessage1Edit.SetWindowText(str);
			m_midiType1ComboBox.AddString("cmd");
			m_midiType1ComboBox.AddString("twk");
			m_midiType1ComboBox.AddString("tws");
			m_midiType1ComboBox.AddString("ins");
			m_midiType1ComboBox.AddString("mcm");
			m_midiType1ComboBox.SetCurSel(Global::pConfig->_midiType1);
			str.Format("%x", Global::pConfig->_midiCommand1);
			m_midiCommand1Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom1);
			m_midiFrom1Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo1);
			m_midiTo1Edit.SetWindowText(str);

			m_midiRecord2Button.SetCheck(Global::pConfig->_midiRecord2?1:0);
			str.Format("%x", Global::pConfig->_midiMessage2);
			m_midiMessage2Edit.SetWindowText(str);
			m_midiType2ComboBox.AddString("cmd");
			m_midiType2ComboBox.AddString("twk");
			m_midiType2ComboBox.AddString("tws");
			m_midiType2ComboBox.AddString("ins");
			m_midiType2ComboBox.AddString("mcm");
			m_midiType2ComboBox.SetCurSel(Global::pConfig->_midiType2);
			str.Format("%x", Global::pConfig->_midiCommand2);
			m_midiCommand2Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom2);
			m_midiFrom2Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo2);
			m_midiTo2Edit.SetWindowText(str);

			m_midiRecord3Button.SetCheck(Global::pConfig->_midiRecord3?1:0);
			str.Format("%x", Global::pConfig->_midiMessage3);
			m_midiMessage3Edit.SetWindowText(str);
			m_midiType3ComboBox.AddString("cmd");
			m_midiType3ComboBox.AddString("twk");
			m_midiType3ComboBox.AddString("tws");
			m_midiType3ComboBox.AddString("ins");
			m_midiType3ComboBox.AddString("mcm");
			m_midiType3ComboBox.SetCurSel(Global::pConfig->_midiType3);
			str.Format("%x", Global::pConfig->_midiCommand3);
			m_midiCommand3Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom3);
			m_midiFrom3Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo3);
			m_midiTo3Edit.SetWindowText(str);

			m_midiRecord4Button.SetCheck(Global::pConfig->_midiRecord4?1:0);
			str.Format("%x", Global::pConfig->_midiMessage4);
			m_midiMessage4Edit.SetWindowText(str);
			m_midiType4ComboBox.AddString("cmd");
			m_midiType4ComboBox.AddString("twk");
			m_midiType4ComboBox.AddString("tws");
			m_midiType4ComboBox.AddString("ins");
			m_midiType4ComboBox.AddString("mcm");
			m_midiType4ComboBox.SetCurSel(Global::pConfig->_midiType4);
			str.Format("%x", Global::pConfig->_midiCommand4);
			m_midiCommand4Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom4);
			m_midiFrom4Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo4);
			m_midiTo4Edit.SetWindowText(str);

			m_midiRecord5Button.SetCheck(Global::pConfig->_midiRecord5?1:0);
			str.Format("%x", Global::pConfig->_midiMessage5);
			m_midiMessage5Edit.SetWindowText(str);
			m_midiType5ComboBox.AddString("cmd");
			m_midiType5ComboBox.AddString("twk");
			m_midiType5ComboBox.AddString("tws");
			m_midiType5ComboBox.AddString("ins");
			m_midiType5ComboBox.AddString("mcm");
			m_midiType5ComboBox.SetCurSel(Global::pConfig->_midiType5);
			str.Format("%x", Global::pConfig->_midiCommand5);
			m_midiCommand5Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom5);
			m_midiFrom5Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo5);
			m_midiTo5Edit.SetWindowText(str);

			m_midiRecord6Button.SetCheck(Global::pConfig->_midiRecord6?1:0);
			str.Format("%x", Global::pConfig->_midiMessage6);
			m_midiMessage6Edit.SetWindowText(str);
			m_midiType6ComboBox.AddString("cmd");
			m_midiType6ComboBox.AddString("twk");
			m_midiType6ComboBox.AddString("tws");
			m_midiType6ComboBox.AddString("ins");
			m_midiType6ComboBox.AddString("mcm");
			m_midiType6ComboBox.SetCurSel(Global::pConfig->_midiType6);
			str.Format("%x", Global::pConfig->_midiCommand6);
			m_midiCommand6Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom6);
			m_midiFrom6Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo6);
			m_midiTo6Edit.SetWindowText(str);

			m_midiRecord7Button.SetCheck(Global::pConfig->_midiRecord7?1:0);
			str.Format("%x", Global::pConfig->_midiMessage7);
			m_midiMessage7Edit.SetWindowText(str);
			m_midiType7ComboBox.AddString("cmd");
			m_midiType7ComboBox.AddString("twk");
			m_midiType7ComboBox.AddString("tws");
			m_midiType7ComboBox.AddString("ins");
			m_midiType7ComboBox.AddString("mcm");
			m_midiType7ComboBox.SetCurSel(Global::pConfig->_midiType7);
			str.Format("%x", Global::pConfig->_midiCommand7);
			m_midiCommand7Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom7);
			m_midiFrom7Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo7);
			m_midiTo7Edit.SetWindowText(str);

			m_midiRecord8Button.SetCheck(Global::pConfig->_midiRecord8?1:0);
			str.Format("%x", Global::pConfig->_midiMessage8);
			m_midiMessage8Edit.SetWindowText(str);
			m_midiType8ComboBox.AddString("cmd");
			m_midiType8ComboBox.AddString("twk");
			m_midiType8ComboBox.AddString("tws");
			m_midiType8ComboBox.AddString("ins");
			m_midiType8ComboBox.AddString("mcm");
			m_midiType8ComboBox.SetCurSel(Global::pConfig->_midiType8);
			str.Format("%x", Global::pConfig->_midiCommand8);
			m_midiCommand8Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom8);
			m_midiFrom8Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo8);
			m_midiTo8Edit.SetWindowText(str);

			m_midiRecord9Button.SetCheck(Global::pConfig->_midiRecord9?1:0);
			str.Format("%x", Global::pConfig->_midiMessage9);
			m_midiMessage9Edit.SetWindowText(str);
			m_midiType9ComboBox.AddString("cmd");
			m_midiType9ComboBox.AddString("twk");
			m_midiType9ComboBox.AddString("tws");
			m_midiType9ComboBox.AddString("ins");
			m_midiType9ComboBox.AddString("mcm");
			m_midiType9ComboBox.SetCurSel(Global::pConfig->_midiType9);
			str.Format("%x", Global::pConfig->_midiCommand9);
			m_midiCommand9Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom9);
			m_midiFrom9Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo9);
			m_midiTo9Edit.SetWindowText(str);

			m_midiRecord10Button.SetCheck(Global::pConfig->_midiRecord10?1:0);
			str.Format("%x", Global::pConfig->_midiMessage10);
			m_midiMessage10Edit.SetWindowText(str);
			m_midiType10ComboBox.AddString("cmd");
			m_midiType10ComboBox.AddString("twk");
			m_midiType10ComboBox.AddString("tws");
			m_midiType10ComboBox.AddString("ins");
			m_midiType10ComboBox.AddString("mcm");
			m_midiType10ComboBox.SetCurSel(Global::pConfig->_midiType10);
			str.Format("%x", Global::pConfig->_midiCommand10);
			m_midiCommand10Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom10);
			m_midiFrom10Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo10);
			m_midiTo10Edit.SetWindowText(str);

			m_midiRecord11Button.SetCheck(Global::pConfig->_midiRecord11?1:0);
			str.Format("%x", Global::pConfig->_midiMessage11);
			m_midiMessage11Edit.SetWindowText(str);
			m_midiType11ComboBox.AddString("cmd");
			m_midiType11ComboBox.AddString("twk");
			m_midiType11ComboBox.AddString("tws");
			m_midiType11ComboBox.AddString("ins");
			m_midiType11ComboBox.AddString("mcm");
			m_midiType11ComboBox.SetCurSel(Global::pConfig->_midiType11);
			str.Format("%x", Global::pConfig->_midiCommand11);
			m_midiCommand11Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom11);
			m_midiFrom11Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo11);
			m_midiTo11Edit.SetWindowText(str);

			m_midiRecord12Button.SetCheck(Global::pConfig->_midiRecord12?1:0);
			str.Format("%x", Global::pConfig->_midiMessage12);
			m_midiMessage12Edit.SetWindowText(str);
			m_midiType12ComboBox.AddString("cmd");
			m_midiType12ComboBox.AddString("twk");
			m_midiType12ComboBox.AddString("tws");
			m_midiType12ComboBox.AddString("ins");
			m_midiType12ComboBox.AddString("mcm");
			m_midiType12ComboBox.SetCurSel(Global::pConfig->_midiType12);
			str.Format("%x", Global::pConfig->_midiCommand12);
			m_midiCommand12Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom12);
			m_midiFrom12Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo12);
			m_midiTo12Edit.SetWindowText(str);

			m_midiRecord13Button.SetCheck(Global::pConfig->_midiRecord13?1:0);
			str.Format("%x", Global::pConfig->_midiMessage13);
			m_midiMessage13Edit.SetWindowText(str);
			m_midiType13ComboBox.AddString("cmd");
			m_midiType13ComboBox.AddString("twk");
			m_midiType13ComboBox.AddString("tws");
			m_midiType13ComboBox.AddString("ins");
			m_midiType13ComboBox.AddString("mcm");
			m_midiType13ComboBox.SetCurSel(Global::pConfig->_midiType13);
			str.Format("%x", Global::pConfig->_midiCommand13);
			m_midiCommand13Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom13);
			m_midiFrom13Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo13);
			m_midiTo13Edit.SetWindowText(str);

			m_midiRecord14Button.SetCheck(Global::pConfig->_midiRecord14?1:0);
			str.Format("%x", Global::pConfig->_midiMessage14);
			m_midiMessage14Edit.SetWindowText(str);
			m_midiType14ComboBox.AddString("cmd");
			m_midiType14ComboBox.AddString("twk");
			m_midiType14ComboBox.AddString("tws");
			m_midiType14ComboBox.AddString("ins");
			m_midiType14ComboBox.AddString("mcm");
			m_midiType14ComboBox.SetCurSel(Global::pConfig->_midiType14);
			str.Format("%x", Global::pConfig->_midiCommand14);
			m_midiCommand14Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom14);
			m_midiFrom14Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo14);
			m_midiTo14Edit.SetWindowText(str);

			m_midiRecord15Button.SetCheck(Global::pConfig->_midiRecord15?1:0);
			str.Format("%x", Global::pConfig->_midiMessage15);
			m_midiMessage15Edit.SetWindowText(str);
			m_midiType15ComboBox.AddString("cmd");
			m_midiType15ComboBox.AddString("twk");
			m_midiType15ComboBox.AddString("tws");
			m_midiType15ComboBox.AddString("ins");
			m_midiType15ComboBox.AddString("mcm");
			m_midiType15ComboBox.SetCurSel(Global::pConfig->_midiType15);
			str.Format("%x", Global::pConfig->_midiCommand15);
			m_midiCommand15Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiFrom15);
			m_midiFrom15Edit.SetWindowText(str);
			str.Format("%x", Global::pConfig->_midiTo15);
			m_midiTo15Edit.SetWindowText(str);

			m_midiRawMcm.SetCheck(Global::pConfig->_midiRawMcm?1:0);

			return TRUE;
			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
		}

		void CMidiInputDlg::OnOK() 
		{
			char str[7];

			Global::pConfig->_midiRecordVel = m_midiRecordVelButton.GetCheck()?true:false;
			m_midiCommandVelEdit.GetWindowText(str,6);
			Global::pConfig->_midiTypeVel = m_midiTypeVelComboBox.GetCurSel()?3:0;

			Global::pConfig->_midiCommandVel = _httoi(str);
			m_midiFromVelEdit.GetWindowText(str,6);
			Global::pConfig->_midiFromVel = _httoi(str);
			m_midiToVelEdit.GetWindowText(str,6);
			Global::pConfig->_midiToVel = _httoi(str);

			Global::pConfig->_midiRecordPit = m_midiRecordPitButton.GetCheck()?true:false;
			Global::pConfig->_midiTypePit = m_midiTypePitComboBox.GetCurSel();
			m_midiCommandPitEdit.GetWindowText(str,6);
			Global::pConfig->_midiCommandPit = _httoi(str);
			m_midiFromPitEdit.GetWindowText(str,6);
			Global::pConfig->_midiFromPit = _httoi(str);
			m_midiToPitEdit.GetWindowText(str,6);
			Global::pConfig->_midiToPit = _httoi(str);

			Global::pConfig->_midiRecord0 = m_midiRecord0Button.GetCheck()?true:false;
			m_midiMessage0Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage0 = _httoi(str);
			Global::pConfig->_midiType0 = m_midiType0ComboBox.GetCurSel();
			m_midiCommand0Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand0 = _httoi(str);
			m_midiFrom0Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom0 = _httoi(str);
			m_midiTo0Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo0 = _httoi(str);

			Global::pConfig->_midiRecord1 = m_midiRecord1Button.GetCheck()?true:false;
			m_midiMessage1Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage1 = _httoi(str);
			Global::pConfig->_midiType1 = m_midiType1ComboBox.GetCurSel();
			m_midiCommand1Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand1 = _httoi(str);
			m_midiFrom1Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom1 = _httoi(str);
			m_midiTo1Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo1 = _httoi(str);

			Global::pConfig->_midiRecord2 = m_midiRecord2Button.GetCheck()?true:false;
			m_midiMessage2Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage2 = _httoi(str);
			Global::pConfig->_midiType2 = m_midiType2ComboBox.GetCurSel();
			m_midiCommand2Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand2 = _httoi(str);
			m_midiFrom2Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom2 = _httoi(str);
			m_midiTo2Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo2 = _httoi(str);

			Global::pConfig->_midiRecord3 = m_midiRecord3Button.GetCheck()?true:false;
			m_midiMessage3Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage3 = _httoi(str);
			Global::pConfig->_midiType3 = m_midiType3ComboBox.GetCurSel();
			m_midiCommand3Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand3 = _httoi(str);
			m_midiFrom3Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom3 = _httoi(str);
			m_midiTo3Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo3 = _httoi(str);

			Global::pConfig->_midiRecord4 = m_midiRecord4Button.GetCheck()?true:false;
			m_midiMessage4Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage4 = _httoi(str);
			Global::pConfig->_midiType4 = m_midiType4ComboBox.GetCurSel();
			m_midiCommand4Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand4 = _httoi(str);
			m_midiFrom4Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom4 = _httoi(str);
			m_midiTo4Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo4 = _httoi(str);

			Global::pConfig->_midiRecord5 = m_midiRecord5Button.GetCheck()?true:false;
			m_midiMessage5Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage5 = _httoi(str);
			Global::pConfig->_midiType5 = m_midiType5ComboBox.GetCurSel();
			m_midiCommand5Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand5 = _httoi(str);
			m_midiFrom5Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom5 = _httoi(str);
			m_midiTo5Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo5 = _httoi(str);

			Global::pConfig->_midiRecord6 = m_midiRecord6Button.GetCheck()?true:false;
			m_midiMessage6Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage6 = _httoi(str);
			Global::pConfig->_midiType6 = m_midiType6ComboBox.GetCurSel();
			m_midiCommand6Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand6 = _httoi(str);
			m_midiFrom6Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom6 = _httoi(str);
			m_midiTo6Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo6 = _httoi(str);

			Global::pConfig->_midiRecord7 = m_midiRecord7Button.GetCheck()?true:false;
			m_midiMessage7Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage7 = _httoi(str);
			Global::pConfig->_midiType7 = m_midiType7ComboBox.GetCurSel();
			m_midiCommand7Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand7 = _httoi(str);
			m_midiFrom7Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom7 = _httoi(str);
			m_midiTo7Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo7 = _httoi(str);

			Global::pConfig->_midiRecord8 = m_midiRecord8Button.GetCheck()?true:false;
			m_midiMessage8Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage8 = _httoi(str);
			Global::pConfig->_midiType8 = m_midiType8ComboBox.GetCurSel();
			m_midiCommand8Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand8 = _httoi(str);
			m_midiFrom8Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom8 = _httoi(str);
			m_midiTo8Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo8 = _httoi(str);

			Global::pConfig->_midiRecord9 = m_midiRecord9Button.GetCheck()?true:false;
			m_midiMessage9Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage9 = _httoi(str);
			Global::pConfig->_midiType9 = m_midiType9ComboBox.GetCurSel();
			m_midiCommand9Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand9 = _httoi(str);
			m_midiFrom9Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom9 = _httoi(str);
			m_midiTo9Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo9 = _httoi(str);

			Global::pConfig->_midiRecord10 = m_midiRecord10Button.GetCheck()?true:false;
			m_midiMessage10Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage10 = _httoi(str);
			Global::pConfig->_midiType10 = m_midiType10ComboBox.GetCurSel();
			m_midiCommand10Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand10 = _httoi(str);
			m_midiFrom10Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom10 = _httoi(str);
			m_midiTo10Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo10 = _httoi(str);

			Global::pConfig->_midiRecord11 = m_midiRecord11Button.GetCheck()?true:false;
			m_midiMessage11Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage11 = _httoi(str);
			Global::pConfig->_midiType11 = m_midiType11ComboBox.GetCurSel();
			m_midiCommand11Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand11 = _httoi(str);
			m_midiFrom11Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom11 = _httoi(str);
			m_midiTo11Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo11 = _httoi(str);

			Global::pConfig->_midiRecord12 = m_midiRecord12Button.GetCheck()?true:false;
			m_midiMessage12Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage12 = _httoi(str);
			Global::pConfig->_midiType12 = m_midiType12ComboBox.GetCurSel();
			m_midiCommand12Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand12 = _httoi(str);
			m_midiFrom12Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom12 = _httoi(str);
			m_midiTo12Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo12 = _httoi(str);

			Global::pConfig->_midiRecord13 = m_midiRecord13Button.GetCheck()?true:false;
			m_midiMessage13Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage13 = _httoi(str);
			Global::pConfig->_midiType13 = m_midiType13ComboBox.GetCurSel();
			m_midiCommand13Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand13 = _httoi(str);
			m_midiFrom13Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom13 = _httoi(str);
			m_midiTo13Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo13 = _httoi(str);

			Global::pConfig->_midiRecord14 = m_midiRecord14Button.GetCheck()?true:false;
			m_midiMessage14Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage14 = _httoi(str);
			Global::pConfig->_midiType14 = m_midiType14ComboBox.GetCurSel();
			m_midiCommand14Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand14 = _httoi(str);
			m_midiFrom14Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom14 = _httoi(str);
			m_midiTo14Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo14 = _httoi(str);

			Global::pConfig->_midiRecord15 = m_midiRecord15Button.GetCheck()?true:false;
			m_midiMessage15Edit.GetWindowText(str,6);
			Global::pConfig->_midiMessage15 = _httoi(str);
			Global::pConfig->_midiType15 = m_midiType15ComboBox.GetCurSel();
			m_midiCommand15Edit.GetWindowText(str,6);
			Global::pConfig->_midiCommand15 = _httoi(str);
			m_midiFrom15Edit.GetWindowText(str,6);
			Global::pConfig->_midiFrom15 = _httoi(str);
			m_midiTo15Edit.GetWindowText(str,6);
			Global::pConfig->_midiTo15 = _httoi(str);

			Global::pConfig->_midiRawMcm = m_midiRawMcm.GetCheck();

			CPropertyPage::OnOK();
		}
	}
}
