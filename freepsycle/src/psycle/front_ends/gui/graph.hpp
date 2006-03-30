// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief \interface psycle::front_ends::gui::graph
#pragma once
#include "forward_declarations.hpp"
#include "color.hpp"
#include <psycle/engine/engine.hpp>
#include <psycle/host/host.hpp>
#include <universalis/compiler/cast.hpp>
#include <boost/signal.hpp>
#include <gtkmm/adjustment.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/box.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/progressbar.h>
#include <libgnomecanvasmm/canvas.h>
#include <libgnomecanvasmm/group.h>
#include <libgnomecanvasmm/rect.h>
#include <libgnomecanvasmm/line.h>
#include <libgnomecanvasmm/text.h>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PACKAGENERIC__MODULE__SOURCE__PSYCLE__FRONT_ENDS__GUI__GRAPH
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
			typedef ::guint time;
			
			typedef
				Gnome::Canvas::
					//Canvas
					CanvasAA
						canvas_base;
			
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK canvas : public canvas_base
			{
				public:
					typedef canvas_base base;
					canvas(graph &);

				protected:
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES on_event(GdkEvent *);

				private:
					Gnome::Canvas::Group group_;
					
				public:
					graph inline & graph_instance() { return graph_; }
					inline operator graph & () { return graph_instance(); }
				private:
					graph & graph_;
					
				public:
					void inline selected_port(port & port) { selected_port_ = &port; }
					port inline * const selected_port() { return selected_port_; }
				private:
					port * selected_port_;

				protected:
					Gnome::Canvas::Line inline & line() { return line_; }
				private:
					Gnome::Canvas::Line line_;
			};
			
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK graph : public Gtk::VBox
			{
				public:
					typedef Gtk::VBox base;
					graph(std::string const & name, host::plugin_resolver & resolver);
					virtual ~graph() throw();

				public:
					host::plugin_resolver inline & resolver() { return resolver_; }
				private:
					host::plugin_resolver & resolver_;

				public:
					engine::graph inline & graph_instance() { return graph_; }
					inline operator engine::graph & () { return graph_instance(); }
				private:
					engine::graph graph_;
					
				protected:
					host::schedulers::single_threaded::scheduler inline & scheduler() throw() { return scheduler_; }
				private:
					host::schedulers::single_threaded::scheduler scheduler_;

				protected:
					void on_start();
					Gtk::ToggleButton inline & start() throw() { return start_; }
				private:
					Gtk::ToggleButton start_;

				public:
					canvas inline & canvas_instance() throw() { return canvas_; }
					inline operator canvas & () throw() { return canvas_instance(); }
					inline operator Gnome::Canvas::Group & () throw() { return *canvas_instance().root(); }
					inline operator Gnome::Canvas::Group * () throw() { return canvas_instance().root(); }
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES on_size_allocate(Gtk::Allocation & allocation);
					void on_zoom();
				private:
					canvas canvas_;
					Gtk::ScrolledWindow scroll_;
					Gtk::Adjustment adjustment_;
					Gtk::SpinButton spin_;

				private:
					Gtk::VBox box_;
			};
			
			typedef universalis::compiler::numeric<>::floating_point real;

			class UNIVERSALIS__COMPILER__DYNAMIC_LINK contraption : public Gnome::Canvas::Group
			{
				public:
					contraption(Gnome::Canvas::Group & parent, real const & x, real const & y, color const &, std::string const & text);
					
				protected:
					Gnome::Canvas::Rect inline & rectangle() throw() { return rectangle_; }
				private:
					Gnome::Canvas::Rect rectangle_;

				///\name text
				///\{
					public:
						void text(std::string const & text);
					protected:
						Gnome::Canvas::Text inline & text_shadow() throw() { return text_shadow_; }
						Gnome::Canvas::Text inline & text_highlight() throw() { return text_highlight_; }
					private:
						Gnome::Canvas::Text text_shadow_, text_highlight_;
				///\}

				///\name colors
				///\{
					public:
						color inline & fill_color() { return fill_color_; }
						color inline & outline_color() { return outline_color_; }
						color inline & highlight_fill_color() { return highlight_fill_color_; }
						color inline & highlight_outline_color() { return highlight_outline_color_; }
					private:
						color fill_color_, outline_color_, highlight_fill_color_, highlight_outline_color_;
				///\}

				///\name signals
				///\{
					public:
						boost::signal<void (contraption &)> inline & signal_enter()  { return signal_enter_; }
						boost::signal<void (contraption &)> inline & signal_select() { return signal_select_; }
						boost::signal<void (contraption &)> inline & signal_move()   { return signal_move_; }
						boost::signal<void (contraption &)> inline & signal_leave()  { return signal_leave_; }
					private:
						boost::signal<void (contraption &)> signal_enter_, signal_move_, signal_select_, signal_leave_;
					protected:
						bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES on_event(GdkEvent *);
				///\}

				///\name dragging
				///\{
					protected:
						void dragging_start(real const & x, real const & y, time const & time);
						void dragging(real const & x, real const & y);
						void dragging_stop(time const & time);
						bool inline const & dragging() { return dragging_; }
					private:
						bool dragging_;
						real dragging_x_, dragging_y_;
				///\}
			};

			class UNIVERSALIS__COMPILER__DYNAMIC_LINK node : public Gnome::Canvas::Group
			{
				public:
					node(graph &, engine::node &, real const & x, real const & y);
					
				public:
					graph inline & graph_instance() throw() { return graph_; }
				private:
					graph & graph_;
				
				protected:
					contraption inline & contraption_instance() throw() { return contraption_; }
				private:
					contraption contraption_;
				
				public:
					engine::node inline & node_instance() throw() { return node_; }
					inline operator engine::node & () throw() { return node_instance(); }
				private:
					engine::node & node_;
					
				protected:
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES on_event(GdkEvent *);
					
				friend class port;
		
				public:
					typedef std::vector<ports::output*> output_ports_type;
					/// the output ports owned by this node
					output_ports_type inline const & output_ports() const { return output_ports_; }
					/// finds an output port by its name
					ports::output * const output_port(std::string const & name) const;
				private:
					friend class ports::output;
					output_ports_type output_ports_;
		
				friend class ports::input;
	
				public:
					typedef std::vector<ports::inputs::single*> input_ports_type;
					/// the input ports owned by this node
					input_ports_type inline const & input_ports() const { return input_ports_; }
					/// finds an input port by its name
					ports::input * const input_port(std::string const & name) const;
				private:
					friend class ports::inputs::single;
					input_ports_type input_ports_;
					
				public:
					ports::inputs::multiple inline * const multiple_input_port() const throw() { return multiple_input_port_; }
				protected:
					void inline multiple_input_port(ports::inputs::multiple & multiple_input_port) throw() { assert(!this->multiple_input_port()); this->multiple_input_port_ = &multiple_input_port; }
				private:
					friend class ports::inputs::multiple;
					ports::inputs::multiple * multiple_input_port_;
			};

			class UNIVERSALIS__COMPILER__DYNAMIC_LINK port
			:
				public universalis::compiler::cast::underlying_wrapper<engine::port>,
				public Gnome::Canvas::Group
			{
				protected:
					port(node &, underlying_type &, real const & x, real const & y, color const &);
				
				public:
					node inline & node_instance() throw() { return node_; }
				private:
					node & node_;

				protected:
					contraption inline & contraption_instance() throw() { return contraption_; }
				private:
					contraption contraption_;
				
				protected:
					Gnome::Canvas::Line inline & line() throw() { return line_; }
				private:
					Gnome::Canvas::Line line_;

				protected:
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES on_event(GdkEvent *);
					void on_select(contraption &);
					void on_move(contraption &);
			};

			namespace ports
			{
				namespace cast
				{
					template<typename Derived_Underlying, typename Base_Wrapper>
					class underlying_wrapper
					:
						public Base_Wrapper,
						public universalis::compiler::cast::underlying_wrapper<Derived_Underlying, Base_Wrapper>
					{
						UNIVERSALIS__COMPILER__CAST__UNDERLYING_WRAPPER__DISAMBIGUATES(Derived_Underlying, underlying_wrapper)
						protected:
							inline  underlying_wrapper(node & node, Derived_Underlying & underlying, real const & x, real const & y, color const & color) : Base_Wrapper(node, underlying, x, y, color) {}
					};
				}

				class UNIVERSALIS__COMPILER__DYNAMIC_LINK output : public cast::underlying_wrapper<engine::ports::output, port>
				{
					public:
						output(node &, underlying_type &, real const & x, real const & y);
				};

				class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public cast::underlying_wrapper<engine::ports::input, port>
				{
					protected:
						input(node &, underlying_type &, real const & x, real const & y, color const &);
				};
				
				namespace inputs
				{
					class UNIVERSALIS__COMPILER__DYNAMIC_LINK single : public cast::underlying_wrapper<engine::ports::inputs::single, input>
					{
						public:
							single(node &, underlying_type &, real const & x, real const & y);
					};

					class UNIVERSALIS__COMPILER__DYNAMIC_LINK multiple : public cast::underlying_wrapper<engine::ports::inputs::multiple, input>
					{
						public:
							multiple(node &, underlying_type &, real const & x, real const & y);
					};
				}
			}
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
