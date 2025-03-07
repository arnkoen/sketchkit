#include "sk.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"
#define M3D_IMPLEMENTATION
#include "m3d.h"

#include <stdio.h>
#include <assert.h>

#define SK_DEF(val, def) (val == 0 ? def : val)
#define SK_DEFAULT_WIDTH 800
#define SK_DEFAULT_HEIGHT 600
#define SK_DEFAULT_TITLE "sketch"

RGFW_window* sk_init(const sk_desc* desc) {
	RGFW_window* win = RGFW_createWindow(
		SK_DEF(desc->title, SK_DEFAULT_TITLE),
		RGFW_RECT(20, 20, SK_DEF((i32)desc->width, SK_DEFAULT_WIDTH), SK_DEF((i32)desc->height, SK_DEFAULT_HEIGHT)),
		SK_DEF(desc->flags, 0)
    );

	bgfx_init_t init = { 0 };
	bgfx_init_ctor(&init);

	init.platformData.nwh = win->src.window;
	init.type = SK_DEF(desc->renderer, BGFX_RENDERER_TYPE_COUNT);
	init.resolution.width = SK_DEF(desc->width, SK_DEFAULT_WIDTH);
	init.resolution.height = SK_DEF(desc->height, SK_DEFAULT_HEIGHT);
	init.resolution.reset = desc->reset;
	bgfx_init(&init);
	bgfx_reset((uint32_t)win->r.w, (uint32_t)win->r.h, desc->reset, init.resolution.format);
	return win;
}

void sk_shutdown(RGFW_window* window) {
	bgfx_shutdown();
	RGFW_window_close(window);
}


//--FILE-READING----------------------------------------------------


