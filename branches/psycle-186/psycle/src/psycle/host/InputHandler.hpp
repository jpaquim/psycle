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

		class SPatternUndo
		{
		public:
			int type;
			SPatternUndo* pPrev;
			unsigned char* pData;
			int pattern;
			int x;
			int y;
			int	tracks;
			int	lines;
			// store positional data plz
			int edittrack;
			int editline;
			int editcol;
			int seqpos;
			// counter for tracking, works like ID
			int counter;
		};


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

			void AddUndo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=true, int counter=0);
			void AddRedo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, int counter);
			void AddUndoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=true, int counter=0);
			void AddRedoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, int counter);
			void AddUndoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=true, int counter=0);
			void AddRedoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, int counter);
			void AddUndoSong(int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo=true, int counter=0);
			void AddRedoSong(int edittrack, int editline, int editcol, int seqpos, int counter);
			void AddMacViewUndo(); // place holder
			void KillUndo();
			void KillRedo();
			bool IsModified();
			bool HasUndo(int viewMode);
			bool HasRedo(int viewMode);
			void SafePoint();

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

			SPatternUndo * pUndoList;
			SPatternUndo * pRedoList;

			int UndoCounter;
			int UndoSaved;

			int UndoMacCounter;
			int UndoMacSaved;
		};
	}
}
