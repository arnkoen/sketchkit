#include "sk.h"
#include "hmm.h"
#include <stdio.h>

typedef struct {
	float x;
	float y;
	float z;
	uint32_t col;
} vertex_t;

static const vertex_t vertices [] = {
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t indices[] = {
	0, 1, 2, // 0
	1, 3, 2,
	4, 6, 5, // 2
	5, 6, 7,
	0, 2, 4, // 4
	4, 2, 6,
	1, 5, 3, // 6
	5, 7, 3,
	0, 4, 1, // 8
	4, 5, 1,
	2, 3, 6, // 10
	6, 3, 7,
};

int main(int argc, char* argv[]) {
	RGFW_window* win = sk_init(&(sk_desc) { 
		.width = 1280,
		.height = 720,
		.flags = RGFW_windowCenter | RGFW_windowNoInitAPI,
		.renderer = BGFX_RENDERER_TYPE_VULKAN,
		.reset = BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X4,
	});

    bgfx_set_view_clear(
		0,
		BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
		0x201020ff,
		1.0f, 
		0
    );

	bgfx_vertex_layout_t layout = {0};
	bgfx_vertex_layout_begin(&layout, BGFX_RENDERER_TYPE_VULKAN);
	bgfx_vertex_layout_add(&layout, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_layout_add(&layout, BGFX_ATTRIB_COLOR0, 4, BGFX_ATTRIB_TYPE_UINT8, true, true);
	bgfx_vertex_layout_end(&layout);

	const bgfx_memory_t* vtx_mem = bgfx_make_ref(&vertices, sizeof(vertices));
	bgfx_vertex_buffer_handle_t vbuf = bgfx_create_vertex_buffer(vtx_mem, &layout, BGFX_BUFFER_NONE);

	const bgfx_memory_t* idx_mem = bgfx_make_ref(&indices, sizeof(indices));
	bgfx_index_buffer_handle_t ibuf = bgfx_create_index_buffer(idx_mem, BGFX_BUFFER_NONE);

	bgfx_program_handle_t program = sk_load_program(
		"vk/hello.vs.bin",
		"vk/hello.fs.bin"
	);

	uint64_t state = 0|
        BGFX_STATE_WRITE_RGB| 
        BGFX_STATE_WRITE_Z|
        BGFX_STATE_DEPTH_TEST_LESS|
        BGFX_STATE_MSAA;

	bgfx_set_state(state, 0);
	bgfx_set_view_rect(0, 0, 0, (uint16_t)win->r.w, (uint16_t)win->r.h);

	sk_cam cam = {0};
	sk_init_cam(&cam, &(sk_cam_desc) {
		.distance = 4.f,
	});

    while (!RGFW_window_shouldClose(win)) {
        while (RGFW_window_checkEvent(win) != NULL) {
			if (win->event.type == RGFW_quit) {
				break;
			}
			if (win->event.type == RGFW_windowResized) {
				bgfx_reset(
					(uint32_t)win->r.w, 
					(uint32_t)win->r.h, 
					BGFX_RESET_VSYNC | BGFX_RESET_MSAA_X4,
					BGFX_TEXTURE_FORMAT_COUNT
				);
				bgfx_set_view_rect(0, 0, 0, (uint16_t)win->r.w, (uint16_t)win->r.h);
			}
			sk_cam_input(&cam, win);
        }
		sk_update_cam(&cam, win->r.w, win->r.h);
		bgfx_set_view_transform(0, &cam.view.Elements[0], &cam.proj.Elements[0]);

		HMM_Mat4 model = HMM_Translate(HMM_V3(0, 0, 0));
		bgfx_set_transform(&model.Elements[0], 1);

		bgfx_set_vertex_buffer(0, vbuf, 0, sizeof(vertices) / sizeof(vertex_t));
		bgfx_set_index_buffer(ibuf, 0, sizeof(indices) / sizeof(uint16_t));

		bgfx_submit(0, program, 0, BGFX_DISCARD_NONE);
		bgfx_frame(false);
    }

	sk_shutdown(win);
}
