///\file
///\brief implementation file for psycle::host::CGreetDialog.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/Psycle.hpp>
#include <psycle/host/GreetDialog.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		CGreetDialog::CGreetDialog(CWnd* pParent /* = 0 */) : CDialog(CGreetDialog::IDD, pParent)
		{
			//{{AFX_DATA_INIT(CGreetDialog)
				// NOTE: the ClassWizard will add member initialization here
			//}}AFX_DATA_INIT
		}

		void CGreetDialog::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CGreetDialog)
			DDX_Control(pDX, IDC_LIST1, m_greetz);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CGreetDialog, CDialog)
			//{{AFX_MSG_MAP(CGreetDialog)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		BOOL CGreetDialog::OnInitDialog() 
		{
			CDialog::OnInitDialog();
		/*
			//Original Arguru's Greetings.
			m_greetz.AddString("Hamarr Heylen 'Hymax' [Logo design]");
			m_greetz.AddString("Raul Reales 'DJLaser'");
			m_greetz.AddString("Fco. Portillo 'Titan3_4'");
			m_greetz.AddString("Juliole");
			m_greetz.AddString("Sergio 'Zuprimo'");
			m_greetz.AddString("Oskari Tammelin [buzz creator]");
			m_greetz.AddString("Amir Geva 'Photon'");
			m_greetz.AddString("WhiteNoize");
			m_greetz.AddString("Zephod");
			m_greetz.AddString("Felix Petrescu 'WakaX'");
			m_greetz.AddString("Spiril at #goa [EFNET]");
			m_greetz.AddString("Joselex 'Americano'");
			m_greetz.AddString("Lach-ST2");
			m_greetz.AddString("DrDestral");
			m_greetz.AddString("Ic3man");
			m_greetz.AddString("Osirix");
			m_greetz.AddString("Mulder3");
			m_greetz.AddString("HexDump");
			m_greetz.AddString("Robotico");
			m_greetz.AddString("Krzysztof Foltman [FSM]");

			m_greetz.AddString("All #track at Irc-Hispano");

		*/
			m_greetz.AddString("All the people in the Forums");
			m_greetz.AddString("All at #psycle [EFnet]");

			m_greetz.AddString("Alk [Extreme testing + Coding]");
			m_greetz.AddString("BigTick [for his excellent VSTs]");
			m_greetz.AddString("bohan");
			m_greetz.AddString("Byte");
			m_greetz.AddString("CyanPhase [for porting VibraSynth]");
			m_greetz.AddString("dazld");
			m_greetz.AddString("dj_d [Beta testing]");
			m_greetz.AddString("DJMirage");
			m_greetz.AddString("Drax_D [for asking to be here ;·D]");
			m_greetz.AddString("Druttis [Machines]");
			m_greetz.AddString("Erodix");
			m_greetz.AddString("Felix Kaplan / Spirit Of India");
			m_greetz.AddString("Felix Petrescu 'WakaX'");
			m_greetz.AddString("Gerwin / FreeH2o");
			m_greetz.AddString("Imagineer");
			m_greetz.AddString("[JAZ] [Our Project Father!]");
			m_greetz.AddString("KooPer");
			m_greetz.AddString("Krzysztof Foltman / fsm [Coding help]");
			m_greetz.AddString("krokpitr");
			m_greetz.AddString("ksn [Psycledelics WebMaster]");
			m_greetz.AddString("lastfuture");
			m_greetz.AddString("LegoStar [asio]");
			m_greetz.AddString("Loby [for being away]");
			m_greetz.AddString("Pikari");
			m_greetz.AddString("pooplog [Machines + Coding]");
			m_greetz.AddString("sampler");
			m_greetz.AddString("[SAS] SOLARiS [project-psy WebMaster]");
			m_greetz.AddString("_sue_ [Extreme testing]");
			m_greetz.AddString("TAo-AAS");
			m_greetz.AddString("TimEr [Site Graphics and more]");
			m_greetz.AddString("Vir|us");
			return true;
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
