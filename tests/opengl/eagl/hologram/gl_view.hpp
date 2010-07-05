#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

@protocol GLTriangleViewDelegate;

@interface GLView: UIView
{
	@private
	// The pixel dimensions of the backbuffer
	GLint backingWidth;
	GLint backingHeight;
	
	EAGLContext *context;
	GLuint viewRenderbuffer, viewFramebuffer;
	GLuint depthRenderbuffer;
	NSTimer *animationTimer;
	NSTimeInterval animationInterval;

	id<GLTriangleViewDelegate> delegate;
	BOOL delegateSetup;
}

@property(nonatomic, assign) id<GLTriangleViewDelegate> delegate;

-(void)startAnimation;
-(void)stopAnimation;
-(void)drawView;

@property NSTimeInterval animationInterval;

// TODO private
- (id)initGLES;
- (BOOL)createFramebuffer;
- (void)destroyFramebuffer;

@end

@protocol GLTriangleViewDelegate<NSObject>

@required
-(void)drawView:(GLView*)view;

@optional
-(void)setupView:(GLView*)view;

@end
