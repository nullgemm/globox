#ifndef H_GLOBOX_INTERNAL_X11_COMMON
#define H_GLOBOX_INTERNAL_X11_COMMON

#include "include/globox.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <xcb/xcb.h>

// # private helpers
enum x11_atoms
{
	X11_ATOM_STATE_MAXIMIZED_HORIZONTAL = 0,
	X11_ATOM_STATE_MAXIMIZED_VERTICAL,
	X11_ATOM_STATE_FULLSCREEN,
	X11_ATOM_STATE_HIDDEN,
	X11_ATOM_STATE,
	X11_ATOM_ICON,
	X11_ATOM_HINTS_MOTIF,
	X11_ATOM_BLUR_KDE,
	X11_ATOM_BLUR_DEEPIN,
	X11_ATOM_PROTOCOLS,
	X11_ATOM_DELETE_WINDOW,
	X11_ATOM_COUNT,
};

struct x11_thread_event_loop_data
{
	struct globox* globox;
	struct x11_platform* platform;
	struct globox_error_info* error;
};

struct x11_platform
{
	pthread_mutex_t mutex_main;
	pthread_mutex_t mutex_block;
	pthread_cond_t cond_main;

	// connection init
	bool closed;
	xcb_connection_t* conn;

	int screen_id;
	xcb_screen_t* screen_obj;
	xcb_window_t root_win;

	xcb_atom_t atoms[X11_ATOM_COUNT];

	// window creation
	uint32_t attr_mask;
	uint32_t attr_val[3];
	xcb_window_t win;
	int visual_depth;
	xcb_visualid_t visual_id;

	// event handling
	pthread_t thread_event_loop;
	struct x11_thread_event_loop_data thread_event_loop_data;
};

void globox_x11_common_init(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_error_info* error);

void globox_x11_common_clean(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_error_info* error);

void globox_x11_common_window_create(
	struct globox* context,
	struct x11_platform* platform,
	void** features,
	struct globox_error_info* error);

void globox_x11_common_window_destroy(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_error_info* error);

void globox_x11_common_window_start(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_error_info* error);

void globox_x11_common_window_block(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_error_info* error);

void globox_x11_common_window_stop(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_error_info* error);


void globox_x11_common_init_events(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_config_events* config,
	struct globox_error_info* error);

enum globox_event globox_x11_common_handle_events(
	struct globox* context,
	struct x11_platform* platform,
	void* event,
	struct globox_error_info* error);

struct globox_config_features* globox_x11_common_init_features(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_error_info* error);

bool globox_x11_common_feature_set_interaction(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_feature_interaction* config,
	struct globox_error_info* error);

bool globox_x11_common_feature_set_state(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_feature_state* config,
	struct globox_error_info* error);

bool globox_x11_common_feature_set_title(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_feature_title* config,
	struct globox_error_info* error);

bool globox_x11_common_feature_set_icon(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_feature_icon* config,
	struct globox_error_info* error);

bool globox_x11_common_feature_set_size(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_feature_size* config,
	struct globox_error_info* error);

bool globox_x11_common_feature_set_pos(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_feature_pos* config,
	struct globox_error_info* error);

bool globox_x11_common_feature_set_frame(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_feature_frame* config,
	struct globox_error_info* error);

bool globox_x11_common_feature_set_background(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_feature_background* config,
	struct globox_error_info* error);

bool globox_x11_common_feature_set_vsync_callback(
	struct globox* context,
	struct x11_platform* platform,
	struct globox_feature_vsync_callback* config,
	struct globox_error_info* error);

#endif
