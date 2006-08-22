///\file
///\implementation psycle::host::CMidiInputDlg.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include "psycle.hpp"
#include "OutputDlg.hpp"
#include "MidiInputDlg.hpp"
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/helpers/helpers.hpp>
#include <psycle/engine/MidiInput.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		IMPLEMENT_DYNCREATE(CMidiInputDlg, CPropertyPage)

		CMidiInputDlg::CMidiInputDlg()
		:
			CPropertyPage(CMidiInputDlg::IDD),
			groups(16)
		{
			for(groups_type::iterator i(groups.begin()) ; i != groups.end() ; ++i) *i = new group::with_message;
		}

		CMidiInputDlg::~CMidiInputDlg()
		{
		}

		void CMidiInputDlg::DoDataExchange(CDataExchange* pDX)
		{
			CPropertyPage::DoDataExchange(pDX);

			DDX_Control(pDX, IDC_MIDI_RAW, raw);

			#define $rad_tools_slow_you_down(id, var) \
				DDX_Control(pDX, IDC_MIDI_RECORD_##id , var.record ); \
				DDX_Control(pDX, IDC_MIDI_TYPE_##id   , var.type   ); \
				DDX_Control(pDX, IDC_MIDI_COMMAND_##id, var.command); \
				DDX_Control(pDX, IDC_MIDI_FROM_##id   , var.from   ); \
				DDX_Control(pDX, IDC_MIDI_TO_##id     , var.to     );

			$rad_tools_slow_you_down(VEL, velocity)
			$rad_tools_slow_you_down(PIT, pitch)

			#include <boost/preprocessor/repetition.hpp>
			#define $rad_tools_slow_you_down_loop(_, i, __) \
				DDX_Control(pDX, IDC_MIDI_MESSAGE_##i, groups[i]->message); \
				$rad_tools_slow_you_down(i, (*groups[i]))
			BOOST_PP_REPEAT(16, $rad_tools_slow_you_down_loop, ~)

			#undef $rad_tools_slow_you_down
			#undef $rad_tools_slow_you_down_loop
		}

		BEGIN_MESSAGE_MAP(CMidiInputDlg, CPropertyPage)
		END_MESSAGE_MAP()

		namespace
		{
			template<typename Model>
			void write_to_gui_text(CEdit & gui, Model const & model)
			{
				std::ostringstream s;
				s << std::hex << model;
				gui.SetWindowText(s.str().c_str());
			}
			void write_to_gui(CMidiInputDlg::group & gui, UIConfiguration::midi_type::group_type const & model)
			{
				gui.record.SetCheck(model.record());
				gui.type.SetCurSel(model.type());
				write_to_gui_text(gui.command, model.command());
				write_to_gui_text(gui.from, model.from());
				write_to_gui_text(gui.to, model.to());
			}
		}

		BOOL CMidiInputDlg::OnInitDialog() 
		{
			CPropertyPage::OnInitDialog();

			raw.SetCheck(UIGlobal::configuration().midi().raw());

			velocity.type.AddString("cmd");
			velocity.type.AddString("ins");
			write_to_gui(velocity, UIGlobal::configuration().midi().velocity());

			pitch.type.AddString("cmd");
			pitch.type.AddString("twk");
			pitch.type.AddString("tws");
			pitch.type.AddString("ins");
			pitch.type.AddString("mcm");
			write_to_gui(pitch, UIGlobal::configuration().midi().pitch());

			assert(groups.size() == UIGlobal::configuration().midi().groups().size());
			for(std::size_t i(0) ; i < groups.size() ; ++i)
			{
				groups[i]->type.AddString("cmd");
				groups[i]->type.AddString("twk");
				groups[i]->type.AddString("tws");
				groups[i]->type.AddString("ins");
				groups[i]->type.AddString("mcm");
				write_to_gui(*groups[i], UIGlobal::configuration().midi().group(i));
				write_to_gui_text(groups[i]->message, UIGlobal::configuration().midi().group(i).message());
			}

			return true;
		}

		namespace
		{
			template<typename Model>
			void read_from_gui_text(CEdit const & gui, Model & model)
			{
				CString mfc;
				gui.GetWindowText(mfc);
				hexstring_to_integer(static_cast<char const * const>(mfc), model);
			}
			void read_from_gui(CMidiInputDlg::group const & gui, UIConfiguration::midi_type::group_type & model)
			{
				model.record() = gui.record.GetCheck();
				model.type() = gui.type.GetCurSel();
				read_from_gui_text(gui.command, model.command());
				read_from_gui_text(gui.from, model.from());
				read_from_gui_text(gui.to, model.to());

			}
		}

		void CMidiInputDlg::OnOK() 
		{
			UIGlobal::configuration().midi().raw() = raw.GetCheck();
			read_from_gui(velocity, UIGlobal::configuration().midi().velocity());
			read_from_gui(pitch, UIGlobal::configuration().midi().pitch());
			assert(groups.size() == UIGlobal::configuration().midi().groups().size());
			for(std::size_t i(0) ; i < groups.size() ; ++i)
			{
				read_from_gui(*groups[i], UIGlobal::configuration().midi().group(i));
				read_from_gui_text(groups[i]->message, UIGlobal::configuration().midi().group(i).message());
			}
			CPropertyPage::OnOK();
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
