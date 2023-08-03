#include "include/globox.h"
#include "common/globox_private.h"
#include "win/win_common.h"
#include "win/win_common_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dwmapi.h>
#include <process.h>
#include <shellscalingapi.h>
#include <windows.h>
#include <winuser.h>

#ifdef GLOBOX_ERROR_HELPER_WIN
#include <errhandlingapi.h>
#endif

unsigned __stdcall win_helpers_render_loop(void* data)
{
	struct win_thread_render_loop_data* thread_render_loop_data = data;

	struct globox* context = thread_render_loop_data->globox;
	struct win_platform* platform = thread_render_loop_data->platform;
	struct globox_error_info* error = thread_render_loop_data->error;

	bool closed = platform->closed;

	// wait for the window cond
	AcquireSRWLockExclusive(&(platform->lock_render));

	while (platform->render == false)
	{
		SleepConditionVariableSRW(
			&(platform->cond_render),
			&(platform->lock_render),
			INFINITE,
			0);
	}

	ReleaseSRWLockExclusive(&(platform->lock_render));

	// thread init callback
	if (platform->render_init_callback != NULL)
	{
		platform->render_init_callback(thread_render_loop_data);

		if (globox_error_get_code(error) != GLOBOX_ERROR_OK)
		{
			_endthreadex(0);
			return 1;
		}
	}

	while (closed == false)
	{
		// run developer callback
		context->render_callback.callback(context->render_callback.data);

		// update boolean
		closed = platform->closed;
	}

	_endthreadex(0);
	return 0;
}

unsigned __stdcall win_helpers_event_loop(void* data)
{
	struct win_thread_event_loop_data* thread_event_loop_data = data;

	struct globox* context = thread_event_loop_data->globox;
	struct win_platform* platform = thread_event_loop_data->platform;
	struct globox_error_info* error = thread_event_loop_data->error;

	DWORD style = 0;
	DWORD exstyle = 0;

	if (context->feature_frame->frame == true)
	{
		style |= WS_OVERLAPPEDWINDOW;
	}
	else
	{
		style |= WS_POPUP;
		style |= WS_BORDER;
	}

	platform->event_handle =
		CreateWindowExW(
			exstyle,
			platform->window_class_name,
			platform->window_class_name,
			style,
			context->feature_pos->x,
			context->feature_pos->y,
			context->feature_size->width,
			context->feature_size->height,
			NULL,
			NULL,
			platform->window_class_module,
			data);

	if (platform->event_handle == NULL)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_WINDOW_CREATE);
		_endthreadex(0);
		return 1;
	}

	ShowWindow(platform->event_handle, SW_SHOWNORMAL);

	if (context->feature_background->background != GLOBOX_BACKGROUND_OPAQUE)
	{
		DWM_BLURBEHIND blur_behind =
		{
			.dwFlags = DWM_BB_ENABLE,
			.fEnable = TRUE,
			.hRgnBlur = NULL,
			.fTransitionOnMaximized = FALSE,
		};

		HRESULT ok_blur =
			DwmEnableBlurBehindWindow(
				platform->event_handle,
				&blur_behind);

		if (ok_blur != S_OK)
		{
			globox_error_throw(context, error, GLOBOX_ERROR_WIN_DWM_ENABLE);

			#if defined(GLOBOX_COMPAT_WINE)
				globox_error_ok(error);
			#else
				_endthreadex(0);
				return 1;
			#endif
		}
	}

	BOOL ok_placement =
		GetWindowPlacement(platform->event_handle, &(platform->placement));

	if (ok_placement == FALSE)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_PLACEMENT_GET);
		_endthreadex(0);
		return 1;
	}

	switch (context->feature_state->state)
	{
		case GLOBOX_STATE_FULLSCREEN:
		{
			globox_feature_set_state(context, context->feature_state, error);
			break;
		}
		case GLOBOX_STATE_MAXIMIZED:
		{
			ShowWindow(platform->event_handle, SW_SHOWMAXIMIZED);
			break;
		}
		case GLOBOX_STATE_MINIMIZED:
		{
			ShowWindow(platform->event_handle, SW_SHOWMINIMIZED);
			break;
		}
		default:
		{
			break;
		}
	}

	// trigger the window creation cond
	WakeConditionVariable(&(platform->cond_window));

	// start the render thread
	platform->thread_render =
		(HANDLE) _beginthreadex(
			NULL,
			0,
			win_helpers_render_loop,
			(void*) &(platform->thread_render_loop_data),
			0,
			NULL);

	if (platform->thread_render == 0)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_THREAD_RENDER_START);
		_endthreadex(0);
		return 1;
	}

	// start the event loop
	MSG msg;
	BOOL ok_msg = GetMessageW(&msg, NULL, 0, 0);

	while (ok_msg != 0)
	{
		// According to the documentation, the BOOL returned by GetMessage has
		// three states, because it's obviously what bools are made for...
		// -1 means failure
		// 0 means we just received a WM_QUIT message
		// any other value means we received another message
		if (ok_msg == -1)
		{
			globox_error_throw(context, error, GLOBOX_ERROR_WIN_MSG_GET);
			_endthreadex(0);
			return 1;
		}

		// dispatch messsage
		TranslateMessage(&msg);
		DispatchMessageW(&msg);

		// get next message
		ok_msg = GetMessageW(&msg, NULL, 0, 0);
	}

	_endthreadex(0);
	return 0;
}

