#pragma once
#include <bgfx/c99/bgfx.h>
#include "RGFW.h"
#include "hmm.h"

typedef struct {
	uint32_t width;
	uint32_t height;
	const char* title;
	uint32_t reset;
	RGFW_windowFlags flags;
	bgfx_renderer_type_t renderer;
} sk_desc;

RGFW_window* sk_init(const sk_desc* desc);
void sk_shutdown(RGFW_window* window);

const bgfx_memory_t* sk_read_file(const char* path);
bgfx_shader_handle_t sk_load_shader(const char* path);
bgfx_program_handle_t sk_load_program(const char* vs_path, const char* fs_path);

//--IMAGE_LOADING-------------------------------------

typedef struct sk_image_data {
	uint32_t w, h;
	void* pixels;
} sk_image_data;

sk_image_data sk_load_image_data(const char* path);
void sk_release_image_data(sk_image_data* data);

// returns a 4*4 checker board texture.
bgfx_texture_handle_t sk_checker_texture(void);
// loads a texture from given file path. returns checker texture on fail.
bgfx_texture_handle_t sk_load_texture(const char* path);
// loads a texture from given image data. returns checker texture on fail.
bgfx_texture_handle_t sk_load_texture_mem(sk_image_data* data);

//--MESH---------------------------------------------

typedef struct sk_vertex_pnt {
	HMM_Vec3 pos;
	HMM_Vec3 norm;
	HMM_Vec2 uv;
} sk_vertex_pnt;

typedef struct sk_mesh {
	uint32_t element_count;
	bgfx_vertex_buffer_handle_t vbuf;
	bgfx_index_buffer_handle_t ibuf;
} sk_mesh;

sk_mesh sk_load_m3d(const char* path);
void sk_release_mesh(sk_mesh* mesh);
void sk_set_mesh(const sk_mesh* mesh);

//--CAM---------------------------------------------

typedef struct sk_cam_desc {
	float mindist;
	float maxdist;
	float minlat;
	float maxlat;
	float distance;
	float latitude;
	float longitude;
	float aspect;
	float nearz;
	float farz;
	float sensitivity;
	HMM_Vec3 center;
} sk_cam_desc;

typedef struct sk_cam {
	float mindist;
	float maxdist;
	float minlat;
	float maxlat;
	float distance;
	float latitude;
	float longitude;
	float aspect;
	float nearz;
	float farz;
	float sensitivity;
	HMM_Vec3 center;
	HMM_Vec3 eyepos;
	HMM_Mat4 view;
	HMM_Mat4 proj;
	HMM_Mat4 viewproj;
} sk_cam;

void sk_init_cam(sk_cam* cam, const sk_cam_desc* desc);
void sk_orbit_cam(sk_cam* cam, float dx, float dy);
void sk_zoom_cam(sk_cam* cam, float d);
void sk_update_cam(sk_cam* cam, int fb_width, int fb_height);
void sk_cam_input(sk_cam* cam, RGFW_window* w);

