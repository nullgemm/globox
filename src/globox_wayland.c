#define _XOPEN_SOURCE 700
#ifdef GLOBOX_WAYLAND

#include "globox.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <unistd.h>
#include "xdg-shell-client-protocol.h"
#include "wayland.h"
#include "nix.h"

#ifdef GLOBOX_RENDER_OGL
#include <wayland-egl.h>
#include <EGL/egl.h>
#endif

inline bool globox_open(
	struct globox* globox,
	enum globox_state state,
	const char* title,
	int32_t x,
	int32_t y,
	uint32_t width,
	uint32_t height,
	bool frame_event,
	void (*callback)(
		void* event,
		void* data),
	void* data)
{
	bool err;

	globox->init_x = x;
	globox->init_y = y;
	globox->width = width;
	globox->height = height;
	globox->buf_width = width;
	globox->buf_height = height;
	globox->redraw = true;
	globox->frame_event = frame_event;
	globox->closed = false;
	globox->wl_icon = NULL;
	globox->wl_icon_len = 0;
	globox->wl_screen_width = 1920;
	globox->wl_screen_height = 1080;
	globox->event_callback = callback;
	globox->event_callback_data = data;

	// callbacks
	globox->wl_buffer_listener.release = wl_buffer_release;
	globox->xdg_surface_listener.configure = xdg_surface_configure;
	globox->xdg_wm_base_listener.ping = xdg_wm_base_ping;
	globox->wl_output_listener.geometry = wl_output_geometry;
	globox->wl_output_listener.mode = wl_output_mode;
	globox->wl_output_listener.done = wl_output_done;
	globox->wl_output_listener.scale = wl_output_scale;
	globox->wl_registry_listener.global = registry_global;
	globox->wl_registry_listener.global_remove = registry_global_remove;

#if 0
	globox->wl_callback_geometry = NULL;
	globox->wl_callback_scale = NULL;
#endif

	// surface callbacks
	globox->wl_surface_frame_listener.done = wl_surface_frame_done;
	globox->xdg_toplevel_listener.configure = xdg_toplevel_configure;
	globox->xdg_toplevel_listener.close = xdg_toplevel_close;
	globox->wl_wait_dispatch = true;

#ifdef GLOBOX_RENDER_OGL
	globox->wl_egl_display = EGL_NO_DISPLAY;
	globox->wl_egl_context = EGL_NO_CONTEXT;
	globox->wl_egl_surface = EGL_NO_SURFACE;
#if 0
	globox->wl_egl_window;
	globox->wl_egl_config;
#endif
#endif

	// init
	globox->wl_display = NULL;
	globox->wl_display = wl_display_connect(NULL);

	if (globox->wl_display == NULL)
	{
		return false;
	}

	globox->fd.descriptor = wl_display_get_fd(globox->wl_display);
	globox->wl_registry = wl_display_get_registry(globox->wl_display);
	wl_registry_add_listener(
		globox->wl_registry,
		&(globox->wl_registry_listener),
		globox);
	wl_display_dispatch(globox->wl_display);
	wl_display_roundtrip(globox->wl_display);

#ifdef GLOBOX_RENDER_OGL
	globox->wl_egl_display = eglGetDisplay((EGLNativeDisplayType) globox->wl_display);

	if (globox->wl_egl_display == EGL_NO_DISPLAY)
	{
		return false;
	}

	EGLint egl_major;
	EGLint egl_minor;
	EGLBoolean egl_ret;

	egl_ret = eglInitialize(globox->wl_egl_display, &egl_major, &egl_minor);

	if (egl_ret != EGL_TRUE)
	{
		return false;
	}

	egl_ret = eglBindAPI(EGL_OPENGL_API);

	if (egl_ret != EGL_TRUE)
	{
		return false;
	}

	EGLint egl_cfg;
	static EGLint egl_cfg_attr[] =
	{
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE,
	};

	egl_ret = eglChooseConfig(
		globox->wl_egl_display,
		egl_cfg_attr,
		&(globox->wl_egl_config),
		1,
		&egl_cfg);

	if (egl_ret != EGL_TRUE)
	{
		return false;
	}

	static EGLint egl_ctx_attr[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE,
	};

	globox->wl_egl_context =
		eglCreateContext(
			globox->wl_egl_display,
			globox->wl_egl_config,
			EGL_NO_CONTEXT,
			egl_ctx_attr);

	if (globox->wl_egl_context == EGL_NO_CONTEXT)
	{
		return false;
	}
#endif

	// surface init
	err = surface_init(globox);

	// post-surface creation init
	globox->title = NULL;
	globox_set_title(globox, title);
	globox_set_state(globox, state);

	if (globox->frame_event)
	{
		globox->fd_frame = eventfd(0, 0);
	}

	globox_epoll_init(globox);

	return err;
}