LRESULT CALLBACK win_helpers_window_procedure(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	// process message first
	switch (msg)
	{
		case WM_ERASEBKGND:
		{
			// ignore clearing requests
			return S_OK;
		}
		case WM_CREATE:
		{
			// save a context pointer in the window
			SetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA,
				(LONG_PTR) ((CREATESTRUCT*) lParam)->lpCreateParams);
			break;
		}
		default:
		{
			break;
		}
	}

	LRESULT result = DefWindowProc(hwnd, msg, wParam, lParam);

	// get context pointer from window
	struct win_thread_event_loop_data* thread_event_loop_data =
		(struct win_thread_event_loop_data*)
			GetWindowLongPtrW(hwnd, GWLP_USERDATA);

	if (thread_event_loop_data == NULL)
	{
		return result;
	}

	struct globox* context = thread_event_loop_data->globox;
	struct win_platform* platform = thread_event_loop_data->platform;
	struct globox_error_info* error = thread_event_loop_data->error;

	// run developer callback
	MSG event =
	{
		.message = msg,
		.wParam = wParam,
		.lParam = lParam,
	};

	context->event_callbacks.handler(context->event_callbacks.data, &event);

	// handle interactive move and resize
	if (msg == WM_NCHITTEST)
	{
		switch (context->feature_interaction->action)
		{
			case GLOBOX_INTERACTION_MOVE:
			{
				result = HTCAPTION;
				break;
			}
			case GLOBOX_INTERACTION_N:
			{
				result = HTTOP;
				break;
			}
			case GLOBOX_INTERACTION_NW:
			{
				result = HTTOPLEFT;
				break;
			}
			case GLOBOX_INTERACTION_W:
			{
				result = HTLEFT;
				break;
			}
			case GLOBOX_INTERACTION_SW:
			{
				result = HTBOTTOMLEFT;
				break;
			}
			case GLOBOX_INTERACTION_S:
			{
				result = HTBOTTOM;
				break;
			}
			case GLOBOX_INTERACTION_SE:
			{
				result = HTBOTTOMRIGHT;
				break;
			}
			case GLOBOX_INTERACTION_E:
			{
				result = HTRIGHT;
				break;
			}
			case GLOBOX_INTERACTION_NE:
			{
				result = HTTOPRIGHT;
				break;
			}
			default:
			{
				break;
			}
		}
	}

	// pass over the message processor return value
	return result;
}

