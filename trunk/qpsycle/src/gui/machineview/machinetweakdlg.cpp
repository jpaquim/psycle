/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*   psycle.sourceforge.net   *
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
#include "qpsyclePch.hpp"

#include <psycle/core/machine.h>
#include <psycle/core/plugin.h>
#include <psycle/core/binread.h>
#include <psycle/core/constants.h>

#include "machinetweakdlg.h"
#include "../global.h"
#include "../configuration.h"
#include "../inputhandler.h"
#include "machinegui.h"
#include "effectgui.h"


#include <QVBoxLayout>
#include <QLabel>
#include <QDial>
#include <QKeyEvent>
#include <QPainter>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QCompleter>
#include <QModelIndex>

#include <iostream>
#include <iomanip>

MachineTweakDlg::MachineTweakDlg( MachineGui *macGui, QWidget *parent ) 
	: QDialog( parent )
{
	setWindowIcon( QIcon(":/images/plugin.png") );
	pMachine_ = macGui->mac();
	m_macGui = macGui;


	knobPanel = new QWidget( this );
	mainLayout = new QVBoxLayout();
	knobPanelLayout = new QGridLayout();

	createActions();
	createMenus();
	
	
	mainLayout->setSpacing( 0 );
	mainLayout->setMargin( 0 );
	mainLayout->addWidget( menuBar );
	mainLayout->addWidget( knobPanel );
	setLayout( mainLayout );
	initParameterGui();
}

void MachineTweakDlg::createMenus()
{
	menuBar = new QMenuBar( this );
	aboutMenu = menuBar->addMenu( "About" );
	aboutMenu->addAction( aboutAction_ );
	
	paramsMenu = menuBar->addMenu( "Parameters" );
	paramsMenu->addAction( paramsResetAction_ );
	paramsMenu->addAction( paramsRandomAction_ );
	paramsMenu->addAction( paramsOpenPrsAction_ );
}

void MachineTweakDlg::createActions() 
{
	aboutAction_ = new QAction( "About this machine", this );
	
	paramsResetAction_ = new QAction( "R&eset parameters", this );
	paramsRandomAction_ = new QAction( "&Random parameters", this );
	paramsOpenPrsAction_ = new QAction( "Open &preset dialog", this );

	paramsOpenPrsAction_->setShortcut( tr("Ctrl+P") );
	paramsRandomAction_->setShortcut( tr("Ctrl+R") );
	paramsResetAction_->setShortcut( tr("Ctrl+E") );
	
	connect( aboutAction_, SIGNAL( triggered() ),
			this, SLOT( showAboutDialog() ) );
	connect( paramsResetAction_, SIGNAL( triggered() ),
			this, SLOT( resetParameters() ) );
	connect( paramsRandomAction_, SIGNAL( triggered() ),
			this, SLOT( randomiseParameters() ) );
	connect( paramsOpenPrsAction_, SIGNAL( triggered() ),
			this, SLOT( showPresetsDialog() ) );
	
}

/**
 *
 */
void MachineTweakDlg::initParameterGui()
{

	knobPanelLayout->setMargin( 0 );
	knobPanelLayout->setSpacing( 0 );
	knobPanel->setLayout( knobPanelLayout );

	int numParameters = pMachine_->GetNumParams();
	int cols = pMachine_->GetNumCols();
	int rows = numParameters/cols;
	// Various checks for "non-standard" windows ( lots of parameters, or "odd" parameter numbers)
	if (rows>24) // check for "too big" windows
	{
		rows=24;
		cols=numParameters/24;
		if (cols*24 != numParameters)
		{
			cols++;
		}
	}
	if ( rows*cols < numParameters) rows++; // check if all the parameters are visible.


	int x = 0;
	int y = 0;

	for ( int knobIdx =0; knobIdx < cols*rows; knobIdx++ ) {
		int min_v,max_v;

		if ( knobIdx < numParameters ) {
			pMachine_->GetParamRange( knobIdx,min_v,max_v);

			// FIXME: bit of a crude check to see if we have
			// a knob or a header/gap.
			bool bDrawKnob = (min_v==max_v)?false:true;

			if ( !bDrawKnob ) {
				char parName[64];
				pMachine_->GetParamName(knobIdx,parName);
				if(!std::strlen(parName) /* <bohan> don't know what pooplog's plugins use for separators... */ || std::strlen(parName) == 1) {
					knobPanelLayout->addWidget( new KnobHole(), y, x );
				} else {
					FHeader* cell = new FHeader();
					headerMap[ knobIdx ] = cell;
					cell->setText(parName);
					knobPanelLayout->addWidget( cell, y, x );
				}
			} else if ( knobIdx < numParameters ) {
				KnobGroup *knobGroup = new KnobGroup( knobIdx );
				char parName[64];
				pMachine_->GetParamName( knobIdx, parName );
				char buffer[128];
				pMachine_->GetParamValue( knobIdx, buffer );
				knobGroup->setNameText(parName);
				knobGroupMap[ knobIdx ] = knobGroup;
				connect( knobGroup, SIGNAL( changed( KnobGroup* ) ),
						this, SLOT( onKnobGroupChanged( KnobGroup* ) ) );
				knobPanelLayout->addWidget( knobGroup, y, x );
			}
		} else {
			// knob hole
			knobPanelLayout->addWidget( new KnobHole(), y, x );
		}
		y++;
		if ( !(y % rows) ) {
			y = 0;
			x++;
		}
	}
	knobPanel->repaint();
	updateValues();
}

