// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2010-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#import <UIKit/UIKit.h>

int main(int argc, char *argv[])
{
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	/* never returns */ int returnCode = UIApplicationMain(argc, argv, nil, @"HologramAppDelegate");
	[pool release];
    return returnCode;
}

