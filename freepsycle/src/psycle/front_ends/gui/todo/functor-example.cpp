	class GUI
	{
	public:
		GUI();
		void start() throw(boost::thread_resource_error);
		void operator()() throw();
	private:
		boost::thread * thread_;
	};

	GUI::GUI() : thread_(0) {}

	void GUI::operator()() throw()
	{
		psycle::front_ends::gui::main();
	}

	void GUI::start() throw(boost::thread_resource_error)
	{
		if(thread_) return;
		try
		{
			template<typename Functor> class Thread
			{
			public:
				inline Thread(Functor & functor) : functor_(functor) {}
				inline void operator()() throw() { functor_(); }
			private:
				Functor & functor_;
			};
			thread_ = new boost::thread(Thread<GUI>(*this));
		}
		catch(const boost::thread_resource_error & e)
		{
			std::cerr << typeid(e).name() << ": " << e.what() << std::endl;
			throw;
		}
	}