const bgfx_memory_t* sk_read_file(const char* path) {
	FILE* file = fopen(path, "rb");
	if(file == NULL) {
		fprintf(stderr, "Could not open file : %s\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(fileSize + 1);
	if (buffer == NULL) {
		fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
		exit(74);
	}
	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	if (bytesRead < fileSize) {
		fprintf(stderr, "Could not read file \"%s\".\n", path);
		exit(74);
	}
	buffer[bytesRead] = '\0';

	const bgfx_memory_t* ret = bgfx_copy((const void*)buffer, (uint32_t)fileSize);
	free(buffer);

	fclose(file);
	return ret;
}

bgfx_shader_handle_t sk_load_shader(const char* path) {
	const bgfx_memory_t* src = sk_read_file(path);
	return bgfx_create_shader(src);
}

bgfx_program_handle_t sk_load_program(const char* vs_path, const char* fs_path) {
    bgfx_shader_handle_t vs = sk_load_shader(vs_path);
    bgfx_shader_handle_t fs = sk_load_shader(fs_path);
    return bgfx_create_program(vs, fs, true);
}

//--IMAGE-----------------------------------------------------------

static const uint32_t _checker_pixels[4 * 4] = {
    0xFFAAAAAA, 0xFF555555, 0xFFAAAAAA, 0xFF555555,
    0xFF555555, 0xFFAAAAAA, 0xFF555555, 0xFFAAAAAA,
    0xFFAAAAAA, 0xFF555555, 0xFFAAAAAA, 0xFF555555,
    0xFF555555, 0xFFAAAAAA, 0xFF555555, 0xFFAAAAAA,
};

#define ENDS_WITH(S, E) (strcmp(S + strlen(S) - (sizeof(E)-1), E) == 0)

sk_image_data sk_load_image_data(const char* path) {
    sk_image_data ret = { 0 };
    if (ENDS_WITH(path, ".qoi")) {
        qoi_desc qoi = { 0 };
        ret.pixels = (void*)qoi_read(path, &qoi, 4);
        ret.w = qoi.width;
        ret.h = qoi.height;
    }
    return ret;
}

void sk_release_image_data(sk_image_data* data) {
    if (data->pixels) free(data->pixels);
    data->pixels = NULL;
    data->w = 0;
    data->h = 0;
}

bgfx_texture_handle_t sk_checker_texture(void) {
	const bgfx_memory_t* mem = bgfx_copy((const void*)&_checker_pixels, 4 * 4 * 4);
	return bgfx_create_texture_2d(4, 4, false, 0, BGFX_TEXTURE_FORMAT_RGBA8, 0, mem);
}

bgfx_texture_handle_t sk_load_texture(const char* path) {
    sk_image_data data = sk_load_image_data(path);
    bgfx_texture_handle_t ret = sk_load_texture_mem(&data);
    sk_release_image_data(&data);
    return ret;
}

bgfx_texture_handle_t sk_load_texture_mem(sk_image_data* data) {
    if (data->pixels != NULL) {
        const bgfx_memory_t* mem = bgfx_copy((const void*)data->pixels, data->w * data->h * 4);
        return bgfx_create_texture_2d((uint16_t)data->w, (uint16_t)data->w, false, 0, BGFX_TEXTURE_FORMAT_RGBA8, 0, mem);
    }
    else {
        return sk_checker_texture();
    }
}

//--MESH------------------------------------------------------------

static bgfx_vertex_layout_t _pnt_layout(void) {
    bgfx_vertex_layout_t ret = { 0 };
    bgfx_vertex_layout_begin(&ret, bgfx_get_renderer_type());
    bgfx_vertex_layout_add(&ret, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
    bgfx_vertex_layout_add(&ret, BGFX_ATTRIB_NORMAL, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
    bgfx_vertex_layout_add(&ret, BGFX_ATTRIB_TEXCOORD0, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
    bgfx_vertex_layout_end(&ret);
    return ret;
}

sk_mesh sk_load_m3d(const char* path) {
    const bgfx_memory_t* mem = sk_read_file(path);
    assert(mem->data != NULL && mem->size > 0);
    m3d_t* m3d = m3d_load((unsigned char*)mem->data, NULL, NULL, NULL);
    assert(m3d);
    
    uint32_t total_vertices = m3d->numface * 3;

    sk_vertex_pnt* pnt = malloc(total_vertices * sizeof(sk_vertex_pnt));
    assert(pnt);
    uint32_t* indices = malloc(sizeof(uint32_t) * total_vertices);
    assert(indices);

    for (uint32_t i = 0; i < m3d->numface; i++) {
        m3d_face_t* face = &m3d->face[i];
        for (int n = 0; n < 3; n++) {
            uint32_t vertIdx = face->vertex[n];
            uint32_t outIndex = i * 3 + n;

            sk_vertex_pnt vert = { 0 };
            m3d_vertex_t* m3dVert = &m3d->vertex[vertIdx];

            vert.pos.X = m3dVert->x;
            vert.pos.Y = m3dVert->y;
            vert.pos.Z = m3dVert->z;

            if (face->normal[0] != M3D_UNDEF) {
                // Using the normal index from the face. (Assuming face->normal[n] is valid.)
                m3d_vertex_t* normVert = &m3d->vertex[face->normal[n]];
                vert.norm.X = normVert->x;
                vert.norm.Y = normVert->y;
                vert.norm.Z = normVert->z;
            }
            else {
                vert.norm = HMM_V3(0, 0, 0);
            }

            if (face->texcoord[0] != M3D_UNDEF) {
                // Raylib flips the V coordinate.
                uint32_t tcIdx = face->texcoord[n];
                if (tcIdx < m3d->numtmap) {
                    vert.uv.X = m3d->tmap[tcIdx].u;
                    vert.uv.Y = 1.0f - m3d->tmap[tcIdx].v;
                }
                else {
                    vert.uv = HMM_V2(0, 0);
                }
            }
            else {
                vert.uv = HMM_V2(0, 0);
            }

            pnt[outIndex] = vert;
            indices[outIndex] = outIndex;
        }
    }

    sk_mesh mesh = { 0 };
    bgfx_vertex_layout_t layout = _pnt_layout();

    mesh.vbuf = bgfx_create_vertex_buffer(
        bgfx_make_ref(pnt, total_vertices * sizeof(sk_vertex_pnt)),
        &layout,
        BGFX_BUFFER_NONE
    );

    mesh.ibuf = bgfx_create_index_buffer(
        bgfx_make_ref(indices, total_vertices * sizeof(uint32_t)),
        BGFX_BUFFER_INDEX32
    );

    mesh.element_count = total_vertices;
    return mesh;
}


void sk_release_mesh(sk_mesh* mesh) {
    bgfx_destroy_vertex_buffer(mesh->vbuf);
    bgfx_destroy_index_buffer(mesh->ibuf);
    mesh->element_count = 0;
}

void sk_set_mesh(const sk_mesh* mesh) {
    bgfx_set_vertex_buffer(0, mesh->vbuf, 0, mesh->element_count);
    bgfx_set_index_buffer(mesh->ibuf, 0, mesh->element_count);
}

//--CAMERA----------------------------------------------------------

#define CAMERA_DEFAULT_MIN_DIST (2.0f)
#define CAMERA_DEFAULT_MAX_DIST (50.0f)
#define CAMERA_DEFAULT_MIN_LAT (-85.0f)
#define CAMERA_DEFAULT_MAX_LAT (85.0f)
#define CAMERA_DEFAULT_DIST (10.0f)
#define CAMERA_DEFAULT_ASPECT (60.0f)
#define CAMERA_DEFAULT_NEARZ (1.0f)
#define CAMERA_DEFAULT_FARZ (1000.0f)
#define CAMERA_DEFAULT_SENSITIVITY (30.0f)

static float _cam_def(float val, float def) { return ((val == 0.0f) ? def : val); }

static HMM_Vec3 _cam_euclidean(float latitude, float longitude) {
    const float lat = latitude * HMM_DegToRad;
    const float lng = longitude * HMM_DegToRad;
    return HMM_V3(cosf(lat)* sinf(lng), sinf(lat), cosf(lat)* cosf(lng));
}

void sk_init_cam(sk_cam* cam, const sk_cam_desc* desc) {
    assert(desc && cam);
    cam->mindist = _cam_def(desc->mindist, CAMERA_DEFAULT_MIN_DIST);
    cam->maxdist = _cam_def(desc->maxdist, CAMERA_DEFAULT_MAX_DIST);
    cam->minlat = _cam_def(desc->minlat, CAMERA_DEFAULT_MIN_LAT);
    cam->maxlat = _cam_def(desc->maxlat, CAMERA_DEFAULT_MAX_LAT);
    cam->distance = _cam_def(desc->distance, CAMERA_DEFAULT_DIST);
    cam->center = desc->center;
    cam->latitude = desc->latitude;
    cam->longitude = desc->longitude;
    cam->aspect = _cam_def(desc->aspect, CAMERA_DEFAULT_ASPECT);
    cam->nearz = _cam_def(desc->nearz, CAMERA_DEFAULT_NEARZ);
    cam->farz = _cam_def(desc->farz, CAMERA_DEFAULT_FARZ);
    cam->sensitivity = _cam_def(desc->sensitivity, CAMERA_DEFAULT_SENSITIVITY);
}

void sk_orbit_cam(sk_cam* cam, float dx, float dy) {
    assert(cam);
    cam->longitude -= dx;
    if (cam->longitude < 0.0f) {
        cam->longitude += 360.0f;
    }
    if (cam->longitude > 360.0f) {
        cam->longitude -= 360.0f;
    }
    cam->latitude = HMM_Clamp(cam->minlat, cam->latitude + dy, cam->maxlat);
}

void sk_zoom_cam(sk_cam* cam, float d) {
    assert(cam);
    cam->distance = HMM_Clamp(cam->mindist, cam->distance + d, cam->maxdist);
}

void sk_update_cam(sk_cam* cam, int fb_width, int fb_height) {
    assert(cam);
    const float w = (float)fb_width;
    const float h = (float)fb_height;
    
    cam->eyepos =  HMM_AddV3(cam->center, HMM_MulV3F(_cam_euclidean(cam->latitude, cam->longitude), cam->distance));
    cam->view = HMM_LookAt_RH(cam->eyepos, cam->center, HMM_V3(0.0f, 1.0f, 0.0f));
    cam->proj = HMM_Perspective_RH_NO(cam->aspect * HMM_DegToRad, w / h, 0.1f, 1000.f);
    cam->viewproj = HMM_MulM4(cam->proj, cam->view);
}

void sk_cam_input(sk_cam* cam, RGFW_window* w) {
    assert(cam);
	const RGFW_event* ev = &w->event;
    switch (ev->type) {
    case RGFW_mouseButtonPressed:
        if (ev->button == RGFW_mouseRight) {
			RGFW_window_mouseHold(w, RGFW_AREA(0, 0));
			RGFW_window_showMouse(w, false);
        }
        break;
    case RGFW_mouseButtonReleased:
        if (ev->button == RGFW_mouseRight) {
			RGFW_window_mouseUnhold(w);
			RGFW_window_showMouse(w, true);
        }
        break;
    case RGFW_mousePosChanged:
        if (RGFW_window_mouseHidden(w) && ev->button == RGFW_mouseRight) {
            sk_orbit_cam(cam, ev->vector.x* 0.25f, ev->vector.y* 0.25f);
        }
        break;
    default:
        break;
    }
	//sk_zoom_cam(cam, (float)ev->scroll);
}