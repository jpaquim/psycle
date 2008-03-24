// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008 psycledelics <http://psycle.sourceforge.net> ; Johan Boule <bohan@jabber.org>

///\file
///\brief LV2 extension for sparse audio buffers (SAB) 
///
/// This header defines the code portion of a proposal for a LV2 extension
/// called "sparse audio buffer" (SAB for short), with URI
/// <https://psycle.svn.sourceforge.net/svnroot/psycle/branches/bohan/lv2/extensions/sparse_audio_buffer.h>
///
/// This extension is a type of audio buffer which allows for sparse data.

#pragma once
 
char const lv2_sab_uri [] = "https://psycle.svn.sourceforge.net/svnroot/psycle/branches/bohan/lv2/extensions/sparse_audio_buffer.h";

/// the type of the elements contained in a sparse audio buffer.
typedef struct {

	/// logical time frame of the sample value relative to the start of the buffer, expressed in samples.
	uint32_t index;
	
	/// the sample value.
	float sample;
	
} LV2_SAB_Value;

/// a hint for processing loops to decide how to handle a sparse audio buffer.
enum LV2_SAB_Buffer_Flag {
	lv2_sab_buffer_empty, /// < indicates that the buffer contains no value at all, and thus can be ignored.
	lv2_sab_buffer_discrete, /// < indicates that the buffer has at least one value.
	lv2_sab_buffer_continuous /// < indicates that the buffer contains a value for every time frame.
};

/// a sparse audio buffer.
typedef struct {

	/// the element count allocated in memory by the host for this buffer.
	/// The plugin is not interested in this field ; it will read the count field instead.
	/// Invariant: count <= capacity
	uint32_t capacity;

	/// the maximum element count the plugin shall read from or write to the buffer.
	/// The host must set this field to the same value that the number of sample frames
	/// it asks the plugin to process when calling run().
	/// The plugin must not change this field.
	/// Note for an input port, the plugin will stop reading
	/// the elements as soon as values[x].index >= count.
	/// Invariant: count <= capacity
	uint32_t count;
	
	/// a hint for processing loops to decide how to handle a LV2 sparse audio buffer.
	/// For output ports, the plugin must set the flag appropriatly.
	LV2_SAB_Buffer_Flag flag;

	// followed by:
	// LV2_SAB_Value[capacity] values;
	
} LV2_SAB_Buffer;