void MachineTweakDlg::updateValues( )
{
	std::map<int, KnobGroup*>::iterator it = knobGroupMap.begin();
	for ( ; it != knobGroupMap.end(); it++ ) {
		int knobIdx = it->first;
		int min_v,max_v;
		pMachine_->GetParamRange( knobIdx, min_v, max_v);
		KnobGroup* kGroup = it->second;
		char buffer[128];
		pMachine_->GetParamValue( knobIdx, buffer );
		int val_v = pMachine_->GetParamValue( knobIdx );
		kGroup->knob()->setRange(min_v,max_v);
		kGroup->knob()->setValue(val_v);
		kGroup->setValueText( QString::fromStdString( buffer ) );
	}

	knobPanel->repaint();
}

void MachineTweakDlg::onKnobGroupChanged( KnobGroup *kGroup ) 
{
	int param = kGroup->knob()->param();
	int value = kGroup->knob()->value();
	pMachine_->SetParameter( param, value );
	char buffer[128];
	int val_v = pMachine_->GetParamValue( param );
	pMachine_->GetParamValue( param, buffer );
	kGroup->setValueText( QString::fromStdString( buffer ) );
}

void MachineTweakDlg::showEvent( QShowEvent *event )
{
	// FIXME: can adjustSize() be called somewhere else?
	adjustSize();
	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);
	buffer.str("");
	buffer << std::setfill('0') << std::hex << std::setw(2);
	buffer << pMachine_->id() << ": " << pMachine_->GetEditName();

	setWindowTitle( QString::fromStdString( buffer.str() ) );

	QWidget::showEvent( event );
}

void MachineTweakDlg::keyPressEvent( QKeyEvent *event )
{
	if ( event->key() == Qt::Key_W && event->modifiers() == Qt::ControlModifier ) {
		reject(); // closes the dialog
	} else {
		if ( !event->isAutoRepeat() ) {
			int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );
			int note = NULL;
			note = noteFromCommand( command );
			if (note) {
				emit notePress( note, pMachine_ );   
			}
		}
	}
}

void MachineTweakDlg::keyReleaseEvent( QKeyEvent *event )
{
	if ( !event->isAutoRepeat() ) {
		int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );
		int note = NULL;
		note = noteFromCommand( command );
		if (note) {
			emit noteRelease( note );   
		}
	}
}

void MachineTweakDlg::randomiseParameters() 
{
	int numpars = pMachine_->GetNumParams();
	for (int c=0; c<numpars; c++)
	{
		int minran,maxran;
		pMachine_->GetParamRange(c,minran,maxran);
		int range=(maxran-minran)+1; 
		float random = minran+(range*(rand()/(RAND_MAX + 1.0))); 

		pMachine_->SetParameter(c,random);
	}
	updateValues();
}

void MachineTweakDlg::resetParameters()
{
	if ( pMachine_->type() == psy::core::MACH_PLUGIN)
	{
		int numpars = pMachine_->GetNumParams();
		for (int c=0; c<numpars; c++)
		{
			int dv = ((psy::core::Plugin*)pMachine_)->GetInfo().Parameters[c]->DefValue;
			pMachine_->SetParameter(c,dv);
		}
	}
	updateValues();
	
}

void MachineTweakDlg::showPresetsDialog()
{
	prsDlg = new PresetsDialog( m_macGui, this );
	if ( prsDlg->exec() == QDialog::Accepted )
		updateValues();
}


