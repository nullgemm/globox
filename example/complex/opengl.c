#include "globuf.h"
#include "cursoryx.h"
#include "dpishit.h"
#include "willis.h"

#if defined(GLOBUF_EXAMPLE_X11)
#if defined(GLOBUF_EXAMPLE_GLX)
	#include "globuf_x11_glx.h"
#elif defined(GLOBUF_EXAMPLE_EGL)
	#include "globuf_x11_egl.h"
#endif
	#include "cursoryx_x11.h"
	#include "dpishit_x11.h"
	#include "willis_x11.h"
#elif defined(GLOBUF_EXAMPLE_APPKIT)
	#include "globuf_appkit_egl.h"
	#include "cursoryx_appkit.h"
	#include "dpishit_appkit.h"
	#include "willis_appkit.h"
#elif defined(GLOBUF_EXAMPLE_WIN)
	#include "globuf_win_wgl.h"
	#include "cursoryx_win.h"
	#include "dpishit_win.h"
	#include "willis_win.h"
#elif defined(GLOBUF_EXAMPLE_WAYLAND)
	#include "globuf_wayland_egl.h"
	#include "cursoryx_wayland.h"
	#include "dpishit_wayland.h"
	#include "willis_wayland.h"
#endif

#ifdef GLOBUF_EXAMPLE_APPKIT
#define main real_main
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(GLOBUF_EXAMPLE_GLX)
	#include <GL/glx.h>
	#include <GLES2/gl2.h>
#elif defined(GLOBUF_EXAMPLE_EGL)
	#include <EGL/egl.h>
	#include <GLES2/gl2.h>
#elif defined(GLOBUF_EXAMPLE_WGL)
	#include <GL/gl.h>
	#undef WGL_WGLEXT_PROTOTYPES
	#include <GL/wglext.h>
	#define GL_GLES_PROTOTYPES 0
	#include <GLES2/gl2.h>
#endif

extern uint8_t iconpix[];
extern int iconpix_size;

extern uint8_t cursorpix[];
extern int cursorpix_size;

#if defined(GLOBUF_EXAMPLE_APPKIT)
extern uint8_t square_frag_gles2[];
extern int square_frag_gles2_size;

extern uint8_t square_vert_gles2[];
extern int square_vert_gles2_size;
#else
extern uint8_t square_frag_gl1[];
extern int square_frag_gl1_size;

extern uint8_t square_vert_gl1[];
extern int square_vert_gl1_size;
#endif

#define VERTEX_ATTR_POSITION 0

char* feature_names[GLOBUF_FEATURE_COUNT] =
{
	[GLOBUF_FEATURE_INTERACTION] = "interaction",
	[GLOBUF_FEATURE_STATE] = "state",
	[GLOBUF_FEATURE_TITLE] = "title",
	[GLOBUF_FEATURE_ICON] = "icon",
	[GLOBUF_FEATURE_SIZE] = "size",
	[GLOBUF_FEATURE_POS] = "pos",
	[GLOBUF_FEATURE_FRAME] = "frame",
	[GLOBUF_FEATURE_BACKGROUND] = "background",
	[GLOBUF_FEATURE_VSYNC] = "vsync",
};

#if defined(GLOBUF_EXAMPLE_GLX)
int glx_config_attrib[] =
{
	GLX_DOUBLEBUFFER, True,
	GLX_RENDER_TYPE, GLX_RGBA_BIT,
	GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,

	GLX_RED_SIZE, 8,
	GLX_GREEN_SIZE, 8,
	GLX_BLUE_SIZE, 8,
	GLX_ALPHA_SIZE, 8,
	GLX_DEPTH_SIZE, 24,
	None,
};
#elif defined(GLOBUF_EXAMPLE_EGL)
EGLint egl_config_attrib[] =
{
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
#if defined (GLOBUF_EXAMPLE_APPKIT)
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#else
	EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
#endif
	EGL_NONE,
};
#elif defined(GLOBUF_EXAMPLE_WGL)
int wgl_config_attrib[] =
{
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	WGL_COLOR_BITS_ARB, 32,
	WGL_DEPTH_BITS_ARB, 16,
	WGL_STENCIL_BITS_ARB, 0,
	0,
};

// opengl32.dll only supports OpenGL 1
// so we have to load these functions
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;

PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

PFNGLATTACHSHADERPROC glAttachShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLDELETESHADERPROC glDeleteShader;

PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

