/***************************************************************************
  *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "wavesavedlg.h"
#include "global.h"
#include "player.h"
#include "configuration.h"
#include "machine.h"
#include <iomanip>
#include <ngrs/napp.h>
#include <ngrs/nlabel.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/ntablelayout.h>
#include <ngrs/nflowlayout.h>
#include <ngrs/nalignconstraint.h>
#include <ngrs/nbutton.h>
#include <ngrs/nedit.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/nradiobutton.h>
#include <ngrs/ntogglepanel.h>
#include <ngrs/ncombobox.h>
#include <ngrs/nitem.h>
#include <ngrs/nprogressbar.h>
#include <ngrs/nfiledialog.h>
#include <ngrs/nbevelborder.h>

WaveSaveDlg::WaveSaveDlg()
  : NDialog()
{
  autostop = playblock = loopsong = saving = 0;
  rate = -1;
  bits = -1;
  m_recmode = 0;
  channelmode = -1;

  // init gui

  setTitle("Render as Wav File");
  setSize(400,600);

  fileDlg = new NFileDialog();
  add(fileDlg);

  pane()->setLayout(NAlignLayout(5,5));

  NPanel* pathPanel = new NPanel();
    pathPanel->setLayout(NAlignLayout(5,10));
    pathPanel->add(new NLabel("Output Path"), nAlLeft);
    NButton* browseBtn = new NButton("Browse");
      browseBtn->setFlat(false);
      browseBtn->clicked.connect(this,&WaveSaveDlg::onBrowseBtn);
    pathPanel->add(browseBtn,nAlRight);
    pathEdt = new NEdit();
  pathPanel->add(pathEdt,nAlClient);

  pane()->add( pathPanel, nAlTop );

  wireChkBox = new NCheckBox();
    wireChkBox->setText("Save each unmuted input to master as a separated wav (wire number will be appended to filename)");
    wireChkBox->setWordWrap(true);
  pane()->add(wireChkBox, nAlTop);

  trackChkBox = new NCheckBox();
    trackChkBox->setText("Save each unmuted track as a separated wav (track number will be appended to filename)** may suffer from 'delay bleed' - insert silence at the end of your file if this is a problem");
    trackChkBox->setWordWrap(true);
  pane()->add(trackChkBox, nAlTop);

  generatorChkBox = new NCheckBox();
    generatorChkBox->setText("Save each unmuted generator as a separated wav (generator number will be appended to filename)** may suffer from 'delay bleed' - insert silence at the end of your file if this is a problem");
    generatorChkBox->setWordWrap(true);
  pane()->add(generatorChkBox, nAlTop);

  NTogglePanel* gBox = new NTogglePanel();
    gBox->setBorder(NBevelBorder(nNone,nLowered));
    NTableLayout tableLayout(4,3);
      tableLayout.setVGap(5);
      tableLayout.setHGap(5);
    gBox->setLayout(tableLayout);
    entireRBtn = new NRadioButton();
      entireRBtn->setText("Record the entire song");
    gBox->add(entireRBtn,NAlignConstraint(nAlLeft,0,0));
    numberRBtn = new NRadioButton();
      numberRBtn->setText("Record pattern number");
    gBox->add(numberRBtn,NAlignConstraint(nAlLeft,0,1));
    numberEdt = new NEdit();
    gBox->add(numberEdt,NAlignConstraint(nAlLeft,1,1));
    gBox->add(new NLabel("in HEX value"),NAlignConstraint(nAlLeft,2,1));
    seqRBtn = new NRadioButton();
      seqRBtn->setText("Sequence positions from");
    gBox->add(seqRBtn,NAlignConstraint(nAlLeft,0,2));
    fromEdt = new NEdit();
    gBox->add(fromEdt,NAlignConstraint(nAlLeft,1,2));
    gBox->add(new NLabel("to"),NAlignConstraint(nAlLeft,2,2));
    toEdt = new NEdit();
    gBox->add(toEdt,NAlignConstraint(nAlLeft,3,2));
    gBox->add(new NLabel("in HEX value"),NAlignConstraint(nAlLeft,4,2));
  pane()->add(gBox, nAlTop);
  NPanel * audioPanel = new NPanel();
    audioPanel->setLayout(NAlignLayout());
  audioPanel->add(new NLabel("Note many filters\nscrew up when rendering\nat slow sample rates"),nAlLeft);
    NPanel* cboxPanel = new NPanel();
        cboxPanel->setLayout(NAlignLayout());
        cboxPanel->add(new NLabel("Sampling rate"),nAlTop);
        sampleRateCbx = new NComboBox();
          sampleRateCbx->add(new NItem("8192 hz"));
          sampleRateCbx->add(new NItem("11025 hz"));
          sampleRateCbx->add(new NItem("22050 hz"));
          sampleRateCbx->add(new NItem("44100 hz"));
          sampleRateCbx->add(new NItem("48000 hz"));
          sampleRateCbx->add(new NItem("96000 hz"));
          sampleRateCbx->setIndex(3);
        cboxPanel->add(sampleRateCbx,nAlTop);
        cboxPanel->add(new NLabel("Bit rate"),nAlTop);
        bitDepthCbx = new NComboBox();
          bitDepthCbx->add(new NItem("8"));
          bitDepthCbx->add(new NItem("16"));
          bitDepthCbx->add(new NItem("24"));
          bitDepthCbx->add(new NItem("32"));
          bitDepthCbx->setIndex(1);
        cboxPanel->add(bitDepthCbx,nAlTop);
        cboxPanel->add(new NLabel("Channels"),nAlTop); 
        channelsCbx = new NComboBox();
          channelsCbx->add(new NItem("Mono [mix]"));
          channelsCbx->add(new NItem("Mono [left]"));
          channelsCbx->add(new NItem("Mono [right]"));
          channelsCbx->add(new NItem("Stereo"));
          channelsCbx->setIndex(3);
        cboxPanel->add(channelsCbx,nAlTop);
    audioPanel->add(cboxPanel, nAlClient);
  pane()->add(audioPanel,nAlTop);
  progressBar = new NProgressBar();
      progressBar->setValue(0);
  pane()->add(progressBar,nAlTop);
  NPanel* btnPanel = new NPanel();
    btnPanel->setLayout(NFlowLayout(nAlRight,5,5));
    closeBtn = new NButton("Close");
      closeBtn->setFlat(false);
      closeBtn->clicked.connect(this,&WaveSaveDlg::onCloseBtn);
    btnPanel->add(closeBtn);
    NButton* saveBtn  = new NButton("Save as Wav");
      saveBtn->setFlat(false);
      saveBtn->clicked.connect(this,&WaveSaveDlg::onSaveBtn);
    btnPanel->add(saveBtn);
  pane()->add(btnPanel,nAlTop);

  initVars();
}


WaveSaveDlg::~WaveSaveDlg()
{
}

void WaveSaveDlg::initVars( )
{
  // init variables

  threadopen = 0;
  kill_thread = 1;
  lastpostick = 0;
  lastlinetick = 0;

  std::string name;// = Global::pConfig->GetCurrentSongDir();
  //name+='\\';
  Song* pSong = Global::pSong();

  name+= Global::pSong()->fileName;
  name = name.substr(0,std::max(std::string::size_type(0),name.length()-4));
  name+=".wav";
  pathEdt->setText(name);
  setTitle(name);

  char num[3];
  sprintf(num,"%02x",pSong->playOrder[0]);
  numberEdt->setText(num);
  sprintf(num,"%02x",0);
  fromEdt->setText(num);
  sprintf(num,"%02x",pSong->playLength-1);
  toEdt->setText(num);

  if ( (rate < 0) || (rate >5) )
  {
      if (Global::pConfig()->_pOutputDriver->_samplesPerSec <= 8192)
      {
        rate = 0;
      }
      else if (Global::pConfig()->_pOutputDriver->_samplesPerSec <= 11025)
      {
        rate = 1;
      } else if (Global::pConfig()->_pOutputDriver->_samplesPerSec <= 22050)
      {
        rate = 2;
      }
      else if (Global::pConfig()->_pOutputDriver->_samplesPerSec <= 44100)
      {
        rate = 3;
      }
      else if (Global::pConfig()->_pOutputDriver->_samplesPerSec <= 48000)
      {
        rate = 4;
      }
      else
      {
        rate = 5;
      }
  }

  if ((channelmode < 0) || (channelmode > 3))
  {
    channelmode = Global::pConfig()->_pOutputDriver->_channelmode;
  }

  if ((bits < 0) || (bits > 3))
  {
    if (Global::pConfig()->_pOutputDriver->_bitDepth <= 8)
    {
      bits = 0;
    }
    else if (Global::pConfig()->_pOutputDriver->_bitDepth <= 16)
    {
      bits = 1;
    }
    else if (Global::pConfig()->_pOutputDriver->_bitDepth <= 24)
    {
      bits = 2;
    }
    else if (Global::pConfig()->_pOutputDriver->_bitDepth <= 32)
    {
      bits = 3;
    }
  }

  std::cout << bits << std::endl;
}

void WaveSaveDlg::onBrowseBtn( NButtonEvent * ev )
{
  if (fileDlg->execute()) {

  }
}

void WaveSaveDlg::onCloseBtn( NButtonEvent * ev )
{
}

void WaveSaveDlg::onSaveBtn( NButtonEvent * ev )
{
  Song *pSong     = Global::pSong();
  Player *pPlayer = Global::pPlayer();

  closeBtn->setText("Stop");
  pane()->resize();
  pane()->repaint();

  autostop = Global::pConfig()->autoStopMachines;

  if ( Global::pConfig()->autoStopMachines )
  {
    Global::pConfig()->autoStopMachines = false;
    for (int c=0; c<MAX_MACHINES; c++)
    {
      if (pSong->_pMachine[c])
      {
        pSong->_pMachine[c]->_stopped=false;
      }
    }
  }

  playblock = pPlayer->_playBlock;
  loopsong = pPlayer->_loopSong;
  memcpy(sel,pSong->playOrderSel,MAX_SONG_POSITIONS);
  memset(pSong->playOrderSel,0,MAX_SONG_POSITIONS);

  std::string name = pathEdt->text();

  rootname = name;
  rootname=rootname.substr(0,std::max(std::string::size_type(0),rootname.length()-4));

  const int real_rate[]={8192,11025,22050,44100,48000,96000};
  const int real_bits[]={8,16,24,32};

  if (trackChkBox->checked())
  {
    memcpy(_Muted,pSong->_trackMuted,sizeof(pSong->_trackMuted));

    int count = 0;

    for (int i = 0; i < pSong->SONGTRACKS; i++)
    {
      if (!_Muted[i])
      {
        count++;
        current = i;
        for (int j = 0; j < pSong->SONGTRACKS; j++)
        {
          if (j != i) {
            pSong->_trackMuted[j] = true;
          } else {
                pSong->_trackMuted[j] = false;
          }
        }
        // now save the song
        std::ostringstream filename;
        filename << rootname;

        filename << "-track " << std::setprecision(2) << (unsigned)i;
        saveWav(filename.str().c_str(),real_bits[bits],real_rate[rate],channelmode);
        return;
      }
      }
      current = 256;
      saveEnd();
  } else
  if (wireChkBox->checked())
  {
    // this is tricky - sort of
    // back up our connections first
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        if (pSong->_pMachine[MASTER_INDEX]->_inputCon[i])
        {
          _Muted[i] = pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_mute;
        } else {
          _Muted[i] = true;
        }
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
      if (!_Muted[i])
      {
        current = i;
        for (int j = 0; j < MAX_CONNECTIONS; j++)
        {
          if (pSong->_pMachine[MASTER_INDEX]->_inputCon[j])
          {
            if (j != i)
            {
              pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = true;
            } else
            {
              pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = false;
            }
          }
        }
        // now save the song
        char filename[8000];
        sprintf(filename,"%s-wire %.2u %s.wav",rootname.c_str(),i,pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_editName);
        saveWav(filename,real_bits[bits],real_rate[rate],channelmode);
        return;
      }
    }
    current = 256;
    saveEnd();
  } else 
  if ( generatorChkBox->checked() )
  {
    // this is tricky - sort of
    // back up our connections first

    for (int i = 0; i < MAX_BUSES; i++)
    {
      if (pSong->_pMachine[i])
      {
        _Muted[i] = pSong->_pMachine[i]->_mute;
      }
      else
      {
        _Muted[i] = true;
      }
    }

    for (int i = 0; i < MAX_BUSES; i++)
    {
      if (!_Muted[i])
      {
        current = i;
        for (int j = 0; j < MAX_BUSES; j++)
        {
          if (pSong->_pMachine[j])
          {
            if (j != i)
            {
              pSong->_pMachine[j]->_mute = true;
            }
            else
            {
              pSong->_pMachine[j]->_mute = false;
            }
          }
        }
        // now save the song
        char filename[8000];
        sprintf(filename,"%s-generator %.2u %s.wav",rootname.c_str(),i,pSong->_pMachine[i]->_editName);
        saveWav(filename,real_bits[bits],real_rate[rate],channelmode);
        return;
      }
    }
    current = 256;
    saveEnd();
  } else {
      saveWav(name,real_bits[bits],real_rate[rate],channelmode);
  }
}

void WaveSaveDlg::saveWav( std::string file, int bits, int rate, int channelmode )
{
  std::cout << bits << std::endl;
  std::cout << rate << std::endl;
  std::cout << channelmode << std::endl;

  saving=true;
  Player *pPlayer = Global::pPlayer();
  Song *pSong = Global::pSong();
  pPlayer->StopRecording();
  Global::pConfig()->_pOutputDriver->Enable(false);
//  Global::pConfig()->_pMidiInput->Close();

  std::string::size_type pos = file.rfind('\\');
  if (pos == std::string::npos)
  {
    //m_text.SetWindowText(file.c_str());
  }
  else
  {
    //m_text.SetWindowText(file.substr(pos+1).c_str());
  }

  pPlayer->StartRecording(file,bits,rate,channelmode);

  int tmp;
  int cont;

  std::string name;

  int pstart;
  kill_thread = 0;
  int tickcont=0;
  int lastlinetick=0;
  int i,j;

  int m_recmode = 0;

  switch (m_recmode)
  {
    case 0:
      j=0; // Calculate progress bar range.
      for (i=0;i<pSong->playLength;i++)
      {
        j+=pSong->patternLines[pSong->playOrder[i]];
      }
      //m_progress.SetRange(0,j);

      pPlayer->_playBlock=false;
      lastpostick=0;
      pPlayer->Start(0,0);
    break;
    case 1:
//        m_patnumber.GetWindowText(name);
//        hexstring_to_integer(name.GetBuffer(2), pstart);
        progressBar->setRange(0,pSong->patternLines[pstart]);
        for (cont=0;cont<pSong->playLength;cont++)
        {
          if ( (int)pSong->playOrder[cont] == pstart)
          {
            pstart= cont;
            break;
          }
        }
        lastpostick=pstart;
        pSong->playOrderSel[cont]=true;
        pPlayer->Start(pstart,0);
        pPlayer->_playBlock=true;
        pPlayer->_loopSong=false;
    break;
    case 2:
//        m_rangestart.GetWindowText(name);
//        hexstring_to_integer(name.GetBuffer(2), pstart);
//        m_rangeend.GetWindowText(name);
//        hexstring_to_integer(name.GetBuffer(2), tmp);
        j=0;
        for (cont=pstart;cont<=tmp;cont++)
        {
          pSong->playOrderSel[cont]=true;
          j+=pSong->patternLines[pSong->playOrder[cont]];
        }
        progressBar->setRange(0,j);

        lastpostick=pstart;
        pPlayer->Start(pstart,0);
        pPlayer->_playBlock=true;
        pPlayer->_loopSong=false;
    break;
    default:
      saveEnd();
      return;
  }
  pthread_create(&threadid, NULL, (void*(*)(void*))audioOutThread, (void*) this);
}

void WaveSaveDlg::saveEnd( )
{
  saving=false;
  kill_thread=1;
  if ( autostop )
  {
    Global::pConfig()->autoStopMachines=true;
  }
  Global::pPlayer()->_playBlock = playblock;
  Global::pPlayer()->_loopSong  = loopsong;
  memcpy(Global::pSong()->playOrderSel,sel,MAX_SONG_POSITIONS);
  Global::pConfig()->_pOutputDriver->Enable(true);
  //Global::pConfig->_pMidiInput->Open();

  if ( trackChkBox->checked() )
  {
    Song *pSong = Global::pSong();

    const int real_rate[]={8192,11025,22050,44100,48000,96000};
    const int real_bits[]={8,16,24,32};

    for (int i = current+1; i < pSong->SONGTRACKS; i++)
    {
      if (!_Muted[i])
      {
        current = i;
        for (int j = 0; j < pSong->SONGTRACKS; j++)
        {
          if (j != i)
          {
            pSong->_trackMuted[j] = true;
          } else {
            pSong->_trackMuted[j] = false;
          }
        }
        // now save the song
        char filename[8000];
        sprintf(filename,"%s-track %.2u.wav",rootname,i);
        saveWav(filename,real_bits[bits],real_rate[rate],channelmode);
        return;
      }
    }
    memcpy(pSong->_trackMuted,_Muted,sizeof(pSong->_trackMuted));
  }
  else if ( wireChkBox->checked() ) {
    Song *pSong = Global::pSong();

    const int real_rate[]={8192,11025,22050,44100,48000,96000};
    const int real_bits[]={8,16,24,32};

    for (int i = current+1; i < MAX_CONNECTIONS; i++)
    {
        if (!_Muted[i])
        {
          current = i;
          for (int j = 0; j < MAX_CONNECTIONS; j++)
          {
              if (pSong->_pMachine[MASTER_INDEX]->_inputCon[j]) {
                if (j != i)
                {
                  pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = true;
                } else {
                  pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[j]]->_mute = false;
                }
              }
            }
            // now save the song
            char filename[8000];
            sprintf(filename,"%s-wire %.2u %s.wav",rootname.c_str(),i,pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_editName);
            saveWav(filename,real_bits[bits],real_rate[rate],channelmode);
            return;
        }
      }

      for (int i = 0; i < MAX_CONNECTIONS; i++)
      {
        if (pSong->_pMachine[MASTER_INDEX]->_inputCon[i])
        {
          pSong->_pMachine[pSong->_pMachine[MASTER_INDEX]->_inputMachines[i]]->_mute = _Muted[i];
        }
      }
    } else if ( generatorChkBox->checked() ) {
        Song *pSong = Global::pSong();

        const int real_rate[]={8192,11025,22050,44100,48000,96000};
        const int real_bits[]={8,16,24,32};

        for (int i = current+1; i < MAX_BUSES; i++)
        {
          if (!_Muted[i])
          {
            current = i;
            for (int j = 0; j < MAX_BUSES; j++)
            {
              if (pSong->_pMachine[j])
              {
                if (j != i)
                {
                  pSong->_pMachine[j]->_mute = true;
                } else
                {
                  pSong->_pMachine[j]->_mute = false;
                }
              }
            }
        // now save the song
        char filename[8000];
        sprintf(filename,"%s-generator %.2u %s.wav",rootname.c_str(),i,pSong->_pMachine[i]->_editName);
        saveWav(filename,real_bits[bits],real_rate[rate],channelmode);
        return;
      }
    }

    for (int i = 0; i < MAX_BUSES; i++)
    {
      if (pSong->_pMachine[i]) {
        pSong->_pMachine[i]->_mute = _Muted[i];
      }
    }
  }


  closeBtn->setText("Close");
  pane()->resize();
  pane()->repaint();
}

int WaveSaveDlg::audioOutThread( void * ptr )
{
  WaveSaveDlg* pWaveSaveDlg = (WaveSaveDlg*) ptr;
  pWaveSaveDlg->threadopen++;
  Player* pPlayer = Global::pPlayer();
  int stream_size = 8192; // Player has just a single buffer of 65535 samples to allocate both channels
  //int stream_buffer[65535];
  while(!(pWaveSaveDlg->kill_thread))
  {
    if (!pPlayer->_recording) // the player automatically closes the wav recording when looping.
    {
      pPlayer->Stop();
      pWaveSaveDlg->saveEnd();
      pWaveSaveDlg->threadopen--;
      pthread_exit(0);
    }
    pPlayer->Work(pPlayer,stream_size);
    pWaveSaveDlg->saveTick();
  }

  pPlayer->Stop();
  pPlayer->StopRecording();
  pWaveSaveDlg->saveEnd();
  pWaveSaveDlg->threadopen--;
  pthread_exit(0);
}

void WaveSaveDlg::saveTick( )
{
  Song* pSong = Global::pSong();
  Player* pPlayer = Global::pPlayer();

  for (int i=lastpostick+1;i<pPlayer->_playPosition;i++)
  {
    tickcont+=pSong->patternLines[pSong->playOrder[i]];
  }
  if (lastpostick!= pPlayer->_playPosition ) 
  {
    tickcont+=pSong->patternLines[pSong->playOrder[lastpostick]]-(lastlinetick+1)+pPlayer->_lineCounter;
  }
  else tickcont+=pPlayer->_lineCounter-lastlinetick;

  lastlinetick = pPlayer->_lineCounter;
  lastpostick = pPlayer->_playPosition;

  if (!kill_thread )
  {
//     progressBar->setPos(tickcont);
  }
}

void WaveSaveDlg::setVisible( bool on )
{
  if (on) {
      initVars();
  }
  NDialog::setVisible(on);
}




