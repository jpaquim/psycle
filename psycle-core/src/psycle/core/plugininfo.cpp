// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "plugininfo.h"

namespace psycle { namespace core {

PluginInfo::PluginInfo(){}
PluginInfo::PluginInfo(MachineRole::type role, std::string name, std::string author, std::string desc, std::string version, std::string libname, std::string categ)
:
	role_(role),
	name_(name),
	author_(author),
	desc_(desc),
	version_(version),
	libName_(libname),
	fileTime_( 0 ),
	allow_( true ),
	category_(categ)
{
}

PluginInfo::~PluginInfo() {
}

void PluginInfo::setRole( MachineRole::type role ) {
	role_ = role;
}

MachineRole::type PluginInfo::role() const {
	return role_;
}

void PluginInfo::setName( const std::string & name ) {
	name_ = name;
}

const std::string & PluginInfo::name() const {
	return name_;
}

void PluginInfo::setAuthor( const std::string & author ) {
	author_ = author;
}

const std::string & PluginInfo::author() const {
	return author_;
}

void PluginInfo::setDesc( const std::string & desc ) {
	desc_ = desc;
}

const std::string & PluginInfo::desc() const {
	return desc_;
}

void PluginInfo::setVersion( const std::string & version ) {
	version_ = version;
}

const std::string & PluginInfo::version() const {
	return version_;
}

void PluginInfo::setLibName( const std::string & libName ) {
	libName_ = libName;
}

const std::string & PluginInfo::libName() const {
	return libName_;
}

void PluginInfo::setFileTime( time_t time ) {
	fileTime_ = time;
}

time_t PluginInfo::fileTime() const {
	return fileTime_;
}

void PluginInfo::setError( const std::string & error ) {
	error_ = error;
}

const std::string PluginInfo::error() const {
	return error_;
}

void PluginInfo::setAllow( bool allow ) {
	allow_ = allow;
}

bool PluginInfo::allow() const {
	return allow_;
}

void PluginInfo::setCategory( const std::string & category ) {
	category_ = category;
}

const std::string & PluginInfo::category() const {
	return category_;
}

}}