static void load_wgl_functions()
{
	glCreateShader =
		(PFNGLCREATESHADERPROC)
			wglGetProcAddress("glCreateShader");
	glShaderSource =
		(PFNGLSHADERSOURCEPROC)
			wglGetProcAddress("glShaderSource");
	glCompileShader =
		(PFNGLCOMPILESHADERPROC)
			wglGetProcAddress("glCompileShader");

	glGetShaderiv =
		(PFNGLGETSHADERIVPROC)
			wglGetProcAddress("glGetShaderiv");
	glGetShaderInfoLog =
		(PFNGLGETSHADERINFOLOGPROC)
			wglGetProcAddress("glGetShaderInfoLog");

	glAttachShader =
		(PFNGLATTACHSHADERPROC)
			wglGetProcAddress("glAttachShader");
	glCreateProgram =
		(PFNGLCREATEPROGRAMPROC)
			wglGetProcAddress("glCreateProgram");
	glLinkProgram =
		(PFNGLLINKPROGRAMPROC)
			wglGetProcAddress("glLinkProgram");
	glDeleteShader =
		(PFNGLDELETESHADERPROC)
			wglGetProcAddress("glDeleteShader");

	glGetProgramiv =
		(PFNGLGETPROGRAMIVPROC)
			wglGetProcAddress("glGetProgramiv");
	glGetProgramInfoLog =
		(PFNGLGETPROGRAMINFOLOGPROC)
			wglGetProcAddress("glGetProgramInfoLog");

	glUseProgram =
		(PFNGLUSEPROGRAMPROC)
			wglGetProcAddress("glUseProgram");
	glEnableVertexAttribArray =
		(PFNGLENABLEVERTEXATTRIBARRAYPROC)
			wglGetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer =
		(PFNGLVERTEXATTRIBPOINTERPROC)
			wglGetProcAddress("glVertexAttribPointer");
}
#endif

struct event_callback_data
{
	struct globuf* globuf;

	struct cursoryx* cursoryx;
	struct dpishit* dpishit;
	struct willis* willis;

	struct globuf_feature_interaction action;
	struct cursoryx_custom* mouse_custom[4];
	size_t mouse_custom_active;
	bool mouse_grabbed;
	bool shaders;
};

struct globuf_render_data
{
	struct globuf* globuf;
	bool shaders;
};

static void compile_shaders()
{
	GLint out;

	// compile OpenGL or glES shaders
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
#if defined(GLOBUF_EXAMPLE_APPKIT)
	const char * const square_vert_gl = (char*) &square_vert_gles2;
	GLint square_vert_size_gl = square_vert_gles2_size;
#else
	const char * const square_vert_gl = (char*) &square_vert_gl1;
	GLint square_vert_size_gl = square_vert_gl1_size;
#endif
	glShaderSource(vertex_shader, 1, &square_vert_gl, &square_vert_size_gl);
	fprintf(stderr, "compiling vertex shader:\n%.*s\n", square_vert_size_gl, square_vert_gl);
	glCompileShader(vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
#if defined(GLOBUF_EXAMPLE_APPKIT)
	const char * const square_frag_gl = (char*) &square_frag_gles2;
	GLint square_frag_size_gl = square_frag_gles2_size;
#else
	const char * const square_frag_gl = (char*) &square_frag_gl1;
	GLint square_frag_size_gl = square_frag_gl1_size;
#endif
	glShaderSource(fragment_shader, 1, &square_frag_gl, &square_frag_size_gl);
	fprintf(stderr, "compiling fragment shader:\n%.*s\n", square_frag_size_gl, square_frag_gl);
	glCompileShader(fragment_shader);

	// check for compilation errors
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &out);

	if (out != GL_TRUE)
	{
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &out);
		GLint len;
		GLchar* log = malloc(out);
		glGetShaderInfoLog(vertex_shader, out, &len, log);
		fprintf(stderr, "vertex shader compilation error:\n%.*s\n", len, log);
	}

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &out);

	if (out != GL_TRUE)
	{
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &out);
		GLint len;
		GLchar* log = malloc(out);
		glGetShaderInfoLog(fragment_shader, out, &len, log);
		fprintf(stderr, "fragment shader compilation error:\n%.*s\n", len, log);
	}

	// link shader program
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glLinkProgram(shader_program);

	// check for compilation errors
	glGetProgramiv(shader_program, GL_LINK_STATUS, &out);

	if (out != GL_TRUE)
	{
		GLint len;
		GLchar* log = malloc(out);
		glGetProgramInfoLog(shader_program, out, &len, log);
		fprintf(stderr, "shader program link error:\n%.*s\n", len, log);
	}

	glUseProgram(shader_program);
}