void win_helpers_features_init(
	struct globox* context,
	struct win_platform* platform,
	struct globox_config_request* configs,
	size_t count,
	struct globox_error_info* error)
{
	for (size_t i = 0; i < count; ++i)
	{
		switch (configs[i].feature)
		{
			case GLOBOX_FEATURE_STATE:
			{
				if (configs[i].config != NULL)
				{
					*(context->feature_state) =
						*((struct globox_feature_state*)
							configs[i].config);
				}

				break;
			}
			case GLOBOX_FEATURE_TITLE:
			{
				if (configs[i].config != NULL)
				{
					struct globox_feature_title* tmp = configs[i].config;
					context->feature_title->title = strdup(tmp->title);
				}

				break;
			}
			case GLOBOX_FEATURE_ICON:
			{
				if (configs[i].config != NULL)
				{
					struct globox_feature_icon* tmp = configs[i].config;
					context->feature_icon->pixmap = malloc(tmp->len * 4);

					if (context->feature_icon->pixmap != NULL)
					{
						memcpy(
							context->feature_icon->pixmap,
							tmp->pixmap,
							tmp->len * 4);

						context->feature_icon->len = tmp->len;
					}
					else
					{
						context->feature_icon->len = 0;
					}
				}

				break;
			}
			case GLOBOX_FEATURE_SIZE:
			{
				// handled directly in xcb's window creation code
				if (configs[i].config != NULL)
				{
					*(context->feature_size) =
						*((struct globox_feature_size*)
							configs[i].config);
				}

				break;
			}
			case GLOBOX_FEATURE_POS:
			{
				// handled directly in xcb's window creation code
				if (configs[i].config != NULL)
				{
					*(context->feature_pos) =
						*((struct globox_feature_pos*)
							configs[i].config);
				}

				break;
			}
			case GLOBOX_FEATURE_FRAME:
			{
				if (configs[i].config != NULL)
				{
					*(context->feature_frame) =
						*((struct globox_feature_frame*)
							configs[i].config);
				}

				break;
			}
			case GLOBOX_FEATURE_BACKGROUND:
			{
				if (configs[i].config != NULL)
				{
					*(context->feature_background) =
						*((struct globox_feature_background*)
							configs[i].config);
				}

				break;
			}
			case GLOBOX_FEATURE_VSYNC:
			{
				if (configs[i].config != NULL)
				{
					*(context->feature_vsync) =
						*((struct globox_feature_vsync*)
							configs[i].config);
					context->feature_vsync->vsync = true;
				}

				break;
			}
			default:
			{
				globox_error_throw(context, error, GLOBOX_ERROR_FEATURE_INVALID);
				return;
			}
		}
	}
}

void win_helpers_set_state(
	struct globox* context,
	struct win_platform* platform,
	struct globox_error_info* error)
{
	switch (context->feature_state->state)
	{
		case GLOBOX_STATE_MINIMIZED:
		{
			SendMessage(
				platform->event_handle,
				WIN_USER_MSG_MINIMIZE,
				0,
				0);
			break;
		}
		case GLOBOX_STATE_MAXIMIZED:
		{
			SendMessage(
				platform->event_handle,
				WIN_USER_MSG_MAXIMIZE,
				0,
				0);
			break;
		}
		case GLOBOX_STATE_FULLSCREEN:
		{
			SendMessage(
				platform->event_handle,
				WIN_USER_MSG_FULLSCREEN,
				0,
				0);
			break;
		}
		default:
		{
			SendMessage(
				platform->event_handle,
				WIN_USER_MSG_REGULAR,
				0,
				0);
			break;
		}
	}

	globox_error_ok(error);
}

LPWSTR win_helpers_utf8_to_wchar(const char* string)
{
	size_t codepoints =
		MultiByteToWideChar(
			CP_UTF8,
			MB_PRECOMPOSED,
			string,
			-1,
			NULL,
			0);

	if (codepoints == 0)
	{
		return NULL;
	}

	wchar_t* buf = malloc(codepoints * (sizeof (wchar_t)));

	if (buf == NULL)
	{
		return NULL;
	}

	buf[0] = '\0';

	int ok =
		MultiByteToWideChar(
			CP_UTF8,
			MB_PRECOMPOSED,
			string,
			-1,
			buf,
			codepoints);

	if (ok == 0)
	{
		free(buf);
		return NULL;
	}

	return buf;
}

HICON win_helpers_bitmap_to_icon(
	struct globox* context,
	struct win_platform* platform,
	BITMAP* bmp,
	struct globox_error_info* error)
{
	HDC device_context = GetDC(platform->event_handle);

	if (device_context == NULL)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_DEVICE_CONTEXT_GET);
		return NULL;
	}

	HBITMAP mask =
		CreateCompatibleBitmap(
			device_context,
			bmp->bmWidth,
			bmp->bmHeight);

	if (mask == NULL)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_BMP_MASK_CREATE);
		return NULL;
	}

	HBITMAP color = CreateBitmapIndirect(bmp);

	if (color == NULL)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_BMP_COLOR_CREATE);
		return NULL;
	}

	ICONINFO info =
	{
		.fIcon = TRUE,
		.xHotspot = 0,
		.yHotspot = 0,
		.hbmMask = mask,
		.hbmColor = color,
	};

	HICON icon = CreateIconIndirect(&info);

	if (icon == NULL)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_ICON_CREATE);
		return NULL;
	}

	int ok;

	ok = DeleteObject(mask);

	if (ok == 0)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_OBJECT_DELETE);
		return NULL;
	}

	ok = DeleteObject(color);

	if (ok == 0)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_OBJECT_DELETE);
		return NULL;
	}

	ReleaseDC(platform->event_handle, device_context);
	return icon;
}

