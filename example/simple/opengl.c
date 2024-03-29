#include "globox.h"

#if defined(GLOBOX_EXAMPLE_X11)
#if defined(GLOBOX_EXAMPLE_GLX)
	#include "globox_x11_glx.h"
#elif defined(GLOBOX_EXAMPLE_EGL)
	#include "globox_x11_egl.h"
#endif
#elif defined(GLOBOX_EXAMPLE_APPKIT)
#include "globox_appkit_egl.h"
#elif defined(GLOBOX_EXAMPLE_WIN)
#include "globox_win_wgl.h"
#elif defined(GLOBOX_EXAMPLE_WAYLAND)
#include "globox_wayland_egl.h"
#endif

#ifdef GLOBOX_EXAMPLE_APPKIT
#define main real_main
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(GLOBOX_EXAMPLE_GLX)
	#include <GL/glx.h>
	#include <GLES2/gl2.h>
#elif defined(GLOBOX_EXAMPLE_EGL)
	#include <EGL/egl.h>
	#include <GLES2/gl2.h>
#elif defined(GLOBOX_EXAMPLE_WGL)
	#include <GL/gl.h>
	#undef WGL_WGLEXT_PROTOTYPES
	#include <GL/wglext.h>
	#define GL_GLES_PROTOTYPES 0
	#include <GLES2/gl2.h>
#endif

extern uint8_t iconpix[];
extern int iconpix_size;

#if defined(GLOBOX_EXAMPLE_APPKIT)
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

char* feature_names[GLOBOX_FEATURE_COUNT] =
{
	[GLOBOX_FEATURE_INTERACTION] = "interaction",
	[GLOBOX_FEATURE_STATE] = "state",
	[GLOBOX_FEATURE_TITLE] = "title",
	[GLOBOX_FEATURE_ICON] = "icon",
	[GLOBOX_FEATURE_SIZE] = "size",
	[GLOBOX_FEATURE_POS] = "pos",
	[GLOBOX_FEATURE_FRAME] = "frame",
	[GLOBOX_FEATURE_BACKGROUND] = "background",
	[GLOBOX_FEATURE_VSYNC] = "vsync",
};

#if defined(GLOBOX_EXAMPLE_GLX)
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
#elif defined(GLOBOX_EXAMPLE_EGL)
EGLint egl_config_attrib[] =
{
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
#if defined (GLOBOX_EXAMPLE_APPKIT)
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#else
	EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
#endif
	EGL_NONE,
};
#elif defined(GLOBOX_EXAMPLE_WGL)
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

struct globox_render_data
{
	struct globox* globox;
	bool shaders;
};