static void event_callback(void* data, void* event)
{
	struct event_callback_data* event_callback_data = data;

	struct globuf* globuf = event_callback_data->globuf;
	struct globuf_error_info error = {0};

	// print some debug info on internal events
	enum globuf_event abstract =
		globuf_handle_events(globuf, event, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		return;
	}

	switch (abstract)
	{
		case GLOBUF_EVENT_INVALID:
		{
			// shouldn't be possible since we handle the error
			fprintf(stderr, "received invalid event\n");
			break;
		}
		case GLOBUF_EVENT_UNKNOWN:
		{
#ifdef GLOBUF_EXAMPLE_LOG_ALL
			fprintf(stderr, "received unknown event\n");
#endif
			break;
		}
		case GLOBUF_EVENT_RESTORED:
		{
			fprintf(stderr, "received `restored` event\n");
			break;
		}
		case GLOBUF_EVENT_MINIMIZED:
		{
			fprintf(stderr, "received `minimized` event\n");
			break;
		}
		case GLOBUF_EVENT_MAXIMIZED:
		{
			fprintf(stderr, "received `maximized` event\n");
			break;
		}
		case GLOBUF_EVENT_FULLSCREEN:
		{
			fprintf(stderr, "received `fullscreen` event\n");
			break;
		}
		case GLOBUF_EVENT_CLOSED:
		{
			fprintf(stderr, "received `closed` event\n");
			break;
		}
		case GLOBUF_EVENT_MOVED_RESIZED:
		{
			fprintf(stderr, "received `moved` event\n");
			break;
		}
		case GLOBUF_EVENT_DAMAGED:
		{
#ifdef GLOBUF_EXAMPLE_LOG_ALL
			struct globuf_rect rect = globuf_get_expose(globuf, &error);

			if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
			{
				globuf_error_log(globuf, &error);
				break;
			}

			fprintf(
				stderr,
				"received `content damaged` event:\n"
				"\tx: %d px\n"
				"\ty: %d px\n"
				"\twidth: %d px\n"
				"\theight: %d px\n",
				rect.x,
				rect.y,
				rect.width,
				rect.height);
#endif

			break;
		}
	}

#ifdef GLOBUF_EXAMPLE_LOG_ALL
	// handle dpi changes
	struct dpishit* dpishit = event_callback_data->dpishit;
	struct dpishit_error_info error_dpishit = {0};
	struct dpishit_display_info display_info = {0};
	bool dpishit_valid = false;

	if (dpishit != NULL)
	{
		dpishit_valid =
			dpishit_handle_event(
				dpishit,
				event,
				&display_info,
				&error_dpishit);

		if (dpishit_error_get_code(&error_dpishit) != DPISHIT_ERROR_OK)
		{
			dpishit_error_log(dpishit, &error_dpishit);
			return;
		}

		if (dpishit_valid == true)
		{
			fprintf(
				stderr,
				"dpishit returned display info:\n"
				"\twidth: %u px\n"
				"\theight: %u px\n"
				"\twidth: %u mm\n"
				"\theight: %u mm\n",
				display_info.px_width,
				display_info.px_height,
				display_info.mm_width,
				display_info.mm_height);

			if (display_info.dpi_logic_valid == true)
			{
				fprintf(
					stderr,
					"\tlogic dpi: %lf dpi\n",
					display_info.dpi_logic);
			}

			if (display_info.dpi_scale_valid == true)
			{
				fprintf(
					stderr,
					"\tscale: %lf\n",
					display_info.dpi_scale);
			}
		}
	}
#endif

	// handle cursor changes
	struct cursoryx* cursoryx = event_callback_data->cursoryx;
	struct cursoryx_error_info error_cursoryx = {0};

	// handle dpi changes
	struct willis* willis = event_callback_data->willis;
	struct willis_error_info error_willis = {0};
	struct willis_event_info event_info = {0};

	if (willis != NULL)
	{
		willis_handle_event(
			willis,
			event,
			&event_info,
			&error_willis);

		if (willis_error_get_code(&error_willis) != WILLIS_ERROR_OK)
		{
			willis_error_log(willis, &error_willis);
			return;
		}

		// handle keys
	if (event_info.event_state != WILLIS_STATE_PRESS)
	{
		struct globuf_feature_state state;
		bool sizemove = false;

		switch (event_info.event_code)
		{
			case WILLIS_KEY_G:
			{
				if (event_callback_data->mouse_grabbed == false)
				{
					willis_mouse_grab(willis, &error_willis);
				}
				else
				{
					willis_mouse_ungrab(willis, &error_willis);
				}

				if (willis_error_get_code(&error_willis) != WILLIS_ERROR_OK)
				{
					willis_error_log(willis, &error_willis);
					return;
				}

				event_callback_data->mouse_grabbed =
					!event_callback_data->mouse_grabbed;

				break;
			}
			case WILLIS_KEY_M:
			{
				if (cursoryx == NULL)
				{
					break;
				}

				size_t cur = event_callback_data->mouse_custom_active;

				++cur;

				if (cur > 3)
				{
					cur = 0;
				}

				cursoryx_custom_set(
					cursoryx,
					event_callback_data->mouse_custom[cur],
					&error_cursoryx);

				if (cursoryx_error_get_code(&error_cursoryx) != CURSORYX_ERROR_OK)
				{
					cursoryx_error_log(cursoryx, &error_cursoryx);
					return;
				}

				event_callback_data->mouse_custom_active = cur;

				break;
			}
			case WILLIS_KEY_N:
			{
				// set a default regular cursor for our window (wait/busy cursor)
				cursoryx_set(cursoryx, CURSORYX_BUSY, &error_cursoryx);

				if (cursoryx_error_get_code(&error_cursoryx) != CURSORYX_ERROR_OK)
				{
					cursoryx_error_log(cursoryx, &error_cursoryx);
					return;
				}

				break;
			}
			case WILLIS_KEY_W:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_N;
				sizemove = true;
				break;
			}
			case WILLIS_KEY_Q:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_NW;
				sizemove = true;
				break;
			}
			case WILLIS_KEY_A:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_W;
				sizemove = true;
				break;
			}
			case WILLIS_KEY_Z:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_SW;
				sizemove = true;
				break;
			}
			case WILLIS_KEY_X:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_S;
				sizemove = true;
				break;
			}
			case WILLIS_KEY_C:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_SE;
				sizemove = true;
				break;
			}
			case WILLIS_KEY_D:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_E;
				sizemove = true;
				break;
			}
			case WILLIS_KEY_E:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_NE;
				sizemove = true;
				break;
			}
			case WILLIS_KEY_S:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_MOVE;
				sizemove = true;
				break;
			}
			case WILLIS_MOUSE_CLICK_LEFT:
			{
				event_callback_data->action.action = GLOBUF_INTERACTION_STOP;
				break;
			}
			case WILLIS_KEY_1:
			{
				state.state = GLOBUF_STATE_REGULAR;
				globuf_feature_set_state(globuf, &state, &error);
				break;
			}
			case WILLIS_KEY_2:
			{
				state.state = GLOBUF_STATE_MINIMIZED;
				globuf_feature_set_state(globuf, &state, &error);
				break;
			}
			case WILLIS_KEY_3:
			{
				state.state = GLOBUF_STATE_MAXIMIZED;
				globuf_feature_set_state(globuf, &state, &error);
				break;
			}
			case WILLIS_KEY_4:
			{
				state.state = GLOBUF_STATE_FULLSCREEN;
				globuf_feature_set_state(globuf, &state, &error);
				break;
			}
			default:
			{
				break;
			}
		}

		if (sizemove == true)
		{
			globuf_feature_set_interaction(globuf, &(event_callback_data->action), &error);
		}
	}

		// print debug info
		if (event_info.event_code != WILLIS_NONE)
		{
			fprintf(
				stderr,
				"willis returned event info:\n"
				"\tcode: %s\n"
				"\tstate: %s\n",
				willis_get_event_code_name(willis, event_info.event_code, &error_willis),
				willis_get_event_state_name(willis, event_info.event_state, &error_willis));
		}

		if (event_info.event_code == WILLIS_MOUSE_MOTION)
		{
			if (event_callback_data->mouse_grabbed == false)
			{
				fprintf(
					stderr,
					"\tmouse x: %d\n"
					"\tmouse y: %d\n",
					event_info.mouse_x,
					event_info.mouse_y);
			}
			else
			{
				fprintf(
					stderr,
					"\tdiff x: %d\n"
					"\tdiff y: %d\n",
					(int) (event_info.diff_x >> 32),
					(int) (event_info.diff_y >> 32));
			}
		}
		else if (event_info.utf8_size > 0)
		{
			fprintf(
				stderr,
				"\ttext: %.*s\n",
				(int) event_info.utf8_size,
				event_info.utf8_string);

			free(event_info.utf8_string);
		}
	}
}