void MachineTweakDlg::showAboutDialog()
{
	if ( pMachine_->type() == psy::core::MACH_PLUGIN )
	{
		QMessageBox::information( this,
						"About " + QString::fromStdString(((psy::core::Plugin*)pMachine_)->GetInfo().Name),
						"Authors: " + QString::fromStdString(((psy::core::Plugin*)pMachine_)->GetInfo().Author) );
	
	}
}


KnobGroup::KnobGroup( int param )
{
	QGridLayout *layout = new QGridLayout();
	layout->setMargin( 0 );
	layout->setSpacing( 0 );
	setLayout( layout );
	setFixedSize( LABEL_WIDTH, K_YSIZE ); // FIXME: unfix the size.

	knob_ = new Knob( param );

	nameLbl = new QLabel();
	QPalette plt = nameLbl->palette();
	plt.setBrush( QPalette::Window, QBrush( QColor( 194, 190, 210 ) ) );
	plt.setBrush( QPalette::WindowText, QBrush( Qt::black ) );
	nameLbl->setPalette( plt );
	nameLbl->setFont( QFont( "Verdana", 8 ) );
	nameLbl->setFixedSize( 150, K_XSIZE/2 );
	nameLbl->setIndent( 5 );
	nameLbl->setAutoFillBackground( true );

	valueLbl = new QLabel();
	QPalette plt1 = valueLbl->palette();
	plt1.setBrush( QPalette::Window, QBrush( QColor( 121, 109, 156 ) ) );
	plt1.setBrush( QPalette::WindowText, QBrush( Qt::white ) );
	valueLbl->setPalette( plt1 );
	valueLbl->setFont( QFont( "Verdana", 8 ) );
	valueLbl->setFixedSize( 150, K_YSIZE/2 );
	valueLbl->setIndent( 5 );
	valueLbl->setAutoFillBackground( true );

	connect( knob_, SIGNAL( valueChanged( int ) ),
			this, SLOT( onKnobChanged() ) );
	connect( knob_, SIGNAL( sliderPressed() ),
			this, SLOT( onKnobPressed() ) );
	connect( knob_, SIGNAL( sliderReleased() ),
			this, SLOT( onKnobReleased() ) );

	layout->addWidget( knob_, 0, 0, 2, 2, Qt::AlignLeft );
	layout->addWidget( nameLbl, 0, 2, 1, 7, Qt::AlignLeft );
	layout->addWidget( valueLbl, 1, 2, 1, 7, Qt::AlignLeft );
}

void KnobGroup::setKnob( Knob *inKnob )
{
	knob_ = inKnob;
}

Knob *KnobGroup::knob()
{
	return knob_;
}

void KnobGroup::setNameText( const QString & text )
{
	nameLbl->setText( text );
}

void KnobGroup::setValueText( const QString & text )
{
	valueLbl->setText( text );
}

void KnobGroup::onKnobChanged()
{
	emit changed( this );
}

void KnobGroup::onKnobPressed()
{
	QPalette plt = nameLbl->palette();
	plt.setBrush( QPalette::Window, QBrush( QColor( 169, 148, 172 ) ) );
	plt.setBrush( QPalette::WindowText, QBrush( Qt::black ) );
	nameLbl->setPalette( plt );

	QPalette plt1 = valueLbl->palette();
	plt1.setBrush( QPalette::Window, QBrush( QColor( 114, 90, 139 ) ) );
	plt1.setBrush( QPalette::WindowText, QBrush( QColor( 255, 238, 68 ) ) );
	valueLbl->setPalette( plt1 );
}

void KnobGroup::onKnobReleased()
{
	QPalette plt = nameLbl->palette();
	plt.setBrush( QPalette::Window, QBrush( QColor( 194, 190, 210 ) ) );
	plt.setBrush( QPalette::WindowText, QBrush( Qt::black ) );
	nameLbl->setPalette( plt );

	QPalette plt1 = valueLbl->palette();
	plt1.setBrush( QPalette::Window, QBrush( QColor( 121, 109, 156 ) ) );
	plt1.setBrush( QPalette::WindowText, QBrush( Qt::white ) );
	valueLbl->setPalette( plt1 );
}

QSize KnobGroup::sizeHint() const
{
	return QSize( LABEL_WIDTH, K_YSIZE );
}

/**
	* Knob class.
	* 
	*/
Knob::Knob( int param ) 
	: param_( param )
{
	setFixedSize( K_XSIZE, K_YSIZE ); // FIXME: unfix the size.
}