void win_helpers_save_window_state(
	struct globox* context,
	struct win_platform* platform,
	struct globox_error_info* error)
{
	// save window state if relevant
	if (context->feature_state->state == GLOBOX_STATE_REGULAR)
	{
		BOOL ok =
			GetWindowPlacement(platform->event_handle, &(platform->placement));

		if (ok == FALSE)
		{
			globox_error_throw(
				context,
				error,
				GLOBOX_ERROR_WIN_PLACEMENT_GET);
		}
	}
}

enum win_dpi_api win_helpers_set_dpi_awareness()
{
	// try the Windows 10 API, v2 (available since the "creators update")
	BOOL ok;

	ok =
		SetProcessDpiAwarenessContext(
			DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	if (ok == TRUE)
	{
		return WIN_DPI_API_10_V2;
	}

	// try the Windows 10 API, v1
	ok =
		SetProcessDpiAwarenessContext(
			DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

	if (ok == TRUE)
	{
		return WIN_DPI_API_10_V1;
	}

	// try the Windows 8 API
	HRESULT result =
		SetProcessDpiAwareness(
			PROCESS_PER_MONITOR_DPI_AWARE);

	if (result == S_OK)
	{
		return WIN_DPI_API_8;
	}

	// try the Windows Vista API
	ok = SetProcessDPIAware();

	if (ok == TRUE)
	{
		return WIN_DPI_API_VISTA;
	}

	return WIN_DPI_API_NONE;
}

void win_helpers_set_title(
	struct globox* context,
	struct win_platform* platform,
	struct globox_error_info* error)
{
	platform->window_class_name =
		win_helpers_utf8_to_wchar(context->feature_title->title);

	if (platform->window_class_name == NULL)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_CLASS_NAME_SET);
		return;
	}

	globox_error_ok(error);
}

void win_helpers_set_icon(
	struct globox* context,
	struct win_platform* platform,
	struct globox_error_info* error)
{
	BITMAP pixmap_32 =
	{
		.bmType = 0,
		.bmWidth = 32,
		.bmHeight = 32,
		.bmWidthBytes = 32 * 4,
		.bmPlanes = 1,
		.bmBitsPixel = 32,
		.bmBits = context->feature_icon->pixmap + 4 + (16 * 16),
	};

	platform->icon_32 =
		win_helpers_bitmap_to_icon(context, platform, &pixmap_32, error);

	if (globox_error_get_code(error) != GLOBOX_ERROR_OK)
	{
		return;
	}

	if (platform->icon_32 != NULL)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_ICON_SMALL);
		return;
	}

	BITMAP pixmap_64 =
	{
		.bmType = 0,
		.bmWidth = 64,
		.bmHeight = 64,
		.bmWidthBytes = 64 * 4,
		.bmPlanes = 1,
		.bmBitsPixel = 32,
		.bmBits = context->feature_icon->pixmap + 4 + (16 * 16) + (32 * 32),
	};

	platform->icon_64 =
		win_helpers_bitmap_to_icon(context, platform, &pixmap_64, error);

	if (globox_error_get_code(error) != GLOBOX_ERROR_OK)
	{
		return;
	}

	if (platform->icon_64 != NULL)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_ICON_BIG);
		return;
	}

	globox_error_ok(error);
}

void win_helpers_set_frame(
	struct globox* context,
	struct win_platform* platform,
	struct globox_error_info* error)
{
	globox_error_ok(error);
}

void win_helpers_set_background(
	struct globox* context,
	struct win_platform* platform,
	struct globox_error_info* error)
{
	if (context->feature_background->background == GLOBOX_BACKGROUND_BLURRED)
	{
		globox_error_throw(context, error, GLOBOX_ERROR_WIN_BACKGROUND_BLUR);
		return;
	}

	globox_error_ok(error);
}

void win_helpers_set_vsync(
	struct globox* context,
	struct win_platform* platform,
	struct globox_error_info* error)
{
	globox_error_ok(error);
}

#ifdef GLOBOX_ERROR_HELPER_WIN
void win_helpers_win32_error_log(
	struct globox* context,
	struct win_platform* platform)
{
	DWORD error;
	LPVOID message;

	error = GetLastError(); 

	DWORD error_format =
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &message,
			0,
			NULL);

	if (error_format == 0)
	{
		fprintf(stderr, "could not get win32 error message\n");
		return;
	}

	fprintf(
		stderr,
		"# Win32 Error Report\n"
		"Error ID: %d\n"
		"Error Message: %s\n",
		error,
		(char*) message);
}
#endif