static void render_callback(void* data)
{
	// render our trademark square as a simple example, updating the whole
	// buffer each time without taking surface damage events into account
	struct globuf_render_data* render_data = data;
	struct globuf* globuf = render_data->globuf;
	struct globuf_error_info error = {0};

	int width = globuf_get_width(globuf, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		return;
	}

	int height = globuf_get_height(globuf, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		return;
	}

	if ((width == 0) || (height == 0))
	{
		// skip rendering if the window area is 0
		return;
	}

	// we can make OpenGL 1 calls without any loader
	if (render_data->shaders == true)
	{
#ifdef GLOBUF_EXAMPLE_WGL
		load_wgl_functions();
#endif
		compile_shaders();
		render_data->shaders = false;
	}

	GLint viewport_rect[4];

	glGetIntegerv(GL_VIEWPORT, viewport_rect);

	if ((viewport_rect[2] != width) || (viewport_rect[3] != height))
	{
		glViewport(0, 0, width, height);
	}

	glClearColor(0.2f, 0.4f, 0.9f, (0x22 / 255.0f));
	glClear(GL_COLOR_BUFFER_BIT);

	GLfloat vertices[] =
	{
		-100.0f / width, +100.0f / height, 1.0f,
		-100.0f / width, -100.0f / height, 1.0f,
		+100.0f / width, -100.0f / height, 1.0f,
		+100.0f / width, +100.0f / height, 1.0f,
	};

	glEnableVertexAttribArray(VERTEX_ATTR_POSITION);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		vertices);

	glDrawArrays(
		GL_TRIANGLE_FAN,
		0,
		4);

	globuf_update_content(globuf, NULL, &error);

	// reducing latency when resizing
	glFinish();

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		return;
	}
}