void Knob::paintEvent( QPaintEvent *ev )
{
	QPainter painter(this);
	QRectF target( 0, 0, K_XSIZE, K_YSIZE );
	QPixmap pixmap(":/images/TbMainKnob.png");

	int amp = maximum() - minimum();
	int rel = value() - minimum();

	if ( amp > 0 ) {
		int frame = ( K_NUMFRAMES * rel )/amp;
		int xn = frame * K_XSIZE;
		QRectF source( xn, 0, K_XSIZE, K_YSIZE );

		painter.drawPixmap( target, pixmap, source );
	}
}

QSize Knob::sizeHint() const
{
	return QSize( K_XSIZE, K_YSIZE );
}



/**
 * FHeader class.
 * 
 */
FHeader::FHeader( QWidget *parent )
	: QLabel( parent )
{
	setFont( QFont( "Verdana", 8 ) );
	setIndent( 10 );
}

void FHeader::paintEvent( QPaintEvent *ev )
{
	QPainter painter(this);

	painter.fillRect( 0, 0, width(), height()/4, QColor( 194, 190, 210 ) );
	painter.fillRect( 0, height()/4, width(), height()/2, Qt::black );
	QRectF textRect( indent(), height()/4, width(), height()/2 );
	painter.setPen( Qt::white );
	painter.drawText( textRect, Qt::AlignLeft | Qt::AlignVCenter, text() );
	painter.fillRect( 0, (height()*3)/4, width(), height()/4, QColor( 121, 109, 156 ) );
}

/**
 * KnobHole class.
 * 
 */
KnobHole::KnobHole( QWidget *parent )
	: QLabel( parent )
{}

void KnobHole::paintEvent( QPaintEvent *ev )
{
	QPainter painter(this);

	painter.fillRect( 0, 0, width(), height()/2, QColor( 194, 190, 210 ) );
	painter.fillRect( 0, height()/2, width(), height()/2, QColor( 121, 109, 156 ) );
}



PresetsDialog::PresetsDialog( MachineGui *macGui, QWidget *parent )
	: QDialog( parent )
{
	m_macGui = macGui;
	lay = new QGridLayout();
	label = new QLabel( "Machine Presets" );
	lineEdit = new QLineEdit( this );
	prsList = new QListWidget( this );
	saveBtn = new QPushButton( "Save", this );
	delBtn = new QPushButton( "Delete", this );
	impBtn = new QPushButton( "Import", this );
	expBtn = new QPushButton( "Export", this);
	
	prevChk = new QCheckBox( "Preview", this );
	

	useBtn = new QPushButton( "Use", this );
	clsBtn = new QPushButton( "Close", this );
	
	setLayout( lay );
	lay->addWidget( label, 0, 0, 1, 3 );
	lay->addWidget( lineEdit, 1, 0, 2, 2 );
	lay->addWidget( prsList, 3, 0, 10, 2 );
	lay->addWidget( saveBtn, 1, 2, 2, 3 );
	lay->addWidget( delBtn, 2, 2, 3, 3 );
	lay->addWidget( impBtn, 3, 2, 4, 3 );
	lay->addWidget( expBtn, 4, 2, 5, 3 );
	lay->addWidget( prevChk, 5, 2, 6, 3 );
	
	lay->addWidget( useBtn, 7, 2, 8, 3 );
	lay->addWidget( clsBtn, 9, 2, 10, 3 );


	connect( prsList, SIGNAL( itemDoubleClicked( QListWidgetItem* ) ),
			this, SLOT( usePreset() ) );
	connect( prsList, SIGNAL( itemClicked( QListWidgetItem* ) ),
			this, SLOT( onItemClicked( QListWidgetItem* ) ) );

	connect( useBtn, SIGNAL( pressed() ),
			this, SLOT( usePreset() ) );
			
	connect( clsBtn, SIGNAL( pressed() ),
			this, SLOT( reject() ) );

	loadPresets();

	QCompleter *completer = new QCompleter( prsList->model(), this );
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionMode( QCompleter::PopupCompletion );
	lineEdit->setCompleter(completer);
	connect( completer, SIGNAL( activated( const QString& ) ),
		this, SLOT( onCompletionActivated( const QString& ) )
	);
	//<nmather>says: it might be better to use QCompleter::activated( QModelIndex)
	//but I had some trouble getting it to work.
}

