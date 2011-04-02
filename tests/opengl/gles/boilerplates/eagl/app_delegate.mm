#import "app_delegate.hpp"

#define DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) / 180.0 * M_PI)
#define kRenderingFrequency 100.0

GLfloat static accelerometer_x = 0;
GLfloat static accelerometer_y = 0;
GLfloat static accelerometer_z = 0;

void accelerometer_callback(float x, float y, float z, void*) {
	accelerometer_x = x;
	accelerometer_y = y;
	accelerometer_z = z;
}

@implementation HologramAppDelegate

@synthesize window;

- (void) drawView: (GLView*) view {
	const GLfloat triVertices[] = { 
		0.0f, 1.0f, 0.0f, 
		-1.0f, -1.0f, 0.0f, 
		1.0f, -1.0f, 0.0f 
	}; 
	const GLfloat quadVertices[] = {
		-1.0f,  1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};
	glClear(GL_COLOR_BUFFER_BIT); 
	glLoadIdentity(); 
	GLfloat const z = -6;//accelerometer_z;
	GLfloat static x = 0;
	GLfloat static y = 0;
	glTranslatef(x, y, z);
	GLfloat const h0 = -6;
	GLfloat const h1 = 8;
	y += 0.5; if(y > h1) y = h0;
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, triVertices); 
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 3); 
	glTranslatef(0, -2, 0);
	glVertexPointer(3, GL_FLOAT, 0, quadVertices);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4); 
}

- (void) setupView: (GLView*) view {
	const GLfloat zNear = 0.1, 
	zFar = 1000.0, 
	fieldOfView = 60.0; 
	GLfloat size; 
	
	glMatrixMode(GL_PROJECTION); 
	size = zNear * tanf(DEGREES_TO_RADIANS(fieldOfView) / 2.0); 
	CGRect rect = view.bounds; 
	glFrustumf(-size, size, -size / (rect.size.width / rect.size.height), size / 
			   (rect.size.width / rect.size.height), zNear, zFar); 
	glViewport(0, 0, rect.size.width, rect.size.height); 
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity(); 
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
}

- (void) applicationDidFinishLaunching: (UIApplication*) application {
	CGRect	rect = [[UIScreen mainScreen] bounds];
	
	window = [[UIWindow alloc] initWithFrame:rect];
	
	GLView *glView = [[GLView alloc] initWithFrame:rect];
	[window addSubview:glView];

	glView.delegate = self;
	glView.animationInterval = 1.0 / kRenderingFrequency;
	[glView startAnimation];
	[glView release];
	
	[window makeKeyAndVisible];
}

- (void) dealloc {
	[window release];
	[super dealloc];
}

@end
