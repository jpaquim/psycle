///\file
///\brief audio routing system
///\author Magnus Jonsson, Bohan, Psycle Team

class CircularReferenceError : public std::runtime_error {};

/// This class represents a signal output socket. It functions
/// completely independently of Machine, but normally it would
/// be part of a machine. Any number inputs can be connected
/// (done from Input class' interface).

class Output {
public:
  Output() {
    client_count_=0;
    read_count_=0;
    buffer_=0;
    check_invariant();
  }
  ~Output() {
    check_invariant();
    assert(client_count_==0);
  }
  /// get buffer pointer for writing
  float* open() throw CircularReferenceError {
    check_invariant();
    if(buffer) {
      throw CircularReferenceError();
    }
    buffer_=BufferManager::grab_buffer();
    check_invariant();
    return buffer_;
  }
  /// call after done writing
  void close() {
    check_invariant();
    read_count_=0;
    if(client_count_==0) {
      recycle_buffer();
    }
    check_invariant();
  }
protected:
  /// this method is called when this output needs to be written.
  /// normally you will inherit this class and implement produce
  /// so that it will call your machine's process() function, but
  /// other schemes are also possible.
  virtual void produce() = 0;

private:
  friend class Input;

  /// called by an Input when it is connected to this Output.
  void input_connected() {
    check_invariant();
    client_count_++;
    check_invariant();
  }
  /// called by an Input when it is disconnected from this Output.
  void input_disconnected() {
    check_invariant();
    client_count_--;
    check_invariant();
  }

  /// called by an Input when it is opened.
  float const* input_open() {
    check_invariant();
    if(buffer_==0) {
      produce();
    }
    check_invariant();
    return buffer_;
  }

  /// called by an Input when it is closed.
  /// This will recycle the buffer if possible.
  void input_close() {
    check_invariant();
    if(++read_count_ == client_count_) {
      recycle_buffer();
    }
    check_invariant();
  }

private:

  void recycle_buffer() {
    BufferManager::release_buffer(buffer_);
    buffer_=0;
  }


  float* buffer_;
  int read_count_;
  int client_count_;

  void check_invariant() {
    assert(client_count_>=0);
    assert(read_count_>=0);
    assert(read_count_<=client_count);
    assert(read_count_==client_count ? buffer_==0 :
	   read_count_>=0            ? buffer_!=0 :
	   true);
  }
};

/// this class represents a signal input socket. It functions
/// independently of any Machine but is normally part of a
/// Machine. An Input can be connected to at most one Output.

class Input {
public:
  Input() {
    source_=0;
  }
  ~Input() {
    assert(source_==0);
    disconnect();
  }

  /// returns a handle to the source (output).
  bool is_connected() const { return source_ != 0; }
  Output const& source() const { assert(source_); return *source_; }

  /// connect this input to a source (an output), first disconnecting
  /// from it's old source if there was one.
  void connect(Output* source) {
    disconnect();
    source_=source;
    source_->input_connected();
  }
  
  /// disconnect from the source (output), unless it's already
  /// disconnected.
  void disconnect() {
    if(source_) {
      source_->input_disconnected();
      source_=0;
    }
  }
  
  /// get buffer pointer for reading
  float const* open() { return source_->input_open(); }

  /// recycles the  buffer if possible. Must call this after input_open.
  void close() { source_->input_close(); }
private:
  Output* source_;
};

/// This is what a machine class might look like
class Machine {
public:
  int num_inputs() { return inputs.size(); }
  int num_outputs() { return outputs.size(); }
  Input& input(int index) { return *inputs[index]; }
  Output& output(int index) { return *outputs[index]; }
private:
  void add_input(Input& input) { inputs.push_back(&input); }
  void add_output(Output& output) { outputs.push_back(&output); }
protected:
  std::vector<Output*> outputs;
  std::vector<Input*> inputs;
};


/// An example machine that does not
/// support input and output buffers being the same.
class ReadOnlyMachine : public Machine {
public:
  ReadOnlyMachine() {
    class MyOutput : public Output {
      Machine* me_;
    public:
      MyOutput(Machine* me) {
	me_=me;
      }
    private:
      void produce() {
	me_->process();
      }
    }
    add_output(new MyOutput(this));
    ...
    add_input(new Input());
    ...
  }

  void process() {
    for(int i=0;i<num_inputs();i++) {
      inputs(i).open();
    }
    for(int i=0;i<num_outputs();i++) {
      outputs(i).open();
    }
    // do processing
    for(int i=0;i<num_inputs();i++) {
      inputs(i).close();
    }
    for(int i=0;i<num_outputs();i++) {
      outputs(i).close();
    }
  }
};

/// An example machine that does supports (but does not require)
/// input and output buffers being the same.
class ReadWriteMachine : public Machine {
public:
  ReadWriteMachine() {
    class MyOutput : public Output {
      Machine* me_;
    public:
      MyOutput(Machine* me) {
	me_=me;
      }
      void produce() {
	me_->process();
      }
    }
    add_output(new MyOutput(this,....));
    ...
    add_input(new Input());
    ...
  }

  void process() {
    for(int i=0;i<inputs.size();i++) {
      inputs[i]->open();
    }
    for(int i=0;i<inputs.size();i++) {
      inputs[i]->close(); // dangerous stuff. will allow input pointers to be
                         // reused as output pointers.
    }
    for(int i=0;i<outputs.size();i++) {
      outputs[i].open();
    }
    // do processing
    for(int i=0;i<outputs.size();i++) {
      outputs[i].close();
    }
  }
};


/// An example machine that will process
/// one input buffer at a time while
/// recycling the input buffers it doesn't need.
class OneAtATimeMachine : public Machine {
public:
  OneAtATimeMachine() {
    class MyOutput : public Output {
      Machine* me_;
    public:
      MyOutput(Machine* me) {
	me_=me;
      }
      void produce() {
	me_->process();
      }
    }
    add_output(new MyOutput(this,....));
    ...
    add_input(new Input());
    ...
  }

  void process() {
    for(int i=0;i<outputs.size();i++) {
      outputs[i].open();
    }
    for(int i=0;i<inputs.size();i++) {
      inputs[i].open();
      // do processing...
      inputs[i].close();
    }
    for(int i=0;i<outputs.size();i++) {
      outputs[i].close();
    }
  }
};
