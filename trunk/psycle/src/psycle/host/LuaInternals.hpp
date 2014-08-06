#pragma once
#include <psycle/host/detail/project.hpp>
#include "plugininfo.hpp"
#include "LuaArray.hpp"
#include "XMSampler.hpp"
#include <psycle/helpers/resampler.hpp>

struct lua_State;
struct luaL_Reg;

namespace psycle { namespace host {

class Machine;

class LuaMachine {
public:
	LuaMachine() : mac_(0), shared_(false), num_parameter_(0), num_cols_(0) {}
	~LuaMachine();
	void load(const char* name);
	void work(int samples);	
	Machine* mac() { return mac_; }
	void set_mac(Machine* mac) { mac_ = mac; shared_=true; }
	PSArray* channel(int idx) { return &sampleV_[idx]; }
	void update_num_samples(int num);
	void offset(int offset);
	void build_buffer(std::vector<float*>& buf, int num);
	void set_buffer(std::vector<float*>& buf);
	psybuffer& samples() { return sampleV_; }
	void set_numparams(int num) { num_parameter_ = num;  }
	int numparams() const { return num_parameter_; }
	void set_numcols(int num) { num_cols_ = num; }
	int numcols() const { return num_cols_; }	
private:
	Machine* mac_;
	psybuffer sampleV_;
	bool shared_;
	int num_parameter_;
	int num_cols_;
};

struct LuaMachineBind {  
	static int open(lua_State *L);
  private:
	static int create(lua_State* L);
	static int work(lua_State* L);
	static int tick(lua_State* L);
	static int channel(lua_State* L);
	static int resize(lua_State* L);
	static int gc(lua_State* L);	
	static int setbuffer(lua_State* L);
	static int setnorm(lua_State* L);
    static int getnorm(lua_State* L);
	static int name(lua_State* L);
	static int label(lua_State* L);
	static int display(lua_State* L);
	static int getrange(lua_State* L);   
	static int add_parameters(lua_State* L);
	static int set_parameters(lua_State* L);	
	static int set_numchannels(lua_State* L);
	static int numchannels(lua_State* L);
	static int set_numcols(lua_State* L);
};

struct LuaPlayerBind {
  static int open(lua_State *L);
  static int create(lua_State* L);
  static int samplerate(lua_State* L);
};

template <typename T>
class range {
  public:
    typedef T value_type;
    range(T const& center) : min_(center), max_(center) {}
    range(T const& min, T const& max) : min_(min), max_(max) {}
    T min() const { return min_; }
    T max() const { return max_; }
  private:
    T min_;
    T max_;
};

template <typename T>
struct left_of_range : public std::binary_function<range<T>, range<T>, bool> {
    bool operator()(range<T> const & lhs, range<T> const & rhs) const {
        return lhs.min() < rhs.min()
            && lhs.max() <= rhs.min();
    }
};

struct SingleWorkInterface {
  virtual ~SingleWorkInterface() {}
  virtual PSArray* work(int numSamples, int val) { return 0; }
};

template<class T>
struct WaveList {
  typedef std::map<range<double>, const XMInstrument::WaveData<T>*, left_of_range<double> > Type;  
};

struct RWInterface {
  virtual ~RWInterface() {}
  virtual int work(int numSamples, float* pSamplesL, float* pSamplesR, float* fm, float* env,  SingleWorkInterface* master) = 0 { return 0; }
  virtual bool Playing() const { return false; }
  virtual void NoteOff() {}
  virtual void Start(double phase=0) {}
  virtual void set_frequency(double f) {}
  virtual double frequency() const { return 0; }
  virtual void set_quality(helpers::dsp::resampler::quality::type quality) {}
  virtual helpers::dsp::resampler::quality::type quality() const { return helpers::dsp::resampler::quality::linear; }
  virtual void Stop(double phase) {}
  virtual void SetData(WaveList<float>::Type& data) {}  
  virtual void set_gain(float gain) { }
  virtual float gain() const { return 1.0; }
  virtual double phase() const { return 0; }
  virtual void setphase(double phase) {}
  virtual void set_sync_fadeout_size(int size) {}
};

template <class T, int VOL>
class ResamplerWrap : public RWInterface {  
public:  
  ResamplerWrap(typename WaveList<T>::Type& waves) :
	adjust_vol(1/(float)VOL),
	dostop_(false),
	waves_(waves),
	speed_(1.0),
	f_(261.63),
	basef(440),	
	last_wave(0),
	gain_(1.0),
	last_sgn(1),
	startfadeout(false),
	fadeout_sc(0),
	fsize(32),
	oldtrigger(0) {	
	resampler.quality(helpers::dsp::resampler::quality::linear);
    set_frequency(f_);
    wavectrl.Playing(false);
	wave_it = waves_.find(range<double>(f_));
	last_wave = wave_it != waves_.end() ? wave_it->second : 0;
  }
  void Start(double phase=0);
  int work(int numSamples, float* pSamplesL, float* pSamplesR, float* fm, float* env, SingleWorkInterface* master);  
  bool Playing() const { return wavectrl.Playing();  }
  void Stop(double phase) {	
	if (wave_it != waves_.end()) {
	  double curr_phase = wavectrl.Position() / (double) wave_it->second->WaveLength();
	  if (curr_phase == phase) {
		wavectrl.Playing(false);
	  } else {
	    dostop_ = true;
	    stopphase_ = phase;
	  }
	} else {
	  wavectrl.Playing(false);
	}
  }
  void NoteOff() { if (wavectrl.Playing()) wavectrl.NoteOff(); }
  void set_frequency(double f);
  double frequency() const { return f_; }
  void set_quality(helpers::dsp::resampler::quality::type quality) { resampler.quality(quality); }  
  helpers::dsp::resampler::quality::type quality() const { return resampler.quality(); }  
  virtual void SetData(WaveList<float>::Type& data) { 
    assert(sizeof(T)!=sizeof(int16_t));
    waves_ = reinterpret_cast<typename WaveList<T>::Type&>(data);
	wave_it = waves_.find(range<double>(f_));
  }
  void set_gain(float gain) { gain_ = gain; }
  float gain() const { return gain_; }  
  void set_sync_fadeout_size(int size) { fsize = size; }
  virtual double phase() const {
	 if (wave_it != waves_.end()) {
	   return wavectrl.Position() / (double) wave_it->second->WaveLength();
	 }
	 return 0;
  }
  virtual void setphase(double phase) { 
	 if (wave_it != waves_.end()) {
	    wavectrl.Position((int) (phase * wave_it->second->WaveLength()));
	 }
  }
private:
  void check_wave(double f);
  int next_trigger(int numSamples, float* trigger);
  int work_trigger(int numSamples, float* pSamplesL, float* pSamplesR, float* fm, float* env, SingleWorkInterface* master);  
  int work_samples(int numSamples, float* pSamplesL, float* pSamplesR, float* fm, float* env);  
  psycle::helpers::dsp::cubic_resampler resampler;
  float adjust_vol;
  ULARGE_INTEGER m_Position;
  XMSampler::WaveDataController<T> wavectrl;
  typename WaveList<T>::Type waves_;
  typename WaveList<T>::Type::iterator wave_it;
  const XMInstrument::WaveData<T>* last_wave;
  double speed_;
  double basef;
  double f_;
  double dostop_;
  double stopphase_;
  float gain_;
  int last_sgn;
  bool startfadeout;
  int fadeout_sc; 
  int fsize; 
  float* oldtrigger;
  int n;
};

struct LuaSingleWorker : public SingleWorkInterface {
	LuaSingleWorker(lua_State *s) : L(s) {}
	~LuaSingleWorker() {}	
	virtual PSArray* work(int numSamples, int val);
private:
	lua_State *L;
};

struct LuaResamplerBind {
   	static int open(lua_State *L);
private:
	static int create(lua_State *L);
	static int createwavetable(lua_State *L);
    static int gc(lua_State* L);
	static int work(lua_State* L);
	static int noteoff(lua_State* L);
	static int isplaying(lua_State* L);
	static int start(lua_State* L);
	static int stop(lua_State* L);
	static int set_frequency(lua_State* L);
	static int frequency(lua_State* L);
	static int set_wave_data(lua_State* L);
	static int set_quality(lua_State* L);
	static int quality(lua_State*);
	static int set_sync(lua_State*);
	static int set_sync_fadeout(lua_State*);
	static int phase(lua_State*);
	static int setphase(lua_State*);
	static WaveList<float>::Type check_wavelist(lua_State* L);
};

#if !defined WINAMP_PLUGIN
struct LuaPlotterBind {
	static int open(lua_State *L);
private:
	static int create(lua_State* L);
	static int gc(lua_State* L);
	static int stem(lua_State* L);	
};
#endif // #if !defined WINAMP_PLUGIN

struct PSDelay {
  PSDelay(int k) : mem(k,0) {}
  PSArray mem;  
  void work(PSArray& x, PSArray& y);
};

struct LuaDelayBind {
	static int open(lua_State *L);
private:
	static int create(lua_State *L);
	static int work(lua_State* L);
	static int tostring(lua_State* L);
	static int gc(lua_State* L);	
};

struct WaveOscTables {	
   enum Shape {
	  SIN = 1,
	  SAW = 2,
	  SQR = 3,
	  TRI = 4
   };
private:
	WaveOscTables();
   ~WaveOscTables() {  //  AtExit ctor (invoked at application ending)
	  cleartbl(sin_tbl);
	  cleartbl(tri_tbl);
	  cleartbl(sqr_tbl);
	  cleartbl(saw_tbl);
   }
public:
	static WaveOscTables *getInstance() {  // Singleton instance
	  static WaveOscTables theInstance;
      return &theInstance;
	}

