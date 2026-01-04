#define S3L_PIXEL_FUNCTION	fbdevcube_pixel_func
#define S3L_MAX_PIXELS		(512 * 512)
#include "small3dlib/small3dlib.h"

#define FPS 30

#define OPTIONAL_ARGS \
	OPTIONAL_UINT_ARG(scale, 1, "-s", "scale", "Scale factor")

#define BOOLEAN_ARGS \
	BOOLEAN_ARG(help, "-h", "Show help")

#include <easyargs.h>
#define ARRAY_SIZE(_a) (sizeof(_a) / sizeof(_a[0]))
#include <fbdevgl.h>

static const S3L_Unit cube_vertices[] = { S3L_CUBE_VERTICES(S3L_F) };
static const S3L_Index cube_triangles[] = { S3L_CUBE_TRIANGLES };

static struct fbdevgl_context fbinfo = { 0 };

static inline void fbdevcube_pixel_func(S3L_PixelInfo *p)
{
	fbdevgl_set_pixel(&fbinfo, p->x, p->y, p->triangleIndex / 2);
}

#if 0
static int _parse_args(int argc, char **argv)
{
	args_t args = make_default_args();

	if (!parse_args(argc, argv, &args) || args.help) {
		print_help(argv[0]);
		return -1;
	}

	return 0;
}
#endif

int main(int argc, char **argv, char **envp)
{
	const char *fbdev_path = "/dev/fb0";
	size_t framebuffersz;
	int fbfd, ret;

	printf("fbdevcube (%s)\n", __TIME__);

//	ret = _parse_args(argc, argv);
//	if (ret)
//		return 1;

	ret = fbdevgl_init(fbdev_path, &fbinfo);
	if (ret)
		return 1;

	ret = ioctl(fbfd, FBIOBLANK, FB_BLANK_UNBLANK);
	if (ret) {
		printf("failed to unblank framebuffer: %d\n", ret);
		//return 1;
	}

	/* Set S3L the resolution */
	S3L_resolutionX = fbinfo.geometry[0];
	S3L_resolutionY = fbinfo.geometry[1];

	/* Setup the cube */
	S3L_Model3D cube_model;
	S3L_model3DInit(cube_vertices,
		 S3L_CUBE_VERTEX_COUNT,
		 cube_triangles,
		 S3L_CUBE_TRIANGLE_COUNT,
		 &cube_model);

	// scene of 3D models (we only have 1)
	S3L_Scene scene;
	S3L_sceneInit(&cube_model,1,&scene);

	// shift the camera a little bit so that we can see the triangle
	scene.camera.transform.translation.z = -2 * S3L_F;
	scene.camera.transform.translation.y = S3L_F / 4;

	while (1) {
		fbdevgl_reset_damage_rect(&fbinfo);

		cube_model.transform.rotation.x += 4;
		cube_model.transform.rotation.y += 4;

		// has to be called before each frame
		S3L_newFrame();
		/*
		 * this starts the scene rendering, the library
		 * will now start calling our drawPixel function to
		 * render the camera view
		 */
		S3L_drawScene(scene);

		/* Limit the FPS */
		msleep(1000 / FPS);

		fbdevgl_clear_damaged_area(&fbinfo);
	}

	return 0;
}
