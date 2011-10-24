/***************************************************/
/*! \class Wurley
    \brief STK Wurlitzer electric piano FM
           synthesis instrument.

    This class implements two simple FM Pairs
    summed together, also referred to as algorithm
    5 of the TX81Z.

    \code
    Algorithm 5 is :  4->3--\
                             + --> Out
                      2->1--/
    \endcode

    Control Change Numbers: 
       - Modulator Index One = 2
       - Crossfade of Outputs = 4
       - LFO Speed = 11
       - LFO Depth = 1
       - ADSR 2 & 4 Target = 128

    The basic Chowning/Stanford FM patent expired
    in 1995, but there exist follow-on patents,
    mostly assigned to Yamaha.  If you are of the
    type who should worry about this (making
    money) worry away.

    by Perry R. Cook and Gary P. Scavone, 1995 - 2004.
*/
/***************************************************/

#include "Wurley.h"

Wurley :: Wurley()
  : FM()
{
  // Concatenate the STK rawwave path to the rawwave files
  for ( unsigned int i=0; i<3; i++ )
    waves_[i] = new WaveLoop( (Stk::rawwavePath() + "sinewave.raw").c_str(), true );
  waves_[3] = new WaveLoop( (Stk::rawwavePath() + "fwavblnk.raw").c_str(), true );

  this->setRatio(0, 1.0);
  this->setRatio(1, 4.0);
  this->setRatio(2, -510.0);
  this->setRatio(3, -510.0);

  gains_[0] = fmGains_[99];
  gains_[1] = fmGains_[82];
  gains_[2] = fmGains_[92];
  gains_[3] = fmGains_[68];

  adsr_[0]->setAllTimes( 0.001, 1.50, 0.0, 0.04);
  adsr_[1]->setAllTimes( 0.001, 1.50, 0.0, 0.04);
  adsr_[2]->setAllTimes( 0.001, 0.25, 0.0, 0.04);
  adsr_[3]->setAllTimes( 0.001, 0.15, 0.0, 0.04);

  twozero_.setGain( 2.0 );
  vibrato_->setFrequency( 8.0 );
}  

Wurley :: ~Wurley()
{
}

void Wurley :: setFrequency(StkFloat frequency)
{    
  baseFrequency_ = frequency;
  waves_[0]->setFrequency( baseFrequency_ * ratios_[0]);
  waves_[1]->setFrequency( baseFrequency_ * ratios_[1]);
  waves_[2]->setFrequency( ratios_[2] );	// Note here a 'fixed resonance'.
  waves_[3]->setFrequency( ratios_[3] );
}

void Wurley :: noteOn(StkFloat frequency, StkFloat amplitude)
{
  gains_[0] = amplitude * fmGains_[99];
  gains_[1] = amplitude * fmGains_[82];
  gains_[2] = amplitude * fmGains_[82];
  gains_[3] = amplitude * fmGains_[68];
  this->setFrequency( frequency );
  this->keyOn();

#if defined(_STK_DEBUG_)
  errorString_ << "Wurley::NoteOn: frequency = " << frequency << ", amplitude = " << amplitude << '.';
  handleError( StkError::DEBUG_WARNING );
#endif
}

StkFloat Wurley :: tick()
{
  StkFloat temp, temp2;

  temp = gains_[1] * adsr_[1]->tick() * waves_[1]->tick();
  temp = temp * control1_;

  waves_[0]->addPhaseOffset( temp );
  waves_[3]->addPhaseOffset( twozero_.lastOut() );
  temp = gains_[3] * adsr_[3]->tick() * waves_[3]->tick();
  twozero_.tick(temp);

  waves_[2]->addPhaseOffset( temp );
  temp = ( 1.0 - (control2_ * 0.5)) * gains_[0] * adsr_[0]->tick() * waves_[0]->tick();
  temp += control2_ * 0.5 * gains_[2] * adsr_[2]->tick() * waves_[2]->tick();

  // Calculate amplitude modulation and apply it to output.
  temp2 = vibrato_->tick() * modDepth_;
  temp = temp * (1.0 + temp2);
    
  lastOutput_ = temp * 0.5;
  return lastOutput_;
}

StkFloat *Wurley :: tick(StkFloat *vector, unsigned int vectorSize)
{
  return Instrmnt::tick( vector, vectorSize );
}

StkFrames& Wurley :: tick( StkFrames& frames, unsigned int channel )
{
  return Instrmnt::tick( frames, channel );
}
