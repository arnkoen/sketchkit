#include "sk.h"
#include "hmm.h"
#include <stdio.h>

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
		0x200020ff,
		1.0f, 
		0
    );

	uint64_t state = 0|
		BGFX_STATE_WRITE_RGB|
		BGFX_STATE_WRITE_Z|
		BGFX_STATE_DEPTH_TEST_LEQUAL|
		BGFX_STATE_MSAA;

	bgfx_set_state(state, 0);

	bgfx_set_view_rect(0, 0, 0, (uint16_t)win->r.w, (uint16_t)win->r.h);

	bgfx_program_handle_t program = sk_load_program("vk/model.vs.bin", "vk/model.fs.bin");
	bgfx_uniform_handle_t u_color = bgfx_create_uniform("u_color", BGFX_UNIFORM_TYPE_VEC4, 1);

	sk_mesh mesh = sk_load_m3d("assets/big.m3d");

	sk_cam cam = {0};
	sk_init_cam(&cam, &(sk_cam_desc) {
		.distance = 1.5f,
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

		HMM_Mat4 model = HMM_Translate(HMM_V3(0, -0.5f, 0));
		bgfx_set_transform(&model.Elements[0], 1);

		HMM_Vec4 color = HMM_V4(0.1f, 0.1f, 0.5f, 1.0f);
		bgfx_set_uniform(u_color, &color, 1);

		sk_set_mesh(&mesh);
		bgfx_submit(0, program, 0, BGFX_DISCARD_NONE);

		bgfx_frame(false);
    }

	sk_shutdown(win);
}
