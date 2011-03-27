#import "gl_view.hpp"

@interface HologramAppDelegate: NSObject<UIApplicationDelegate, GLTriangleViewDelegate>
{
	UIWindow * window;
}

- (void) applicationDidFinishLaunching: (UIApplication *) application;

@property(nonatomic, retain) IBOutlet UIWindow * window;

@end
