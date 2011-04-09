///\file
///\brief interface file for psycle::host::InputHandler.
#pragma once
#include "Psycle.hpp"
#include "PsycleConfig.hpp"

namespace psycle
{
	namespace host
	{
		class CChildView;
		class CMainFrm;
		class Machine;

		/// input handler, keyboard configuration.
		class InputHandler  
		{
		public:
			/// constructor.
			InputHandler();
			virtual ~InputHandler();
		public:
			void SetChildView(CChildView* p) { pChildView=p; }
			void SetMainFrame(CMainFrame* p) { pMainFrame=p; }
		private:
			CChildView* pChildView;
			CMainFrame* pMainFrame;
		public:	
			///\name translation
			///\{
			/// .
			void CmdToKey(CmdSet cse,WORD & key,WORD & mods);	
			/// .
			CmdDef KeyToCmd(UINT nChar, UINT nFlags);
			/// .
			CmdDef StringToCmd(LPCTSTR str);
			///\}
		public:
			/// control 	
			void PerformCmd(CmdDef &cmd,BOOL brepeat);
			
			///\name commands
			///\{
			/// .
			bool EnterData(UINT nChar,UINT nFlags);
			/// .
			void PlayNote(int note,int instr=255, int velocity=127,bool bTranspose=true,Machine*pMachine=NULL);
			/// .
			void StopNote(int note,int instr=255, bool bTranspose=true,Machine*pMachine=NULL);
			///\}

			void Stop();
			void PlaySong();
			void PlayFromCur();
		private:
			/// get key modifier index.
			UINT GetModifierIdx(UINT nFlags)
			{
				UINT idx=0;
				if(GetKeyState(VK_SHIFT)<0) idx|=MOD_S;		// shift?
				if(GetKeyState(VK_CONTROL)<0) idx|=MOD_C;	// ctrl?
				if(nFlags&(1<<8)) idx|=MOD_E;				// extended?
				return idx;
			}
		public:	
			/// Indicates that Shift+Arrow is Selection.
			bool bDoingSelection;		

			/// multi-key playback state stuff
			int notetrack[MAX_TRACKS];
			int instrtrack[MAX_TRACKS];
			int mactrack[MAX_TRACKS];
			/// last track output to	
			int outtrack;
		};
	}
}
