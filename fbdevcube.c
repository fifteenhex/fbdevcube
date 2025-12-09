#include <linux/fb.h>
#define S3L_PIXEL_FUNCTION	fbdevcube_pixel_func
#define S3L_MAX_PIXELS		(512 * 512)
#include "small3dlib/small3dlib.h"

#define FPS 30

static const S3L_Unit cube_vertices[] = { S3L_CUBE_VERTICES(S3L_F) };
static const S3L_Index cube_triangles[] = { S3L_CUBE_TRIANGLES };

static void *fb;
static unsigned int stride;

static inline void fbdevcube_pixel_func(S3L_PixelInfo *p)
{
	unsigned int bit = ~p->x % 8;

	((uint8_t*)fb) [(p->y * stride) + (p->x / 8)] |= (1 << bit);
}

int main(int argc, char **argv, char **envp)
{
	struct fb_var_screeninfo vscrinfo;
	const char *fbdev_path = "/dev/fb0";
	size_t framebuffersz;
	int fbfd, ret;

	printf("fbdevcube (%s)\n", __TIME__);

	fbfd = open(fbdev_path, O_RDWR);
	if (fbfd < 0) {
		printf("failed to open fbdev %s: %d\n", fbdev_path, fbfd);
		return 1;
	}

	ret = ioctl(fbfd, FBIOGET_VSCREENINFO, &vscrinfo);
	if (ret) {
		printf("failed to get var screeninfo: %d\n", ret);
		return 1;
	}

	stride = (vscrinfo.xres * vscrinfo.bits_per_pixel) / 8;
	framebuffersz = ((vscrinfo.xres * vscrinfo.yres) * vscrinfo.bits_per_pixel) / 8;
	printf("framebuffer is %d x %d @ %d bpp, %d bytes\n",
		vscrinfo.xres, vscrinfo.yres, vscrinfo.bits_per_pixel, (unsigned) framebuffersz);

	fb = mmap(0, framebuffersz, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (fb == MAP_FAILED) {
		printf("failed to map framebuffer.\n");
		return 1;
	}

	printf("framebuffer mapped to 0x%lx\n", (unsigned long) fb);

	/* Set the resolution, we might use dithering or something later... */
	S3L_resolutionX = vscrinfo.xres;
	S3L_resolutionY = vscrinfo.yres;

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
	scene.camera.transform.translation.y = S3L_F / 2;

	while (1) {
		memset(fb, 0, framebuffersz);

		cube_model.transform.rotation.x += 4;

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
	}

	return 0;
}
