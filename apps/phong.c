#include "sk.h"
#include "hmm.h"
#include <stdio.h>

typedef struct {
    HMM_Vec4 ambient;
    HMM_Vec4 diffuse;
    HMM_Vec4 specular;
    HMM_Vec4 shininess;
    bgfx_uniform_handle_t u_ambient;
    bgfx_uniform_handle_t u_diffuse;
    bgfx_uniform_handle_t u_specular;
    bgfx_uniform_handle_t u_shininess;
} material_t;

static void init_material_uniforms(material_t* mat) {
    mat->u_ambient = bgfx_create_uniform("u_matAmbient", BGFX_UNIFORM_TYPE_VEC4, 1);
    mat->u_diffuse = bgfx_create_uniform("u_matDiffuse", BGFX_UNIFORM_TYPE_VEC4, 1);
    mat->u_specular = bgfx_create_uniform("u_matSpecular", BGFX_UNIFORM_TYPE_VEC4, 1);
    mat->u_shininess = bgfx_create_uniform("u_matShininess", BGFX_UNIFORM_TYPE_VEC4, 1);
}

static void apply_material_uniforms(material_t* mat) {
    bgfx_set_uniform(mat->u_ambient, &mat->ambient.Elements[0], 1);
    bgfx_set_uniform(mat->u_diffuse, &mat->diffuse.Elements[0], 1);
    bgfx_set_uniform(mat->u_specular, &mat->specular.Elements[0], 1);
    bgfx_set_uniform(mat->u_shininess, &mat->shininess.Elements[0], 1);
}

typedef struct {
    HMM_Vec4 direction;
    HMM_Vec4 ambient;
    HMM_Vec4 diffuse;
    HMM_Vec4 specular;
    bgfx_uniform_handle_t u_direction;
    bgfx_uniform_handle_t u_ambient;
    bgfx_uniform_handle_t u_diffuse;
    bgfx_uniform_handle_t u_specular;
} light_t;

static void init_light_uniforms(light_t* l) {
    l->u_direction = bgfx_create_uniform("u_lightDir", BGFX_UNIFORM_TYPE_VEC4, 1);
    l->u_ambient = bgfx_create_uniform("u_lightAmbient", BGFX_UNIFORM_TYPE_VEC4, 1);
    l->u_diffuse = bgfx_create_uniform("u_lightDiffuse", BGFX_UNIFORM_TYPE_VEC4, 1);
    l->u_specular = bgfx_create_uniform("u_lightSpecular", BGFX_UNIFORM_TYPE_VEC4, 1);
}

static void apply_light_uniforms(light_t* l) {
    bgfx_set_uniform(l->u_ambient, &l->ambient.Elements[0], 1);
    bgfx_set_uniform(l->u_diffuse, &l->diffuse.Elements[0], 1);
    bgfx_set_uniform(l->u_specular, &l->specular.Elements[0], 1);
    bgfx_set_uniform(l->u_direction, &l->direction.Elements[0], 1);
}


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
		0x101020ff,
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

	bgfx_program_handle_t program = sk_load_program("vk/phong.vs.bin", "vk/phong.fs.bin");

    light_t light = {
        .ambient = HMM_V4(.5f, .5f, .5f, 1.0f),
        .diffuse = HMM_V4(1, 1, 1, 1),
        .specular = HMM_V4(.5f, .5f, .5f, 1),
        .direction = HMM_V4(-0.5f, -1, -.25f, 0),
    };
    init_light_uniforms(&light);

    material_t mat = {
        .ambient = HMM_V4(.2f, .2f, .2f, 1.0f),
        .diffuse = HMM_V4(0.8, 0.2, 0.1, 1),
        .specular = HMM_V4(0.1f, 0.1f, 0.1f, 1),
        .shininess = HMM_V4(32.f, 0, 0, 0),
    };
    init_material_uniforms(&mat);

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

        apply_light_uniforms(&light);
        apply_material_uniforms(&mat);

		sk_set_mesh(&mesh);
		bgfx_submit(0, program, 0, BGFX_DISCARD_NONE);

		bgfx_frame(false);
    }

	sk_shutdown(win);
}

