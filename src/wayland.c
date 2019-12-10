#define _XOPEN_SOURCE 500
#ifdef GLOBOX_WAYLAND

#include "wayland.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "globox.h"

inline void random_id(char *str)
{
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    uint64_t random = time.tv_nsec;

    for (int i = 0; i < 6; ++i)
	{
        str[i] = 'A' + (random&15) + ((random&16) * 2);
        random >>= 5;
    }
}

int create_shm_file(void)
{
    uint8_t retries = 100;

    do
	{
        char name[] = "/wl_shm-XXXXXX";
        random_id(name + (sizeof (name)) - 7);

        int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        --retries;

        if (fd >= 0)
		{
            shm_unlink(name);

            return fd;
        }
    }
	while ((retries > 0) && (errno == EEXIST));

    return -1;
}

int allocate_shm_file(size_t size)
{
	// create file descriptor
    int fd = create_shm_file();

    if (fd < 0)
	{
        return -1;
	}

	// truncate
    int ret;

    do
	{
        ret = ftruncate(fd, size);
    }
	while ((ret < 0) && (errno == EINTR));

    if (ret < 0)
	{
        close(fd);

        return -1;
    }

    return fd;
}

struct wl_buffer* draw_frame(struct globox* globox)
{
    const uint32_t width = 640;
	const uint32_t height = 480;

    int stride = width * 4;
    int size = stride * height;

    int fd = allocate_shm_file(size);

    if (fd == -1)
	{
        return NULL;
    }

    uint32_t *data = mmap(
		NULL,
		size,
        PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fd,
		0);

    if (data == MAP_FAILED)
	{
        close(fd);

        return NULL;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(
		globox->wl_shm,
		fd,
		size);

    struct wl_buffer *buffer = wl_shm_pool_create_buffer(
		pool,
		0,
		width,
		height,
		stride,
		WL_SHM_FORMAT_XRGB8888);

    wl_shm_pool_destroy(pool);
    close(fd);

	// checkerboard example
    for (uint32_t y = 0; y < height; ++y)
	{
        for (uint32_t x = 0; x < width; ++x)
		{
            if ((x + y / 8 * 8) % 16 < 8)
			{
                data[y * width + x] = 0xFF666666;
			}
            else
			{
                data[y * width + x] = 0xFFEEEEEE;
			}
        }
    }

    munmap(data, size);
    wl_buffer_add_listener(buffer, &(globox->wl_buffer_listener), NULL);

    return buffer;
}

// callbacks
void wl_buffer_release(void *data, struct wl_buffer *wl_buffer)
{
    wl_buffer_destroy(wl_buffer);
}

void xdg_surface_configure(
	void *data,
	struct xdg_surface *xdg_surface,
	uint32_t serial)
{
    struct globox* globox = data;
    xdg_surface_ack_configure(xdg_surface, serial);

    struct wl_buffer *buffer = draw_frame(globox);
    wl_surface_attach(globox->wl_surface, buffer, 0, 0);
    wl_surface_commit(globox->wl_surface);
}

void xdg_wm_base_ping(
	void *data,
	struct xdg_wm_base *xdg_wm_base,
	uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

void registry_global(
	void *data,
	struct wl_registry *wl_registry,
	uint32_t name,
	const char *interface,
	uint32_t version)
{
    struct globox* globox = data;

    if (strcmp(interface, wl_shm_interface.name) == 0)
	{
        globox->wl_shm = wl_registry_bind(
			wl_registry,
			name,
			&wl_shm_interface,
			1);
    }
	else if (strcmp(interface, wl_compositor_interface.name) == 0)
	{
        globox->wl_compositor = wl_registry_bind(
			wl_registry,
			name,
			&wl_compositor_interface,
			4);
    }
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
	{
        globox->xdg_wm_base = wl_registry_bind(
			wl_registry,
			name,
			&xdg_wm_base_interface,
			1);

        xdg_wm_base_add_listener(
			globox->xdg_wm_base,
			&(globox->xdg_wm_base_listener),
			globox);
    }
}

void registry_global_remove(
	void *data,
	struct wl_registry *wl_registry,
	uint32_t name)
{

}

#endif