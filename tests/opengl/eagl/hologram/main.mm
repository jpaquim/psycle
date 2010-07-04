// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2010-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#import "main.hpp"

#import <CoreFoundation/CoreFoundation.h>
#import <UIKit/UIKit.h>
#import <UIKit/UITextView.h>
#import <UIKit/UIFont.h>

//#import <OpenGLES/EAGL.h>
//#import <OpenGLES/ES2/gl.h>

int main(int argc, char **argv) {
    NSAutoreleasePool *autoreleasePool = [
        [ NSAutoreleasePool alloc ] init 
    ];

    int returnCode = UIApplicationMain(argc, argv, @"Hologram", @"Hologram");
    [ autoreleasePool release ];
    return returnCode;
}

@implementation Hologram

- (void)applicationDidFinishLaunching: (UIApplication *) application {
    window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];

    CGRect windowRect = [ [ UIScreen mainScreen ] applicationFrame ];
    windowRect.origin.x = windowRect.origin.y = 0.0f;

    // Create the window object and assign it to the
    // window instance variable of the application delegate.

    window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    window.backgroundColor = [UIColor whiteColor];

    CGRect txtFrame = CGRectMake(50,150,200,200);
    UITextView *txtView = [[UITextView alloc] initWithFrame:txtFrame];
    txtView.text = @"Hologram";
    UIFont *font = [UIFont boldSystemFontOfSize:36.0];
    txtView.font = font;

    [ window  addSubview: txtView ];

    [txtView release];

    // Show the window.

    [window makeKeyAndVisible];
}

@end

