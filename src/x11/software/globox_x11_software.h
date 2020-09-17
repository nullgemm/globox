#ifndef H_GLOBOX_X11_SOFTWARE
#define H_GLOBOX_X11_SOFTWARE

#include "globox.h"
#include "x11/globox_x11.h"
// x11 includes
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/shm.h>

struct globox_x11_software
{
	xcb_shm_segment_info_t globox_x11_software_shm;
	xcb_gcontext_t globox_x11_software_gfx;
	xcb_pixmap_t globox_x11_software_pixmap;
	bool globox_x11_software_pixmap_update;
	bool globox_x11_software_shared_pixmaps;

	uint32_t globox_x11_software_buffer_width;
	uint32_t globox_x11_software_buffer_height;
};

#endif