static void config_callback(struct globuf_config_reply* replies, size_t count, void* data)
{
	fprintf(stderr, "window configured succesfully, printing information:\n");

	struct globuf* context = data;
	const char* message = NULL;
	size_t feature;

	for (size_t i = 0; i < count; ++i)
	{
		feature = replies[i].feature;

		if (feature < count)
		{
			if (replies[i].error.code == GLOBUF_ERROR_OK)
			{
				message = "success";
			}
			else
			{
				message = globuf_error_get_msg(context, &replies[i].error);
			}

			fprintf(stderr, "\t%s: %s\n", feature_names[feature], message);
		}
	}
}

int main(int argc, char** argv)
{
	struct globuf_error_info error = {0};
	struct globuf_error_info error_early = {0};
	printf("starting the complex globuf example\n");

	// prepare function pointers
	struct globuf_config_backend config = {0};

#if defined(GLOBUF_EXAMPLE_X11)
#if defined(GLOBUF_EXAMPLE_GLX)
	globuf_prepare_init_x11_glx(&config, &error_early);
#elif defined(GLOBUF_EXAMPLE_EGL)
	globuf_prepare_init_x11_egl(&config, &error_early);
#endif
#elif defined(GLOBUF_EXAMPLE_APPKIT)
	globuf_prepare_init_appkit_egl(&config, &error_early);
#elif defined(GLOBUF_EXAMPLE_WIN)
	globuf_prepare_init_win_wgl(&config, &error_early);

	printf(
		"\nEncoding notice: this example outputs utf-8 encoded text as a"
		" demonstration of the composition capabilities of willis, the"
		" independent lib we use to handle the keyboard and mouse.\n"
		"Since Windows only supports utf-8 console output using wchar_t"
		" (which we do not use) non-ANSI text will not display properly"
		" on this platform, but the text in RAM really is valid.\n\n");
#elif defined(GLOBUF_EXAMPLE_WAYLAND)
	globuf_prepare_init_wayland_egl(&config, &error_early);
#endif

	// set function pointers and perform basic init
	struct globuf* globuf = globuf_init(&config, &error);

	// Unless the context allocation failed it is always possible to access
	// error messages (even when the context initialization failed) so we can
	// always handle the backend initialization error first.

	// context allocation failed
	if (globuf == NULL)
	{
		fprintf(stderr, "could not allocate the main globuf context\n");
		return 1;
	}

	// Backend initialization failed. Since it happens before globuf
	// initialization and errors are accessible even if it fails, we can handle
	// the errors in the right order regardless.
	if (globuf_error_get_code(&error_early) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error_early);
		globuf_clean(globuf, &error);
		return 1;
	}

	// The globuf initialization had failed, make it known now if the backend
	// initialization that happened before went fine.
	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// set OpenGL configuration attributes
	struct globuf_config_opengl config_opengl =
	{
		.major_version = 2,
		.minor_version = 0,
#if defined(GLOBUF_EXAMPLE_GLX)
		.attributes = glx_config_attrib,
#elif defined(GLOBUF_EXAMPLE_EGL)
		.attributes = egl_config_attrib,
#elif defined(GLOBUF_EXAMPLE_WGL)
		.attributes = wgl_config_attrib,
#endif
	};

	globuf_init_opengl(globuf, &config_opengl, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// get available features
	struct globuf_config_features* feature_list =
		globuf_init_features(globuf, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// initialize features when creating the window
	struct globuf_feature_state state =
	{
		.state = GLOBUF_STATE_REGULAR,
	};

	struct globuf_feature_title title =
	{
		.title = "globuf",
	};

	struct globuf_feature_icon icon =
	{
		// acceptable implementation-defined behavior
		// since it's also the implementation that
		// allows us to bundle resources like so
		.pixmap = (uint32_t*) iconpix,
		.len = 2 + (16 * 16) + 2 + (32 * 32) + 2 + (64 * 64),
	};

	struct globuf_feature_size size =
	{
		.width = 500,
		.height = 500,
	};

	struct globuf_feature_pos pos =
	{
		.x = 250,
		.y = 250,
	};

	struct globuf_feature_frame frame =
	{
		.frame = true,
	};

	struct globuf_feature_background background =
	{
		.background = GLOBUF_BACKGROUND_BLURRED,
	};

	struct globuf_feature_vsync vsync =
	{
		.vsync = true,
	};

	// configure the feature and print a list
	printf("received a list of available features:\n");

	struct globuf_config_request configs[GLOBUF_FEATURE_COUNT] = {0};
	size_t feature_added = 0;
	size_t i = 0;

	while (i < feature_list->count)
	{
		enum globuf_feature feature_id = feature_list->list[i];
		printf("\t%s\n", feature_names[feature_id]);
		++i;

		switch (feature_id)
		{
			case GLOBUF_FEATURE_STATE:
			{
				configs[feature_added].config = &state;
				break;
			}
			case GLOBUF_FEATURE_TITLE:
			{
				configs[feature_added].config = &title;
				break;
			}
			case GLOBUF_FEATURE_ICON:
			{
				configs[feature_added].config = &icon;
				break;
			}
			case GLOBUF_FEATURE_SIZE:
			{
				configs[feature_added].config = &size;
				break;
			}
			case GLOBUF_FEATURE_POS:
			{
				configs[feature_added].config = &pos;
				break;
			}
			case GLOBUF_FEATURE_FRAME:
			{
				configs[feature_added].config = &frame;
				break;
			}
			case GLOBUF_FEATURE_BACKGROUND:
			{
				configs[feature_added].config = &background;
				break;
			}
			case GLOBUF_FEATURE_VSYNC:
			{
				configs[feature_added].config = &vsync;
				break;
			}
			default:
			{
				continue;
			}
		}

		configs[feature_added].feature = feature_id;
		++feature_added;
	}

	free(feature_list->list);
	free(feature_list);

	struct event_callback_data event_callback_data =
	{
		.globuf = globuf,
		.dpishit = NULL,
		.willis = NULL,
		.cursoryx = NULL,
		.action = { .action = GLOBUF_INTERACTION_STOP, },
		.mouse_custom = {0},
		.mouse_custom_active = 4,
		.mouse_grabbed = false,
	};

	// register an event handler to track the window's state
	struct globuf_config_events events =
	{
		.data = &event_callback_data,
		.handler = event_callback,
	};

	struct globuf_error_info error_events = {0};
	globuf_init_events(globuf, &events, &error_events);

	if (globuf_error_get_code(&error_events) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error_events);
		globuf_clean(globuf, &error);
		return 1;
	}

	// register a render callback
	struct globuf_render_data render_data =
	{
		.globuf = globuf,
		.shaders = true,
	};

	struct globuf_config_render render =
	{
		.data = &render_data,
		.callback = render_callback,
	};

	struct globuf_error_info error_render = {0};
	globuf_init_render(globuf, &render, &error_render);

	if (globuf_error_get_code(&error_render) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error_render);
		globuf_clean(globuf, &error);
		return 1;
	}

	// create the window
	globuf_window_create(globuf, configs, feature_added, config_callback, globuf, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// for instance, we can set the mouse cursor
	struct cursoryx_error_info error_cursor = {0};
	struct cursoryx_config_backend config_cursor = {0};

#if defined(GLOBUF_EXAMPLE_X11)
	cursoryx_prepare_init_x11(&config_cursor);

	struct cursoryx_x11_data cursoryx_data =
	{
		.conn = globuf_get_x11_conn(globuf),
		.window = globuf_get_x11_window(globuf),
		.screen = globuf_get_x11_screen(globuf),
	};
#elif defined(GLOBUF_EXAMPLE_APPKIT)
	cursoryx_prepare_init_appkit(&config_cursor);

	struct cursoryx_appkit_data cursoryx_data =
	{
		.data = NULL,
	};
#elif defined(GLOBUF_EXAMPLE_WIN)
	cursoryx_prepare_init_win(&config_cursor);

	struct cursoryx_win_data cursoryx_data =
	{
		.data = NULL,
	};
#elif defined(GLOBUF_EXAMPLE_WAYLAND)
	cursoryx_prepare_init_wayland(&config_cursor);

	struct cursoryx_wayland_data cursoryx_data =
	{
		.add_capabilities_handler = globuf_add_wayland_capabilities_handler,
		.add_capabilities_handler_data = globuf,
		.add_registry_handler = globuf_add_wayland_registry_handler,
		.add_registry_handler_data = globuf,
	};
#endif

	struct cursoryx* cursoryx = cursoryx_init(&config_cursor, &error_cursor);

	if (cursoryx == NULL)
	{
		fprintf(stderr, "could not allocate the main cursoryx context\n");
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	if (cursoryx_error_get_code(&error_cursor) != CURSORYX_ERROR_OK)
	{
		cursoryx_error_log(cursoryx, &error_cursor);
		cursoryx_clean(cursoryx, &error_cursor);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	cursoryx_start(cursoryx, &cursoryx_data, &error_cursor);

	if (cursoryx_error_get_code(&error_cursor) != CURSORYX_ERROR_OK)
	{
		cursoryx_error_log(cursoryx, &error_cursor);
		cursoryx_clean(cursoryx, &error_cursor);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// prepare custom cursors
	event_callback_data.cursoryx = cursoryx;

	struct cursoryx_custom_config cursor_config[4] =
	{
		{
			.image = (uint32_t*) (cursorpix + 8 + (16*22*4)*0),
			.width = 16,
			.height = 22,
			.x = 7,
			.y = 13,
		},
		{
			.image = (uint32_t*) (cursorpix + 16 + (16*22*4)*1),
			.width = 16,
			.height = 22,
			.x = 7,
			.y = 13,
		},
		{
			.image = (uint32_t*) (cursorpix + 24 + (16*22*4)*2),
			.width = 16,
			.height = 22,
			.x = 7,
			.y = 13,
		},
		{
			.image = (uint32_t*) (cursorpix + 32 + (16*22*4)*3),
			.width = 16,
			.height = 22,
			.x = 7,
			.y = 13,
		},
	};

	// init willis
	struct willis_error_info error_input = {0};
	struct willis_config_backend config_input = {0};

#if defined(GLOBUF_EXAMPLE_X11)
	willis_prepare_init_x11(&config_input);

	struct willis_x11_data willis_data =
	{
		.conn = globuf_get_x11_conn(globuf),
		.window = globuf_get_x11_window(globuf),
		.root = globuf_get_x11_root(globuf),
	};
#elif defined(GLOBUF_EXAMPLE_APPKIT)
	willis_prepare_init_appkit(&config_input);

	struct willis_appkit_data willis_data =
	{
		.data = NULL,
	};
#elif defined(GLOBUF_EXAMPLE_WIN)
	willis_prepare_init_win(&config_input);

	struct willis_win_data willis_data =
	{
		.data = NULL,
	};
#elif defined(GLOBUF_EXAMPLE_WAYLAND)
	willis_prepare_init_wayland(&config_input);

	struct willis_wayland_data willis_data =
	{
		.add_capabilities_handler = globuf_add_wayland_capabilities_handler,
		.add_capabilities_handler_data = globuf,
		.add_registry_handler = globuf_add_wayland_registry_handler,
		.add_registry_handler_data = globuf,
		.event_callback = event_callback,
		.event_callback_data = &event_callback_data,
	};
#endif

	struct willis* willis = willis_init(&config_input, &error_input);

	if (willis == NULL)
	{
		fprintf(stderr, "could not allocate the main willis context\n");
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	event_callback_data.willis = willis;

	// start willis
	if (willis_error_get_code(&error_input) != WILLIS_ERROR_OK)
	{
		willis_error_log(willis, &error_input);
		willis_clean(willis, &error_input);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	willis_start(willis, &willis_data, &error_input);

	if (willis_error_get_code(&error_input) != WILLIS_ERROR_OK)
	{
		willis_error_log(willis, &error_input);
		willis_clean(willis, &error_input);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// init dpishit
	struct dpishit_error_info error_display = {0};
	struct dpishit_config_backend config_display = {0};

#if defined(GLOBUF_EXAMPLE_X11)
	dpishit_prepare_init_x11(&config_display);

	struct dpishit_x11_data dpishit_data =
	{
		.conn = globuf_get_x11_conn(globuf),
		.window = globuf_get_x11_window(globuf),
		.root = globuf_get_x11_root(globuf),
	};
#elif defined(GLOBUF_EXAMPLE_APPKIT)
	dpishit_prepare_init_appkit(&config_display);

	struct dpishit_appkit_data dpishit_data =
	{
		.data = NULL,
	};
#elif defined(GLOBUF_EXAMPLE_WIN)
	dpishit_prepare_init_win(&config_display);

	struct dpishit_win_data dpishit_data =
	{
		.data = NULL,
	};
#elif defined(GLOBUF_EXAMPLE_WAYLAND)
	dpishit_prepare_init_wayland(&config_display);

	struct dpishit_wayland_data dpishit_data =
	{
		.add_registry_handler = globuf_add_wayland_registry_handler,
		.add_registry_handler_data = globuf,
		.add_registry_remover = globuf_add_wayland_registry_remover,
		.add_registry_remover_data = globuf,
		.event_callback = event_callback,
		.event_callback_data = &event_callback_data,
	};
#endif

	struct dpishit* dpishit = dpishit_init(&config_display, &error_display);

	if (dpishit == NULL)
	{
		fprintf(stderr, "could not allocate the main dpishit context\n");
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	event_callback_data.dpishit = dpishit;

	// start dpishit
	if (dpishit_error_get_code(&error_display) != DPISHIT_ERROR_OK)
	{
		dpishit_error_log(dpishit, &error_display);
		dpishit_clean(dpishit, &error_display);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	dpishit_start(dpishit, &dpishit_data, &error_display);

	if (dpishit_error_get_code(&error_display) != DPISHIT_ERROR_OK)
	{
		dpishit_error_log(dpishit, &error_display);
		dpishit_clean(dpishit, &error_display);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// check window
	globuf_window_confirm(globuf, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	for (size_t i = 0; i < 4; ++i)
	{
		event_callback_data.mouse_custom[i] =
			cursoryx_custom_create(cursoryx, &(cursor_config[i]), &error_cursor);

		if (cursoryx_error_get_code(&error_cursor) != CURSORYX_ERROR_OK)
		{
			cursoryx_error_log(cursoryx, &error_cursor);
			cursoryx_clean(cursoryx, &error_cursor);
			globuf_window_destroy(globuf, &error);
			globuf_clean(globuf, &error);
			return 1;
		}
	}

#if defined(GLOBUF_EXAMPLE_WAYLAND)
	dpishit_set_wayland_surface(
		dpishit,
		globuf_get_wayland_surface(globuf),
		&error_display);
#endif

	// display the window
	globuf_window_start(globuf, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// do some more stuff while the window runs in another thread
	printf(
		"this is a message from the main thread\n"
		"the window should now be visible\n"
		"we can keep computing here\n");

	// wait for the window to be closed
	globuf_window_block(globuf, &error);

	// stop willis
	willis_stop(willis, &error_input);

	if (willis_error_get_code(&error_input) != WILLIS_ERROR_OK)
	{
		willis_error_log(willis, &error_input);
		willis_clean(willis, &error_input);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	willis_clean(willis, &error_input);

	if (willis_error_get_code(&error_input) != WILLIS_ERROR_OK)
	{
		willis_error_log(willis, &error_input);
		willis_clean(willis, &error_input);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// stop dpishit
	dpishit_stop(dpishit, &error_display);

	if (dpishit_error_get_code(&error_display) != DPISHIT_ERROR_OK)
	{
		dpishit_error_log(dpishit, &error_display);
		dpishit_clean(dpishit, &error_display);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	dpishit_clean(dpishit, &error_display);

	if (dpishit_error_get_code(&error_display) != DPISHIT_ERROR_OK)
	{
		dpishit_error_log(dpishit, &error_display);
		dpishit_clean(dpishit, &error_display);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// stop cursoryx
	cursoryx_stop(cursoryx, &error_cursor);

	if (cursoryx_error_get_code(&error_cursor) != CURSORYX_ERROR_OK)
	{
		cursoryx_error_log(cursoryx, &error_cursor);
		cursoryx_clean(cursoryx, &error_cursor);
		globuf_error_log(globuf, &error_render);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	cursoryx_clean(cursoryx, &error_cursor);

	if (cursoryx_error_get_code(&error_cursor) != CURSORYX_ERROR_OK)
	{
		cursoryx_error_log(cursoryx, &error_cursor);
		cursoryx_clean(cursoryx, &error_cursor);
		globuf_error_log(globuf, &error_render);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// stop globuf
	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// handle event thread errors
	if (globuf_error_get_code(&error_events) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error_events);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// handle render thread errors
	if (globuf_error_get_code(&error_render) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error_render);
		globuf_window_destroy(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	// free resources correctly
	globuf_window_destroy(globuf, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		globuf_clean(globuf, &error);
		return 1;
	}

	globuf_clean(globuf, &error);

	if (globuf_error_get_code(&error) != GLOBUF_ERROR_OK)
	{
		globuf_error_log(globuf, &error);
		return 1;
	}

	return 0;
}