static void compile_shaders()
{
	GLint out;

	// compile OpenGL or glES shaders
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
#if defined(GLOBOX_EXAMPLE_APPKIT)
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
#if defined(GLOBOX_EXAMPLE_APPKIT)
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
	struct globox* globox = data;
	struct globox_error_info error = {0};

	// print some debug info on internal events
	enum globox_event abstract =
		globox_handle_events(globox, event, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		return;
	}

	switch (abstract)
	{
		case GLOBOX_EVENT_INVALID:
		{
			// shouldn't be possible since we handle the error
			fprintf(stderr, "received invalid event\n");
			break;
		}
		case GLOBOX_EVENT_UNKNOWN:
		{
#ifdef GLOBOX_EXAMPLE_LOG_ALL
			fprintf(stderr, "received unknown event\n");
#endif
			break;
		}
		case GLOBOX_EVENT_RESTORED:
		{
			fprintf(stderr, "received `restored` event\n");
			break;
		}
		case GLOBOX_EVENT_MINIMIZED:
		{
			fprintf(stderr, "received `minimized` event\n");
			break;
		}
		case GLOBOX_EVENT_MAXIMIZED:
		{
			fprintf(stderr, "received `maximized` event\n");
			break;
		}
		case GLOBOX_EVENT_FULLSCREEN:
		{
			fprintf(stderr, "received `fullscreen` event\n");
			break;
		}
		case GLOBOX_EVENT_CLOSED:
		{
			fprintf(stderr, "received `closed` event\n");
			break;
		}
		case GLOBOX_EVENT_MOVED_RESIZED:
		{
			fprintf(stderr, "received `moved` event\n");
			break;
		}
		case GLOBOX_EVENT_DAMAGED:
		{
#ifdef GLOBOX_EXAMPLE_LOG_ALL
			struct globox_rect rect = globox_get_expose(globox, &error);

			if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
			{
				globox_error_log(globox, &error);
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
}

static void render_callback(void* data)
{
	// render our trademark square as a simple example, updating the whole
	// buffer each time without taking surface damage events into account
	struct globox_render_data* render_data = data;
	struct globox* globox = render_data->globox;
	struct globox_error_info error = {0};

	int width = globox_get_width(globox, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		return;
	}

	int height = globox_get_height(globox, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
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
#ifdef GLOBOX_EXAMPLE_WGL
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

	globox_update_content(globox, NULL, &error);

	// reducing latency when resizing
	glFinish();

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		return;
	}
}

static void config_callback(struct globox_config_reply* replies, size_t count, void* data)
{
	fprintf(stderr, "window configured succesfully, printing information:\n");

	struct globox* context = data;
	const char* message = NULL;
	size_t feature;

	for (size_t i = 0; i < count; ++i)
	{
		feature = replies[i].feature;

		if (feature < GLOBOX_FEATURE_COUNT)
		{
			if (replies[i].error.code == GLOBOX_ERROR_OK)
			{
				message = "success";
			}
			else
			{
				message = globox_error_get_msg(context, &replies[i].error);
			}

			fprintf(stderr, "\t%s: %s\n", feature_names[feature], message);
		}
	}
}

int main(int argc, char** argv)
{
	struct globox_error_info error = {0};
	struct globox_error_info error_early = {0};
	printf("starting the simple globox example\n");

	// prepare function pointers
	struct globox_config_backend config = {0};

#if defined(GLOBOX_EXAMPLE_X11)
#if defined(GLOBOX_EXAMPLE_GLX)
	globox_prepare_init_x11_glx(&config, &error_early);
#elif defined(GLOBOX_EXAMPLE_EGL)
	globox_prepare_init_x11_egl(&config, &error_early);
#endif
#elif defined(GLOBOX_EXAMPLE_APPKIT)
	globox_prepare_init_appkit_egl(&config, &error_early);
#elif defined(GLOBOX_EXAMPLE_WIN)
	globox_prepare_init_win_wgl(&config, &error_early);
#elif defined(GLOBOX_EXAMPLE_WAYLAND)
	globox_prepare_init_wayland_egl(&config, &error_early);
#endif

	// set function pointers and perform basic init
	struct globox* globox = globox_init(&config, &error);

	// Unless the context allocation failed it is always possible to access
	// error messages (even when the context initialization failed) so we can
	// always handle the backend initialization error first.

	// context allocation failed
	if (globox == NULL)
	{
		fprintf(stderr, "could not allocate the main globox context\n");
		return 1;
	}

	// Backend initialization failed. Since it happens before globox
	// initialization and errors are accessible even if it fails, we can handle
	// the errors in the right order regardless.
	if (globox_error_get_code(&error_early) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error_early);
		globox_clean(globox, &error);
		return 1;
	}

	// The globox initialization had failed, make it known now if the backend
	// initialization that happened before went fine.
	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	// set OpenGL configuration attributes
	struct globox_config_opengl config_opengl =
	{
		.major_version = 2,
		.minor_version = 0,
#if defined(GLOBOX_EXAMPLE_GLX)
		.attributes = glx_config_attrib,
#elif defined(GLOBOX_EXAMPLE_EGL)
		.attributes = egl_config_attrib,
#elif defined(GLOBOX_EXAMPLE_WGL)
		.attributes = wgl_config_attrib,
#endif
	};

	globox_init_opengl(globox, &config_opengl, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	// get available features
	struct globox_config_features* feature_list =
		globox_init_features(globox, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	// initialize features when creating the window
	struct globox_feature_state state =
	{
		.state = GLOBOX_STATE_REGULAR,
	};

	struct globox_feature_title title =
	{
		.title = "globox",
	};

	struct globox_feature_icon icon =
	{
		// acceptable implementation-defined behavior
		// since it's also the implementation that
		// allows us to bundle resources like so
		.pixmap = (uint32_t*) iconpix,
		.len = 2 + (16 * 16) + 2 + (32 * 32) + 2 + (64 * 64),
	};

	struct globox_feature_size size =
	{
		.width = 500,
		.height = 500,
	};

	struct globox_feature_pos pos =
	{
		.x = 250,
		.y = 250,
	};

	struct globox_feature_frame frame =
	{
		.frame = true,
	};

	struct globox_feature_background background =
	{
		.background = GLOBOX_BACKGROUND_BLURRED,
	};

	struct globox_feature_vsync vsync =
	{
		.vsync = true,
	};

	// configure the feature and print a list
	printf("received a list of available features:\n");

	struct globox_config_request configs[GLOBOX_FEATURE_COUNT] = {0};
	size_t feature_added = 0;
	size_t i = 0;

	while (i < feature_list->count)
	{
		enum globox_feature feature_id = feature_list->list[i];
		printf("\t%s\n", feature_names[feature_id]);
		++i;

		switch (feature_id)
		{
			case GLOBOX_FEATURE_STATE:
			{
				configs[feature_added].config = &state;
				break;
			}
			case GLOBOX_FEATURE_TITLE:
			{
				configs[feature_added].config = &title;
				break;
			}
			case GLOBOX_FEATURE_ICON:
			{
				configs[feature_added].config = &icon;
				break;
			}
			case GLOBOX_FEATURE_SIZE:
			{
				configs[feature_added].config = &size;
				break;
			}
			case GLOBOX_FEATURE_POS:
			{
				configs[feature_added].config = &pos;
				break;
			}
			case GLOBOX_FEATURE_FRAME:
			{
				configs[feature_added].config = &frame;
				break;
			}
			case GLOBOX_FEATURE_BACKGROUND:
			{
				configs[feature_added].config = &background;
				break;
			}
			case GLOBOX_FEATURE_VSYNC:
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

	// register an event handler to track the window's state
	struct globox_config_events events =
	{
		.data = globox,
		.handler = event_callback,
	};

	struct globox_error_info error_events = {0};
	globox_init_events(globox, &events, &error_events);

	if (globox_error_get_code(&error_events) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error_events);
		globox_clean(globox, &error);
		return 1;
	}

	// register a render callback
	struct globox_render_data render_data =
	{
		.globox = globox,
		.shaders = true,
	};

	struct globox_config_render render =
	{
		.data = &render_data,
		.callback = render_callback,
	};

	struct globox_error_info error_render = {0};
	globox_init_render(globox, &render, &error_render);

	if (globox_error_get_code(&error_render) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error_render);
		globox_clean(globox, &error);
		return 1;
	}

	// create the window
	globox_window_create(globox, configs, feature_added, config_callback, globox, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	// check window
	globox_window_confirm(globox, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	// display the window
	globox_window_start(globox, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		globox_window_destroy(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	// do some more stuff while the window runs in another thread
	printf(
		"this is a message from the main thread\n"
		"the window should now be visible\n"
		"we can keep computing here\n");

	// wait for the window to be closed
	globox_window_block(globox, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		globox_window_destroy(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	// handle event thread errors
	if (globox_error_get_code(&error_events) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error_events);
		globox_window_destroy(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	// handle render thread errors
	if (globox_error_get_code(&error_render) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error_render);
		globox_window_destroy(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	// free resources correctly
	globox_window_destroy(globox, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		globox_clean(globox, &error);
		return 1;
	}

	globox_clean(globox, &error);

	if (globox_error_get_code(&error) != GLOBOX_ERROR_OK)
	{
		globox_error_log(globox, &error);
		return 1;
	}

	return 0;
}
