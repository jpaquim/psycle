/***************************************************************************
*   Copyright (C) 2006 by Stefan Nattkemper  *
*   natti@linux   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "framemachine.h"
#include "machine.h"
#include "skinreader.h"
#include "defaultbitmaps.h"
#include "configuration.h"
#include "presetsdlg.h"
#include <ngrs/napp.h>
#include <ngrs/nmenubar.h>
#include <ngrs/ntogglepanel.h>
#include <ngrs/nbutton.h>
#include <ngrs/nedit.h>
#include <ngrs/nfiledialog.h>
#include <ngrs/nslider.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/ntablelayout.h>
#include <ngrs/nalignconstraint.h>


#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace psycle {
	namespace host {


		NewNameDlg::NewNameDlg( )
		{

			NPanel* btnPnl = new NPanel();
			btnPnl->setLayout(NFlowLayout(nAlRight,5,5));
			btnPnl->add(new NButton("add"));
			btnPnl->add(new NButton("cancel"));
			pane()->add(btnPnl,nAlBottom);

			NEdit* name = new NEdit();
			name->setText("userprs");
			pane()->add(name,nAlClient);

			setPosition(100,100,300,60);
		}

		NewNameDlg::~ NewNameDlg( )
		{
		}

		// base class for cells

		Cell::Cell() {
			
		}

		Cell::~Cell() {
		}


		void Cell::paint( NGraphics * g )
		{
			int CW = clientWidth();
			int CH = clientHeight();

			g->setForeground( SkinReader::Instance()->framemachine_info().machineGUITopColor );
			g->fillRect(0,0,CW, CH / 2);
			g->setForeground( SkinReader::Instance()->framemachine_info().machineGUIBottomColor );
			g->fillRect(0,CH/2,CW, CH / 2);
		}

		int Cell::preferredWidth() const {
			return 100 + borderLeft() + borderRight();
		}

		int Cell::preferredHeight() const {
			return K_YSIZE + borderTop() + borderBottom() ;
		}

		// end of Cell class


		// class for a knob
		Knob::Knob(int param )  : max_range(100), min_range(0), value_(0), istweak(0), finetweak(1), ultrafinetweak(0), sourcepoint(0)  {
			setSpacing( NSize( 0,0,1,0 ));			
			param_ = param;  
			add( label = new NLabel());
			add( vLabel = new NLabel()); // the label that shows the value in %
			NFont font = NFont("6x13",6,nMedium | nStraight | nAntiAlias);
			font.setTextColor( SkinReader::Instance()->framemachine_info().machineGUIFontBottomColor );
			vLabel->setFont(font);
			tvalue =stringify(value_)+"%";
			setValueAsText(tvalue);
			font.setTextColor( SkinReader::Instance()->framemachine_info().machineGUIFontTopColor );
			label->setFont(font);
		}

		void Knob::setValue( int value )
		{
			value_ = value;
			if (tvalue=="") { 
				tvalue = stringify(value_)+"%";
				setValueAsText(tvalue);
			}
		}

		void Knob::setRange( int min, int max )
		{
			min_range = min;
			max_range = max;
		}

		void Knob::paint( NGraphics * g )
		{
			Cell::paint(g);

			int amp = max_range - min_range;
			int rel = value_ - min_range;

			if (amp > 0) {
				int frame = (K_NUMFRAMES*rel)/amp;
				int xn = frame*K_XSIZE;
				int CH = clientHeight();

				g->putBitmap(0,(CH - K_YSIZE)/2,K_XSIZE,K_YSIZE, SkinReader::Instance()->defaultBitmaps().tbmainknob(), xn,0);
			}
		}

		void Knob::setText( const std::string & text )
		{
			label->setText(text);
		}

		void Knob::resize( )
		{
			int CH = clientHeight();

			label->setPosition(K_XSIZE+5,0,clientWidth()-K_XSIZE-5,CH/2);
			vLabel->setPosition(K_XSIZE+5,CH/2,clientWidth()-K_XSIZE-5,CH/2);
		}

		int Knob::preferredHeight( ) const
		{
			NFontMetrics metrics(font());
			return std::max(2*metrics.textHeight(),K_YSIZE) + borderTop() + borderBottom() + spacing().top() + spacing().bottom();
		}

		int Knob::preferredWidth( ) const
		{
			NFontMetrics mtr(font());
			return K_XSIZE + std::max(mtr.textWidth(vLabel->text()),mtr.textWidth(label->text())) + 10 + borderRight() + borderLeft() + spacing().left() + spacing().right();
		}		

		void Knob::setValueAsText( const std::string & text )
		{
			std::string bla = text;
			tvalue = bla;
			tvalue = text;
			vLabel->setText(tvalue);
		}

		void Knob::onMousePress( int x, int y, int button )
		{
			int CH = clientHeight();
			if (NRect(0,(CH - K_YSIZE)/2,K_XSIZE,K_YSIZE).intersects(x,y)) {
				istweak = true;
				sourcepoint = y;
				tweakbase = value_;
			}
		}

		void Knob::onMouseOver( int x, int y )
		{
			if (istweak) {

				int shift = NApp::system().shiftState();

				if (( ultrafinetweak && !( shift & nsShift )) || //shift-key has been left.
					( !ultrafinetweak && ( shift & nsShift ))) //shift-key has just been pressed
				{
					sourcepoint=y;
					ultrafinetweak=!ultrafinetweak;
					tweakbase = value_;
				}
				else if (( finetweak && !( shift & nsCtrl )) || //control-key has been left.
					( !finetweak && ( shift & nsCtrl ))) //control-key has just been pressed
				{
					sourcepoint = y;
					finetweak=!finetweak;
					tweakbase=value_;
				}

				int maxval = max_range;
				int minval = min_range;

				int screenh = NApp::system().screenHeight();
				double freak = 0.5;
				if ( ultrafinetweak ) freak = 0.5f;
				else if (maxval-minval < screenh/4) freak = (maxval-minval)/float(screenh/4);
				else if (maxval-minval < screenh*2/3) freak = (maxval-minval)/float(screenh/3);
				else freak = (maxval-minval)/float(screenh*3/5);

				if (finetweak) freak/=5;

				double nv = (double)(sourcepoint - y)*freak + (double)tweakbase;
				if (nv < minval) nv = minval;
				if (nv > maxval) nv = maxval;

				value_ = (int) nv;
				valueChanged.emit(this,(int) nv, param_);
			}
		}

		void Knob::onMousePressed( int x, int y, int button )
		{
			istweak = false;
		}
		// end of Knob class




		// Header Label class
		Header::Header( ) : Cell()
		{
			NFrameBorder border;
				border.setSpacing( NSize(2,2,2,2) );
				border.setOval();
			setBorder( border );

			label = new NLabel();
			label->setTransparent(false);
			label->setBackground( SkinReader::Instance()->framemachine_info().machineGUITitleColor );
			NFont font = NFont("6x13",6,nMedium | nStraight | nAntiAlias);
			font.setTextColor( SkinReader::Instance()->framemachine_info().machineGUITitleFontColor );
			label->setFont( font );
			add(label);
		}

		void Header::setText( const std::string & text ) {
			label->setText( text );
		}

		void Header::resize() {
			int ch = clientHeight();
			int lh = label->preferredHeight();
			label->setPosition(0,(ch - lh) / 2,clientWidth(),lh);
		}
		// end of Header label class


		FrameMachine::FrameMachine(Machine* pMachine)
			: NWindow()
		{
			pMachine_ = pMachine;

			init();
			initParameterGUI();
			loadPresets();
		}


		FrameMachine::~FrameMachine()
		{
		}

		int FrameMachine::onClose( )
		{
			setVisible(false);
			return nHideWindow;
		}

		void FrameMachine::init( )
		{
			//  NMenuBar* bar = new NMenuBar();
			//    pane()->add(bar);
			/*  NMenu* aboutMenu = new NMenu("About",'a',"Help,|,About this machine");
			aboutMenu->itemClicked.connect(this, &FrameMachine::onItemClicked);
			bar->add(aboutMenu);
			NMenu* parameterMenu = new NMenu("Parameters",'p',"Reset,Random,Presets");
			parameterMenu->itemClicked.connect(this, &FrameMachine::onItemClicked);
			bar->add(parameterMenu);*/

			setTitle(stringify(pMachine_->_macIndex)+std::string(" : ")+pMachine_->GetName());

			NPanel* prs = new NPanel();
			prs->setLayout(NAlignLayout());
			NButton* savePrsBtn = new NButton("save");
			savePrsBtn->setFlat(false);
			prs->add(savePrsBtn,nAlLeft);
			NButton* loadPrsBtn = new NButton("load");
			loadPrsBtn->setFlat(false);
			loadPrsBtn->clicked.connect(this,&FrameMachine::onLoadPrs);
			prs->add(loadPrsBtn,nAlLeft);
			NButton* addPrsBtn = new NButton("add");
			addPrsBtn->setFlat(false);
			addPrsBtn->clicked.connect(this,&FrameMachine::onAddPrs);
			prs->add(addPrsBtn,nAlLeft);
			defaultPrsBtn = new NButton("User");
			defaultPrsBtn->setFlat(false);
			prs->add(defaultPrsBtn,nAlLeft);
			NButton* rightPrsBtn = new NButton(">");
			rightPrsBtn->setFlat(false);
			rightPrsBtn->click.connect(this,&FrameMachine::onRightBtn);
			rightPrsBtn->setRepeatMode(true);
			prs->add(rightPrsBtn,nAlRight);
			NButton* leftPrsBtn = new NButton("<");
			leftPrsBtn->setFlat(false);
			leftPrsBtn->setRepeatMode(true);
			leftPrsBtn->click.connect(this,&FrameMachine::onLeftBtn);
			prs->add(leftPrsBtn,nAlRight);
			prsPanel = new NTogglePanel();
			NFlowLayout fl(nAlLeft,5,5);
			fl.setLineBreak(false);
			prs->setPreferredSize( 200, 20 );
			prsPanel->setLayout(fl);
			prs->add(prsPanel, nAlClient);
			pane()->add(prs, nAlBottom);
		}


		inline int format(int c, int maxcols, int maxrows) {
			return (c / maxcols) + ( (c%maxcols)*maxrows);

		}

		void FrameMachine::initParameterGUI( )
		{
			int numParameters = pMachine_->GetNumParams();
			int cols = pMachine_->GetNumCols();
			int rows = numParameters/cols;
			// Various checks for "non-standard" windows ( lots of parameters, or "odd" parameter numbers)
			if (rows>24)	// check for "too big" windows
			{
				rows=24;
				cols=numParameters/24;
				if (cols*24 != numParameters)
				{
					cols++;
				}
			}
			if ( rows*cols < numParameters) rows++; // check if all the parameters are visible.

			knobPanel = new NPanel();
			NTableLayout tableLayout;
			tableLayout.setRows( rows );
			tableLayout.setColumns( cols );
			knobPanel->setLayout( tableLayout );
			pane()->add( knobPanel, nAlClient );

			NFont font("6x13",6,nMedium | nStraight | nAntiAlias);
			font.setTextColor( SkinReader::Instance()->framemachine_info().machineGUITopColor );
			knobPanel->setFont( font );

			int x = 0;
			int y = 0;

			for (int knobIdx =0; knobIdx < cols*rows; knobIdx++) {
				int min_v,max_v;

				if ( knobIdx < numParameters ) {
					pMachine_->GetParamRange( knobIdx,min_v,max_v);
					bool bDrawKnob = (min_v==max_v)?false:true;

					if (!bDrawKnob) {
						Header* cell = new Header();
						headerMap[ knobIdx ] = cell;
						char parName[64];
						pMachine_->GetParamName(knobIdx,parName);
						cell->setText(parName);
						knobPanel->add( cell, NAlignConstraint( nAlLeft, x, y ), true );
					} else if ( knobIdx < numParameters ) {
						Knob* knob = new Knob( knobIdx );
						char parName[64];
						pMachine_->GetParamName( knobIdx, parName );
						char buffer[128];
						pMachine_->GetParamValue( knobIdx, buffer );
						knob->setText(parName);
						knobMap[ knobIdx ] = knob;
						knob->valueChanged.connect(this,&FrameMachine::onKnobValueChange);												
						knobPanel->add( knob, NAlignConstraint( nAlLeft, x, y ), true );
					}					
				} else {
					// knob hole
					knobPanel->add( new Cell(), NAlignConstraint( nAlLeft, x, y ), true );
				}
				y++;
				if ( !(y % rows) ) {
					y = 0;
					x++;
				}
			}
			updateValues();
		}

		// Knob class



		void FrameMachine::onKnobValueChange( Knob* sender,int value , int param )
		{
			pMachine_->SetParameter(param, value);
			char buffer[128];
			int val_v = pMachine_->GetParamValue(param);
			sender->setValue(val_v);
			pMachine_->GetParamValue(param,buffer);
			std::string valuestring(buffer);
			sender->setValueAsText(valuestring);
			sender->repaint();
			///\todo work out tweak stuff
			//  if (Global::configuration()._RecordTweaks)
			//  {
			//    patternTweakSlide.emit(pMac()->_macIndex, param, value);
			//  }
		}


		void FrameMachine::onItemClicked( NEvent * menuEv, NButtonEvent * itemEv )
		{
			if (itemEv->text() == "Presets") {
				PresetsDlg* dlg = new PresetsDlg(this);
				add(dlg);
				dlg->execute();
				NApp::addRemovePipe(dlg);
			}
		}

		Machine * FrameMachine::pMac( )
		{
			return pMachine_;
		}

		void FrameMachine::updateValues( )
		{			
			std::map<int, Knob*>::iterator it = knobMap.begin();
			for ( ; it != knobMap.end(); it++ ) {	
				int knobIdx = it->first;
				int min_v,max_v;
				pMachine_->GetParamRange( knobIdx, min_v, max_v);
				Knob* cell = it->second;
				char buffer[128];
				pMachine_->GetParamValue( knobIdx, buffer );
				int val_v = pMachine_->GetParamValue( knobIdx );
				cell->setValue(val_v);
				cell->setRange(min_v,max_v);
				std::string valuestring(buffer);
				cell->setValueAsText(valuestring);				
			}

			knobPanel->repaint();
		}



		void FrameMachine::loadPresets() {
			std::string filename(pMac()->GetDllName());

			std::string::size_type pos = filename.find('.')  ;
			if ( pos == std::string::npos ) {
				filename  = filename + '.' + "prs";
			} else {
				filename = filename.substr(0,pos)+".prs";
			}

			try {
				RiffFile f;
				if ( !f.Open(Global::pConfig()->prsPath() +filename) ) throw "couldn`t open file";

				int numpresets;
				f.Read(numpresets);
				int filenumpars;
				f.Read(filenumpars);

				if (numpresets >= 0) {
					// old file format .. do not support so far ..
				} else {
					// new file format
					if (filenumpars == 1) {
						int filepresetsize;
						// new preset format version 1
						// new preset format version 1

						int numParameters = ((Plugin*) pMac())->GetInfo()->numParameters;
						int sizeDataStruct = ((Plugin *) pMac())->proxy().GetDataSize();

						int numpresets;
						f.Read(numpresets);
						f.Read(filenumpars);
						f.Read(filepresetsize);

						if (( filenumpars != numParameters )  || (filepresetsize != sizeDataStruct)) return;

						while (!f.Eof() ) {
							Preset newPreset(numParameters, sizeDataStruct);
							if (newPreset.loadFromFile(&f)) {
								NButton* prsBtn = new NButton(newPreset.name());
								prsBtn->setFlat(false);
								prsBtn->clicked.connect(this,&FrameMachine::onPrsClick);
								prsPanel->add(prsBtn);
								presetMap[prsBtn] = newPreset;
							}
						}
					}
				}
			} catch (const char * e) {
				// couldn`t open presets
			}
		}

		void FrameMachine::onLeftBtn( NButtonEvent * ev )
		{
			prsPanel->setScrollDx(std::max(0,prsPanel->scrollDx()-100));
			prsPanel->repaint();
		}

		void FrameMachine::onRightBtn( NButtonEvent * ev )
		{
			prsPanel->setScrollDx(std::min(prsPanel->preferredWidth() - prsPanel->clientWidth(),prsPanel->scrollDx()+100));
			prsPanel->repaint();
		}

		void FrameMachine::onPrsClick( NButtonEvent * ev )
		{
			std::map<NButton*,Preset>::iterator itr;
			if ( (itr = presetMap.find((NButton*)ev->sender())) != presetMap.end() ) {
				itr->second.tweakMachine(pMac() );
			}
			updateValues();
		}

		void FrameMachine::onAddPrs( NButtonEvent * ev )
		{
			NewNameDlg* dlg = new NewNameDlg();
			add(dlg);
			dlg->execute();
			NApp::addRemovePipe(dlg);
		}

		void FrameMachine::onLoadPrs( NButtonEvent * ev )
		{
			NFileDialog* dlg = new NFileDialog();
			add(dlg);
			dlg->setPosition(10,10,500,500);
			dlg->execute();
			NApp::addRemovePipe(dlg);
		}

		Preset FrameMachine::knobsPreset( )
		{
			Preset prs;

			return prs;
		}

		void FrameMachine::setVisible( bool on ) {
			if ( on ) {
				setPosition(20,20, pane()->preferredWidth(), knobPanel->preferredHeight() + prsPanel->preferredHeight() );
			}    
			NWindow::setVisible( on );
		}


	}
}
