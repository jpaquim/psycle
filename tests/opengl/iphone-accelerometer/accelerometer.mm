#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

#include "accelerometer.hpp"

typedef void* IOHIDServiceRef;
typedef void* IOHIDEventSystemRef;
typedef void* IOHIDEventRef;
extern "C" float IOHIDEventGetFloatValue(IOHIDEventRef ref, int param);
extern "C" int IOHIDEventGetType(IOHIDEventRef);
extern "C" IOHIDEventSystemRef IOHIDEventSystemCreate(void*);
extern "C" CFArrayRef IOHIDEventSystemCopyMatchingServices(IOHIDEventSystemRef, CFDictionaryRef, int, int, int);
extern "C" int IOHIDServiceSetProperty(io_registry_entry_t, CFStringRef, CFNumberRef);
typedef void (*HandleHIDEventCallback)(void*, void*, IOHIDServiceRef, IOHIDEventRef);
extern "C" int IOHIDEventSystemOpen(IOHIDEventSystemRef, HandleHIDEventCallback, void*, void*, void*);

accelerometer_callback_type accelerometer_callback_;
void * data_;

void handleHIDEvent(void*, void*, IOHIDServiceRef, IOHIDEventRef ptr) {
	int type = IOHIDEventGetType(ptr);
	if (type == 13) {
		float x,y,z;
		x = IOHIDEventGetFloatValue(ptr, 0xd0000);
		y = IOHIDEventGetFloatValue(ptr, 0xd0001);
		z = IOHIDEventGetFloatValue(ptr, 0xd0002);
		accelerometer_callback_(x, y, z, data_);
	}
}

#define expect(x) if(!x) { printf("failed: %s\n", #x); return; }

void accelerometer_init(int hz, accelerometer_callback_type accelerometer_callback, void * data) {
	mach_port_t master;
	expect(0 == IOMasterPort(MACH_PORT_NULL, &master));

	int page = 0xff00, usage = 3;

	CFNumberRef nums[2];
	CFStringRef keys[2];
	keys[0] = CFStringCreateWithCString(0, "PrimaryUsagePage", 0);
	keys[1] = CFStringCreateWithCString(0, "PrimaryUsage", 0);
	nums[0] = CFNumberCreate(0, kCFNumberSInt32Type, &page);
	nums[1] = CFNumberCreate(0, kCFNumberSInt32Type, &usage);
	CFDictionaryRef dict = CFDictionaryCreate(0, (const void**)keys, (const void**)nums, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	expect(dict);

	IOHIDEventSystemRef sys = (IOHIDEventSystemRef) IOHIDEventSystemCreate(0);
	expect(sys);

	CFArrayRef srvs = (CFArrayRef)IOHIDEventSystemCopyMatchingServices(sys, dict, 0, 0, 0);
	expect(CFArrayGetCount(srvs)==1);

	io_registry_entry_t serv = (io_registry_entry_t)CFArrayGetValueAtIndex(srvs, 0);
	expect(serv);

	CFStringRef cs = CFStringCreateWithCString(0, "ReportInterval", 0);
	int rv = 1000000/hz;
	CFNumberRef cn = CFNumberCreate(0, kCFNumberSInt32Type, &rv);

	int res = IOHIDServiceSetProperty(serv, cs, cn);
	expect(res == 1);

	data_ = data;
	accelerometer_callback_ = accelerometer_callback;
	res = IOHIDEventSystemOpen(sys, handleHIDEvent, 0, 0, 0);
	expect(res != 0);
}

void accelerometer_deinit() {
	//IOHIDEventSystemClose(system, NULL);
	//CFRelease(system);
}

