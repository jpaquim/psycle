/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper  *
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
#include "global.h"
#include "plugin.h"
#include "binread.h"
#include <ngrs/app.h>
#include <ngrs/menubar.h>
#include <ngrs/togglepanel.h>
#include <ngrs/button.h>
#include <ngrs/edit.h>
#include <ngrs/filedialog.h>
#include <ngrs/slider.h>
#include <ngrs/alignlayout.h>
#include <ngrs/tablelayout.h>
#include <ngrs/alignconstraint.h>
#include <fstream>


#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace psycle {
	namespace host {


		NewNameDlg::NewNameDlg( )
		{
			ngrs::Panel* btnPnl = new ngrs::Panel();
			btnPnl->setLayout( ngrs::FlowLayout( ngrs::nAlRight, 5, 5) );
			btnPnl->add( new ngrs::Button("add") );
			btnPnl->add( new ngrs::Button("cancel") );
			pane()->add( btnPnl, ngrs::nAlBottom );

			ngrs::Edit* name = new ngrs::Edit();
			name->setText("userprs");
			pane()->add( name, ngrs::nAlClient );

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

		void Cell::paint( ngrs::Graphics& g )
		{
			int CW = clientWidth();
			int CH = clientHeight();

			g.setForeground( SkinReader::Instance()->framemachine_info().machineGUITopColor );
			g.fillRect(0,0,CW, CH / 2);
			g.setForeground( SkinReader::Instance()->framemachine_info().machineGUIBottomColor );
			g.fillRect(0,CH/2,CW, CH / 2);
		}

		int Cell::preferredWidth() const {
			return 100 + borderLeft() + borderRight();
		}

		int Cell::preferredHeight() const {
			return K_YSIZE + borderTop() + borderBottom() ;
		}
		// end of Cell class


		// cell subclass for a knob
		Knob::Knob(int param )  : max_range(100), min_range(0), value_(0), istweak(0), finetweak(1), ultrafinetweak(0), sourcepoint(0)  {
			setSpacing( ngrs::Size( 0,0,1,0 ));			
			param_ = param;  
			add( label = new ngrs::Label());
			add( vLabel = new ngrs::Label()); // the label that shows the value in %
			ngrs::Font font = ngrs::Font("6x13",6, ngrs::nMedium | ngrs::nStraight | ngrs::nAntiAlias );
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

		void Knob::paint( ngrs::Graphics& g )
		{
			Cell::paint(g);

			int amp = max_range - min_range;
			int rel = value_ - min_range;

			if (amp > 0) {
				int frame = (K_NUMFRAMES*rel)/amp;
				int xn = frame*K_XSIZE;
				int CH = clientHeight();

				g.putBitmap(0,(CH - K_YSIZE)/2,K_XSIZE,K_YSIZE, SkinReader::Instance()->bitmaps().tbmainknob(), xn,0);
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
			ngrs::FontMetrics metrics( font() );
			return std::max(2*metrics.textHeight(),K_YSIZE) + borderTop() + borderBottom() + spacing().top() + spacing().bottom();
		}

		int Knob::preferredWidth( ) const
		{
			ngrs::FontMetrics mtr( font() );
			return K_XSIZE + std::max(mtr.textWidth(vLabel->text()),mtr.textWidth(label->text())) + 10 + borderRight() + borderLeft() + spacing().left() + spacing().right();
		}		

		void Knob::setValueAsText( const std::string& text )
		{
			std::string bla = text;
			tvalue = bla;
			tvalue = text;
			vLabel->setText( tvalue );
		}

		void Knob::onMousePress( int x, int y, int button )
		{
			int CH = clientHeight();
			if ( ngrs::Rect(0,(CH - K_YSIZE)/2,K_XSIZE,K_YSIZE).intersects(x,y) ) {
				istweak = true;
				sourcepoint = y;
				tweakbase = value_;
			}
		}

		void Knob::onMouseOver( int x, int y )
		{
			if (istweak) {

				int shift = ngrs::App::system().shiftState();

				if (( ultrafinetweak && !( shift & ngrs::nsShift )) || //shift-key has been left.
					( !ultrafinetweak && ( shift & ngrs::nsShift ))) //shift-key has just been pressed
				{
					sourcepoint=y;
					ultrafinetweak=!ultrafinetweak;
					tweakbase = value_;
				}
				else if (( finetweak && !( shift & ngrs::nsCtrl )) || //control-key has been left.
					( !finetweak && ( shift &ngrs:: nsCtrl ))) //control-key has just been pressed
				{
					sourcepoint = y;
					finetweak=!finetweak;
					tweakbase=value_;
				}

				int maxval = max_range;
				int minval = min_range;

				int screenh = ngrs::App::system().screenHeight();
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
			ngrs::FrameBorder border;
			border.setSpacing( ngrs::Size(2,2,2,2) );
			border.setOval();
			setBorder( border );

			label = new ngrs::Label();
			label->setTransparent(false);
			label->setBackground( SkinReader::Instance()->framemachine_info().machineGUITitleColor );
			ngrs::Font font = ngrs::Font("6x13",6, ngrs::nMedium | ngrs::nStraight | ngrs::nAntiAlias );
			font.setTextColor( SkinReader::Instance()->framemachine_info().machineGUITitleFontColor );
			label->setFont( font );
			add(label);
		}

		void Header::setText( const std::string& text ) {
			label->setText( text );
		}

		void Header::resize() {
			int ch = clientHeight();
			int lh = label->preferredHeight();
			label->setPosition(0,(ch - lh) / 2,clientWidth(),lh);
		}
		// end of Header label class


		FrameMachine::FrameMachine( Machine* pMachine )
			: ngrs::Window()
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
			setVisible( false );
			return ngrs::nHideWindow;
		}

		void FrameMachine::init( )
		{
			setTitle( stringify(pMachine_->_macIndex) + std::string(" : ") + pMachine_->GetName() );

			ngrs::Panel* prs = new ngrs::Panel();
			prs->setLayout( ngrs::AlignLayout() );
			ngrs::Button* savePrsBtn = new ngrs::Button("save");
			savePrsBtn->setFlat(false);
			prs->add(savePrsBtn,ngrs::nAlLeft);
			ngrs::Button* loadPrsBtn = new ngrs::Button("load");
			loadPrsBtn->setFlat(false);
			loadPrsBtn->clicked.connect(this,&FrameMachine::onLoadPrs);
			prs->add(loadPrsBtn,ngrs::nAlLeft);
			ngrs::Button* addPrsBtn = new ngrs::Button("add");
			addPrsBtn->setFlat(false);
			addPrsBtn->clicked.connect( this,&FrameMachine::onAddPrs );
			prs->add( addPrsBtn,ngrs::nAlLeft );
			defaultPrsBtn = new ngrs::Button("User");
			defaultPrsBtn->setFlat(false);
			prs->add(defaultPrsBtn,ngrs::nAlLeft);
			ngrs::Button* rightPrsBtn = new ngrs::Button(">");
			rightPrsBtn->setFlat( false );
			rightPrsBtn->click.connect( this,&FrameMachine::onRightBtn );
			rightPrsBtn->setRepeatMode( true );
			prs->add( rightPrsBtn, ngrs::nAlRight );
			ngrs::Button* leftPrsBtn = new ngrs::Button("<");
			leftPrsBtn->setFlat( false );
			leftPrsBtn->setRepeatMode( true );
			leftPrsBtn->click.connect( this,&FrameMachine::onLeftBtn );
			prs->add( leftPrsBtn, ngrs::nAlRight );
			prsPanel = new ngrs::TogglePanel();
			ngrs::FlowLayout fl(ngrs::nAlLeft,5,5);
			fl.setLineBreak( false );
			prs->setPreferredSize( 200, 20 );
			prsPanel->setLayout( fl );
			prs->add( prsPanel, ngrs::nAlClient );
			pane()->add( prs, ngrs::nAlBottom );
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

			knobPanel = new ngrs::Panel();
			ngrs::TableLayout tableLayout;
			tableLayout.setRows( rows );
			tableLayout.setColumns( cols );
			knobPanel->setLayout( tableLayout );
			pane()->add( knobPanel, ngrs::nAlClient );

			ngrs::Font font("6x13",6, ngrs::nMedium | ngrs::nStraight | ngrs::nAntiAlias );
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
						knobPanel->add( cell, ngrs::AlignConstraint( ngrs::nAlLeft, x, y ), true );
					} else if ( knobIdx < numParameters ) {
						Knob* knob = new Knob( knobIdx );
						char parName[64];
						pMachine_->GetParamName( knobIdx, parName );
						char buffer[128];
						pMachine_->GetParamValue( knobIdx, buffer );
						knob->setText(parName);
						knobMap[ knobIdx ] = knob;
						knob->valueChanged.connect(this,&FrameMachine::onKnobValueChange);												
						knobPanel->add( knob, ngrs::AlignConstraint( ngrs::nAlLeft, x, y ), true );
					}					
				} else {
					// knob hole
					knobPanel->add( new Cell(), ngrs::AlignConstraint( ngrs::nAlLeft, x, y ), true );
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

		void FrameMachine::onKnobValueChange( Knob* sender, int value, int param )
		{
			pMachine_->SetParameter( param, value );
			char buffer[128];
			int val_v = pMachine_->GetParamValue( param );
			sender->setValue( val_v );
			pMachine_->GetParamValue( param, buffer );
			std::string valuestring( buffer);
			sender->setValueAsText( valuestring );
			sender->repaint();
			///\todo work out tweak stuff
			//  if (Global::configuration()._RecordTweaks)
			//  {
			//    patternTweakSlide.emit(pMac()->_macIndex, param, value);
			//  }
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
				cell->setValueAsText( buffer );
			}

			knobPanel->repaint();
		}

		bool FrameMachine::loadPresets() {
			std::string filename( pMac()->GetDllName() );

			std::string::size_type pos = filename.find('.')  ;
			if ( pos == std::string::npos ) {
				filename  = filename + '.' + "prs";
			} else {
				filename = filename.substr(0,pos)+".prs";
			}


			std::ifstream prsIn( std::string(Global::pConfig()->prsPath() + filename).c_str() );
			if ( !prsIn.is_open() )
				return false;      
			BinRead binIn( prsIn );

			int numpresets = binIn.readInt4LE();
			int filenumpars = binIn.readInt4LE();

			if (numpresets >= 0) {
				// old file format .. do not support so far ..
			} else {
				// new file format
				if ( filenumpars == 1 ) {
					int filepresetsize;
					// new preset format version 1
					// new preset format version 1

					int numParameters = ((Plugin*) pMac())->GetInfo()->numParameters;
					int sizeDataStruct = ((Plugin *) pMac())->proxy().GetDataSize();

					int numpresets = binIn.readInt4LE();
					filenumpars = binIn.readInt4LE();
					filepresetsize = binIn.readInt4LE();

					if (( filenumpars != numParameters )  || (filepresetsize != sizeDataStruct)) 
						return false;

					while ( !prsIn.eof() ) {
						Preset newPreset(numParameters, sizeDataStruct);
						if (newPreset.read( binIn )) {
							ngrs::Button* prsBtn = new ngrs::Button(newPreset.name());
							prsBtn->setFlat(false);
							prsBtn->clicked.connect(this,&FrameMachine::onPrsClick);
							prsPanel->add(prsBtn);
							presetMap[prsBtn] = newPreset;
						}
					}
				}
			}
			return true;
		}

		void FrameMachine::onLeftBtn( ngrs::ButtonEvent* ev )
		{
			prsPanel->setScrollDx( std::max(0,prsPanel->scrollDx()-100) );
			prsPanel->repaint();
		}

		void FrameMachine::onRightBtn( ngrs::ButtonEvent* ev )
		{
			prsPanel->setScrollDx(std::min(prsPanel->preferredWidth() - prsPanel->clientWidth(),prsPanel->scrollDx()+100));
			prsPanel->repaint();
		}

		void FrameMachine::onPrsClick( ngrs::ButtonEvent* ev )
		{
			std::map<ngrs::Button*,Preset>::iterator itr;
			if ( (itr = presetMap.find((ngrs::Button*)ev->sender())) != presetMap.end() ) {
				if ( pMac() ) itr->second.tweakMachine( *pMac() );
			}
			updateValues();
		}

		void FrameMachine::onAddPrs( ngrs::ButtonEvent* ev )
		{
			NewNameDlg* dlg = new NewNameDlg();
			add( dlg );
			dlg->execute();
			ngrs::App::addRemovePipe(dlg);
		}

		void FrameMachine::onLoadPrs( ngrs::ButtonEvent* ev )
		{
			ngrs::FileDialog* dlg = new ngrs::FileDialog();
			add(dlg);
			dlg->setPosition( 10, 10, 500, 500);
			dlg->execute();
			ngrs::App::addRemovePipe(dlg);
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
			ngrs::Window::setVisible( on );
		}


	}
}
