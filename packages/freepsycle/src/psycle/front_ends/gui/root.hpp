///\file
///\brief \interface psycle::front_ends::gui::root
#pragma once
#include <psycle/engine/engine.hpp>
#include "graph.hpp"
#include <gtkmm/window.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
			class root : public Gtk::Window
			{
				public:
					root(engine::hello &);
					virtual ~root() throw();
				
				protected:
					engine::hello inline & hello() throw() { return hello_; }
					engine::hello inline const & hello() const throw() { return hello_; }
				private:
					engine::hello & hello_;
				
				private:
					host::plugin_resolver resolver_;
				protected:
					host::plugin_resolver inline & resolver() throw() { return resolver_; }

				protected:
					void virtual on_button_clicked();
					Gtk::Button inline & button() throw() { return button_; }
					Gtk::Button inline const & button() const throw() { return button_; }
				private:
					Gtk::Button button_;
				
				protected:
					Gtk::Label inline & label() throw() { return label_; }
					Gtk::Label inline const & label() const throw() { return label_; }
				private:
					Gtk::Label label_;
				
				protected:
					graph inline & graph_instance() throw() { return graph_; }
					graph inline const & graph_instance() const throw() { return graph_; }
				private:
					graph graph_;
					
				private:
					Gtk::Frame hello_frame_, graph_frame_;
					Gtk::VBox v_box_;
					Gtk::HBox h_box_;
			};
		}
	}
}