inline void globox_close(struct globox* globox)
{
	int size = globox->width * globox->height * 4;

	wl_shm_pool_destroy(globox->wl_pool);
	close(globox->wl_buffer_fd);
	munmap(globox->argb, size);
	wl_buffer_destroy(globox->wl_buffer);

	if (globox->frame_event)
	{
		close(globox->fd_frame);
	}

	free(globox->title);
	free(globox->wl_shm);
	free(globox->wl_compositor);
	free(globox->xdg_wm_base);
	free(globox->wl_output);

	wl_surface_destroy(globox->wl_surface);
	xdg_surface_destroy(globox->xdg_surface);
	xdg_toplevel_destroy(globox->xdg_toplevel);
	wl_callback_destroy(globox->wl_frame_callback);

	wl_registry_destroy(globox->wl_registry);
	wl_display_disconnect(globox->wl_display);
}

inline bool globox_handle_events(struct globox* globox)
{
	return false;
}

inline bool globox_shrink(struct globox* globox)
{
#ifndef GLOBOX_RENDER_OGL
	wl_shm_pool_destroy(globox->wl_pool);
	close(globox->wl_buffer_fd);
	munmap(globox->argb, globox->buf_width * globox->buf_height * 4);

	globox->buf_width = globox->width;
	globox->buf_height = globox->height;

	allocate_buffer(globox);
#endif

	return true;
}

inline void globox_copy(
	struct globox* globox,
	int32_t x,
	int32_t y,
	uint32_t width,
	uint32_t height)
{
#ifdef GLOBOX_RENDER_OGL
	eglSwapBuffers(globox->wl_egl_display, globox->wl_egl_surface);
#else
	wl_surface_damage_buffer(globox->wl_surface, x, y, width, height);
#endif

	globox_commit(globox);

	globox->redraw = false;
}

inline void globox_commit(
	struct globox* globox)
{
	wl_surface_commit(globox->wl_surface);
}

inline void globox_prepoll(
	struct globox* globox)
{
	wl_display_dispatch(globox->wl_display);
}

bool globox_wait_events(struct globox* globox)
{
	epoll_wait(
		globox->epoll_fd,
		globox->epoll_list,
		GLOBOX_MAX_EVENTS,
		-1);

	return true;
}

bool globox_poll_events(struct globox* globox)
{
	// not needed
	return true;
}

inline void globox_set_icon(struct globox* globox, uint32_t* pixmap, uint32_t len)
{
	// wayland does not support server-side icons, but we could use one client-side
	globox->wl_icon = pixmap;
	globox->wl_icon_len = len;
}

inline void globox_set_title(struct globox* globox, const char* title)
{
	if (globox->title != NULL)
	{
		free(globox->title);
	}

	globox->title = strdup(title);

    xdg_toplevel_set_title(globox->xdg_toplevel, title);
}

inline void globox_set_state(struct globox* globox, enum globox_state state)
{
	switch (state)
	{
		case GLOBOX_STATE_REGULAR:
		{
			xdg_toplevel_unset_maximized(globox->xdg_toplevel);
			xdg_toplevel_unset_fullscreen(globox->xdg_toplevel);

			// ladies and gentlemen
			if (globox->state == GLOBOX_STATE_MINIMIZED)
			{
				char* title = globox->title;

#ifndef GLOBOX_RENDER_OGL
				int size = globox->width * globox->height * 4;
				wl_shm_pool_destroy(globox->wl_pool);
				close(globox->wl_buffer_fd);
				munmap(globox->argb, size);
				wl_buffer_destroy(globox->wl_buffer);
#endif

				wl_surface_destroy(globox->wl_surface);
				xdg_surface_destroy(globox->xdg_surface);
				xdg_toplevel_destroy(globox->xdg_toplevel);
				wl_callback_destroy(globox->wl_frame_callback);

				surface_init(globox);

				globox->title = NULL;
				globox_set_title(globox, title);
				free(title);
			}

			break;
		}
		case GLOBOX_STATE_MAXIMIZED:
		{
			xdg_toplevel_unset_fullscreen(globox->xdg_toplevel);
			xdg_toplevel_set_maximized(globox->xdg_toplevel);

			break;
		}
		case GLOBOX_STATE_MINIMIZED:
		{
			xdg_toplevel_unset_maximized(globox->xdg_toplevel);
			xdg_toplevel_unset_fullscreen(globox->xdg_toplevel);
			xdg_toplevel_set_minimized(globox->xdg_toplevel);

			break;
		}
		case GLOBOX_STATE_FULLSCREEN:
		{
			xdg_toplevel_unset_maximized(globox->xdg_toplevel);
			xdg_toplevel_set_fullscreen(globox->xdg_toplevel, NULL);

			break;
		}
	}

	globox->state = state;
}

inline char* globox_get_title(struct globox* globox)
{
	return globox->title;
}

inline enum globox_state globox_get_state(struct globox* globox)
{
	return globox->state;
}

inline void globox_get_size(struct globox* globox, uint32_t* width, uint32_t* height)
{
	*width = globox->width;
	*height = globox->height;
}

#endif