bool PresetsDialog::loadPresets()
{
	std::string filename( m_macGui->mac()->GetDllName() );

	std::string::size_type pos = filename.find('.')  ;
	if ( pos == std::string::npos ) {
		filename  = filename + '.' + "prs";
	} else {
		filename = filename.substr(0,pos)+".prs";
	}

	std::ifstream prsIn( std::string( Global::configuration().prsPath() + filename).c_str() );
	if ( !prsIn.is_open() )
		return false; 

	psy::core::BinRead binIn( prsIn );

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

			int numParameters = ((psy::core::Plugin*) m_macGui->mac())->GetInfo().numParameters;
			int sizeDataStruct = ((psy::core::Plugin *) m_macGui->mac())->proxy().GetDataSize();

			int numpresets = binIn.readInt4LE();
			filenumpars = binIn.readInt4LE();
			filepresetsize = binIn.readInt4LE();

			if (( filenumpars != numParameters )  || (filepresetsize != sizeDataStruct)) 
				return false;

			while ( !prsIn.eof() ) {
				qDebug("!prsIn.eof()");
				psy::core::Preset newPreset(numParameters, sizeDataStruct);
				if (newPreset.read( binIn )) {
					qDebug("doin item");
					QListWidgetItem *prsItm = new QListWidgetItem( QString::fromStdString( newPreset.name() ) );
					
					prsList->addItem( prsItm );
					presetMap[prsItm] = newPreset;
				}
			}
		}
	}
	return true;
}

void PresetsDialog::usePreset()
{
	if ( QListWidgetItem *selItem = prsList->currentItem() ) {
		std::map<QListWidgetItem*,psy::core::Preset>::iterator itr;
		if ( ( itr = presetMap.find( selItem ) ) != presetMap.end() ) {
			if ( m_macGui->mac() ) itr->second.tweakMachine( *m_macGui->mac() );
		}
		accept();
	}
}
	

void PresetsDialog::onCompletionActivated( const QString &text ) 
{
	QList<QListWidgetItem*> items = prsList->findItems( text, Qt::MatchExactly );
	QListWidgetItem *item = items.takeFirst();
	prsList->setCurrentItem( item );
}

void PresetsDialog::onItemClicked( QListWidgetItem *item )
{
	lineEdit->setText( item->text() );
}


void PresetsDialog::onSavePreset() 
{
	#if 0
	char str[32];
	str = lineEdit->text().toStdString().c_str();
	if ( str[0] == '\0' )
	{
		qDebug("no name");
		//MessageBox("You have not specified any name. Operation Aborted.","Preset Save Error",MB_OK);
		return;
	}
	//iniPreset.SetName(str);
	//AddPreset(iniPreset);
	SavePresets();
	#endif
}

// FIXME: should be somewhere else, perhaps global.
int MachineTweakDlg::noteFromCommand( int command )
{
	int note = NULL;
	switch ( command ) {
		case commands::key_C_0:
		note = 1;
		break;
		case commands::key_CS0:
		note = 2;
		break;
		case commands::key_D_0:
		note = 3;
		break;
		case commands::key_DS0:
		note = 4;
		break;
		case commands::key_E_0:
		note = 5;
		break;
		case commands::key_F_0:
		note = 6;
		break;
		case commands::key_FS0:
		note = 7;
		break;
		case commands::key_G_0:
		note = 8;
		break;
		case commands::key_GS0:
		note = 9;
		break;
		case commands::key_A_0:
		note = 10;
		break;
		case commands::key_AS0:
		note = 11;
		break;
		case commands::key_B_0: 
		note = 12;
		break;
		case commands::key_C_1:
		note = 13;
		break;
		case commands::key_CS1:
		note = 14;
		break;
		case commands::key_D_1:
		note = 15;
		break;
		case commands::key_DS1:
		note = 16;
		break;
		case commands::key_E_1:
		note = 17;
		break;
		case commands::key_F_1:
		note = 18;
		break;
		case commands::key_FS1:
		note = 19;
		break;
		case commands::key_G_1:
		note = 20;
		break;
		case commands::key_GS1:
		note = 21;
		break;
		case commands::key_A_1:
		note = 22;
		break;
		case commands::key_AS1:
		note = 23;
		break;
		case commands::key_B_1: 
		note = 24;
		break;
		case commands::key_C_2:
		note = 25;
		break;
		case commands::key_CS2:
		note = 26;
		break;
		case commands::key_D_2:
		note = 27;
		break;
		case commands::key_DS2:
		note = 28;
		break;
		case commands::key_E_2:
		note = 29;
		break;
	}
	return note;
}