   WaveList<float>::Type tbl(Shape shape) {
     WaveList<float>::Type list;
     switch (shape) {
	   case SIN : list = getInstance()->sin_tbl; break;
	   case SAW : list = getInstance()->saw_tbl; break;
	   case SQR : list = getInstance()->sqr_tbl; break;
	   case TRI : list = getInstance()->tri_tbl; break;
	   default: list = sin_tbl;
     }
     return list;
   }

   void set_samplerate(int rate);
private:
   static void saw(float* data, int num, int maxharmonic);
   static void sqr(float* data, int num, int maxharmonic);
   static void sin(float* data, int num, int maxharmonic);
   static void tri(float* data, int num, int maxharmonic);
   static void ConstructWave(double fh,  XMInstrument::WaveData<float>& wave,
	                         void (*func)(float*, int, int),
							 int sr); 
   static void cleartbl(WaveList<float>::Type&);
   WaveList<float>::Type sin_tbl, saw_tbl, sqr_tbl, tri_tbl;  
};

struct WaveOsc {
   WaveOsc(WaveOscTables::Shape shape);
  ~WaveOsc() { delete resampler_; }
  void work(int num, float* data, float* fm, float* env, SingleWorkInterface* master) { 
	 resampler_->work(num, data, 0, fm, env, master);
  }
  float base_frequency() const { return resampler_->frequency(); }  
  void set_frequency(float f) { resampler_->set_frequency(f); }
  void Start(double phase) { resampler_->Start(phase); }
  void Stop(double phase) { resampler_->Stop(phase); }
  bool IsPlaying() const { return resampler_->Playing(); }
  void set_gain(float gain) { resampler_->set_gain(gain); }
  float gain() const { return resampler_->gain(); }
  void set_shape(WaveOscTables::Shape shape);
  WaveOscTables::Shape shape() const { return shape_; }
  void set_quality(helpers::dsp::resampler::quality::type quality) { resampler_->set_quality(quality); }  
  helpers::dsp::resampler::quality::type quality() const { return resampler_->quality(); }  
  RWInterface* resampler() { return resampler_; }
  
private:    
  RWInterface* resampler_;
  WaveOscTables::Shape shape_;
};

struct LuaWaveOscBind {
	static int open(lua_State *L);
	static std::map<WaveOsc*,  WaveOsc*> oscs;   // store map for samplerate change
    static void setsamplerate(double sr);
private:
	static int create(lua_State *L);
	static int work(lua_State* L);
	static int tostring(lua_State* L);
	static int gc(lua_State* L);	
	static int get_base_frequency(lua_State* L);
	static int set_base_frequency(lua_State* L);
	static int stop(lua_State* L);
	static int start(lua_State* L);
	static int isplaying(lua_State* L);	
	static int set_gain(lua_State* L);	
	static int gain(lua_State* L);	
	static int set_shape(lua_State* L);
	static int set_quality(lua_State* L);
    static int quality(lua_State*);
	static int set_sync(lua_State*);
	static int set_sync_fadeout(lua_State*);
	static int phase(lua_State*);
	static int setphase(lua_State*);
};

struct LuaDspMathHelper {
   static int open(lua_State *L);
private:
	static int notetofreq(lua_State* L);
	static int freqtonote(lua_State* L);
};

struct LuaDspFilterBind {
   static int open(lua_State *L);
   static std::map<psycle::helpers::dsp::Filter*, psycle::helpers::dsp::Filter*> filters;   // store map for samplerate change
   static void setsamplerate(double sr);
private:
   static int create(lua_State *L);
   static int setcutoff(lua_State* L);
   static int getcutoff(lua_State* L);
   static int setresonance(lua_State* L);
   static int getresonance(lua_State* L);
   static int setfiltertype(lua_State* L);   
   static int work(lua_State* L);
   static int tostring(lua_State* L);
   static int gc(lua_State* L);	
};

struct LuaWaveDataBind {
  static int open(lua_State *L);
private:
  static int create(lua_State *L);
  static int copy(lua_State *L);
  static int set_wave_sample_rate(lua_State *L);
  static int set_wave_tune(lua_State *L);
  static int set_wave_fine_tune(lua_State *L);
  static int set_loop(lua_State *L);
  static int set_bank(lua_State* L);
  static int gc(lua_State* L);
};

struct LEnvelope {
	LEnvelope(const std::vector<double>& times,
		      const std::vector<double>& peaks,
			  const std::vector<int>& types,
		      int suspos,
			  double startpeak,
			  int samplerate) 
		: out_(256, 0),
		  fs_(samplerate),
		  sus_(suspos),
		  startpeak_(startpeak),
	      times_(times),
		  peaks_(peaks),
	      types_(types),
	      stage_(peaks.size()) {       
	}
	~LEnvelope() {};
	void setstagetime(int stage, double t) { times_[stage]=t; }
	void setstagepeak(int stage, double p) { peaks_[stage]=p; }
	double peak(int stage) const { return peaks_[stage]; }
	double time(int stage) const { return times_[stage]; }
	int stage() const { return stage_; }
	void work(int num);
	void release();
	void start() { 
		lv_ = startpeak_;
		stage_ = 0;
		calcstage(startpeak_, peaks_[0], times_[0]);
		susdone_ = false;
	}
	bool is_playing() const {
	  return !(stage_ > peaks_.size()-1);
	}
	PSArray& out() {return out_; }
	void set_samplerate(int sr) {
		int newsc = sc_ * static_cast<int>(sr/(double)fs_);
		calcstage(stage_ == 0 ? startpeak_ : peaks_[stage_-1], peaks_[stage_],
		   times_[stage_]);		
		sc_ = newsc;
		fs_ = sr;		
	}

private:
	PSArray out_;
	int fs_, sc_, sus_, stage_, nexttime_;
	double m_, lv_, startpeak_;
	bool susdone_;
	std::vector<double> times_, peaks_;
	std::vector<int> types_;
	void calcstage(double peakStart, double peakEnd, double time, int type=0) {
	  nexttime_ = time*fs_;
	  lv_ = peakStart;
	  sc_ = 0;
	  if (type==1)  {
		m_ = 1.0 + (log(peakEnd) - log(peakStart)) / (nexttime_);
	  } else {
	    m_ = (peakEnd-peakStart)/nexttime_;		  
	  }
	}
};

struct LuaEnvelopeBind {   
   static int open(lua_State *L);
private:
   static int create(lua_State *L);
   static int createahdsr(lua_State *L);
   static int work(lua_State* L);
   static int release(lua_State* L);
   static int start(lua_State* L);
   static int isplaying(lua_State* L);
   static int setpeak(lua_State* L);
   static int peak(lua_State* L);
   static int tostring(lua_State* L);
   static int setstagetime(lua_State* L);
   static int time(lua_State* L);
   static int gc(lua_State* L);	
};


}  // namespace
}  // namespace
