#ifndef H_GLOBOX_MACOS_SYMBOLS
#define H_GLOBOX_MACOS_SYMBOLS

#include <objc/message.h>
#include <objc/runtime.h>

#include "macos/globox_macos_types.h"

// constants and enums
#define NSEventMaskAny 0xFFFFFFFFFFFFFFFF

enum NSStringEncoding
{
	NSUTF8StringEncoding = 4,
};

enum NSBackingStoreType
{
	NSBackingStoreBuffered = 2,
};

enum NSEventSubtype
{
	NSEventSubtypeApplicationActivated = 1,
};

enum NSEventType
{
	NSEventTypeLeftMouseDown = 1,
	NSEventTypeLeftMouseUp = 2,
	NSEventTypeMouseMoved = 5,
	NSEventTypeAppKitDefined = 13,
	NSEventTypeApplicationDefined = 15,
};

enum NSApplicationPresentationOptions
{
	NSApplicationPresentationHideDock = 2,
	NSApplicationPresentationHideMenuBar = 8,
	NSApplicationPresentationDisableAppleMenu = 16,
	NSApplicationPresentationFullScreen = 1024,
};

enum NSWindowStyleMask
{
	NSWindowStyleMaskTitled = 1,
	NSWindowStyleMaskClosable = 2,
	NSWindowStyleMaskMiniaturizable = 4,
};

enum NSWindowCollectionBehavior
{
	NSWindowCollectionBehaviorParticipatesInCycle = 1 << 5,
	NSWindowCollectionBehaviorFullScreenPrimary = 1 << 7,
	NSWindowCollectionBehaviorFullScreenAllowsTiling = 1 << 11,
};

enum NSTrackingAreaOptions
{
	NSTrackingMouseMoved = 0x02,
	NSTrackingActiveAlways = 0x80,
	NSTrackingInVisibleRect = 0x200,
};

// extra enums required as part of the private API
enum macos_bitmapinfo
{
	GLOBOX_MACOS_BITMAPINFO_PREMULTIPLIEDFIRST = 2,
	GLOBOX_MACOS_BITMAPINFO_BYTEORDER32LITTLE = 2 << 12,
};

enum macos_events_subtype
{
	GLOBOX_MACOS_EVENT_WINDOW_STATE,
	GLOBOX_MACOS_EVENT_WINDOW_RESIZE,
	GLOBOX_MACOS_EVENT_WINDOW_CLOSE,
};

enum macos_events_window_state
{
	GLOBOX_MACOS_EVENT_WINDOW_MINIMIZE_ON,
	GLOBOX_MACOS_EVENT_WINDOW_MINIMIZE_OFF,
	GLOBOX_MACOS_EVENT_WINDOW_FULLSCREEN_ON,
	GLOBOX_MACOS_EVENT_WINDOW_FULLSCREEN_OFF,
	GLOBOX_MACOS_EVENT_WINDOW_MAXIMIZE_ON,
	GLOBOX_MACOS_EVENT_WINDOW_MAXIMIZE_OFF,
};

// objc_msgSend variants
extern void (*macos_msg_void_none)(id, SEL);
extern id (*macos_msg_id_none)(id, SEL);
extern double (*macos_msg_double_none)(id, SEL);
extern void (*macos_msg_void_bool)(id, SEL, BOOL);
extern void (*macos_msg_void_id)(id, SEL, id);
extern void (*macos_msg_void_voidptr)(id, SEL, void*);
extern BOOL (*macos_msg_bool_none)(id, SEL);
extern void (*macos_msg_void_int)(id, SEL, int);
extern struct macos_point (*macos_msg_point_none)(id, SEL);
extern struct macos_rect (*macos_msg_rect_none)(id, SEL);
extern void (*macos_msg_void_size)(id, SEL, struct macos_size);
extern id* (*macos_msg_idptr_none)(id, SEL);
extern id* (*macos_msg_poll)(id, SEL, unsigned long long, id*, id*, BOOL);
extern void (*macos_msg_resize)(id, SEL, struct macos_rect, BOOL);
extern id (*macos_msg_id_size)(id, SEL, struct macos_size);
extern unsigned long (*macos_msg_event_type)(id*, SEL);
extern short (*macos_msg_event_subtype)(id*, SEL);
extern id (*macos_msg_string)(id, SEL, char*, int);
extern void (*macos_msg_post)(id, SEL, id*, BOOL);
extern id (*macos_msg_win)(id, SEL, struct macos_rect, int, int, BOOL);
extern void (*macos_msg_void_ulong)(id, SEL, unsigned long);
extern id* (*macos_msg_idptr_int)(id, SEL, int);
extern id (*macos_msg_id_rect)(id, SEL, struct macos_rect);
extern void (*macos_msg_subview)(id, SEL, id, int, id);
extern id (*macos_msg_id_trackingarea)(
	id, SEL, struct macos_rect, unsigned long, id, id);
extern id* (*macos_msg_event)(
	id, SEL, unsigned long, struct macos_point, unsigned long,
	double, long, id, short, long, long);

// extern symbols
extern id* NSDefaultRunLoopMode;
extern id CGColorSpaceCreateDeviceRGB();
extern id CGBitmapContextCreate(void*, size_t, size_t, size_t, size_t, id, uint32_t);
extern void CGImageRelease(id);
extern id CGBitmapContextCreateImage(id);

#endif
