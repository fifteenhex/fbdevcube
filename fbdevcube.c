#include <linux/fb.h>
#define S3L_PIXEL_FUNCTION	fbdevcube_pixel_func
#define S3L_MAX_PIXELS		(512 * 512)
#include "small3dlib/small3dlib.h"

#define FPS 30

static const S3L_Unit cube_vertices[] = { S3L_CUBE_VERTICES(S3L_F) };
static const S3L_Index cube_triangles[] = { S3L_CUBE_TRIANGLES };

static void *fb;
static unsigned int stride;

static unsigned short geometry[2];
/* The rect damaged by the last *render pass* (yes, it feels wrong writing that.. */
static unsigned short damage_rect[2][2];

static void init_damage_rect(void)
{
	damage_rect[0][0] = 0;
	damage_rect[0][1] = geometry[0];
	damage_rect[1][0] = 0;
	damage_rect[1][1] = geometry[1];
}

static void reset_damage_rect(void)
{
	damage_rect[0][0] = geometry[0];
	damage_rect[0][1] = 0;
	damage_rect[1][0] = geometry[1];
	damage_rect[1][1] = 0;
}

static int scale = 2;

static inline off_t start_of_line(unsigned short y)
{
	return (y * scale) * stride;
}

static inline off_t byte_in_line(unsigned short x)
{
	switch(scale) {
	case 1: return x >> 3;
	case 2: return x >> 2;
	}

	return 0;
}

static inline unsigned int bit_in_byte(unsigned short x)
{
	return 1 << (~x & 0x7);
}

static inline unsigned int twobits_in_byte(unsigned short x)
{
	return 0b11 << ((~x & 0x3) * 2);
}

static inline void fbdevcube_pixel_func(S3L_PixelInfo *p)
{
	unsigned int line = start_of_line(p->y);
	unsigned int byteinline = byte_in_line(p->x);
	unsigned int fboff = line + byteinline;

	/* Slide the damage rect x start out from the right */
	if (p->x < damage_rect[0][0])
		damage_rect[0][0] = p->x;

	/* Slide the damage rect x end out from the left */
	if (p->x > damage_rect[0][1])
		damage_rect[0][1] = p->x;

	/* Slide the damage rect y start out from the bottom */
	if (p->y < damage_rect[1][0])
		damage_rect[1][0] = p->y;

	/* Slide the damage rect y end out from the top */
	if (p->y > damage_rect[1][1])
		damage_rect[1][1] = p->y;

	switch(scale) {
	case 2:
		((uint8_t*)fb) [fboff] |= twobits_in_byte(p->x);
		((uint8_t*)fb) [fboff + stride] |= twobits_in_byte(p->x);
		break;
	case 1:
		((uint8_t*)fb) [fboff] |= bit_in_byte(p->x);
		break;
	}

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

	ret = ioctl(fbfd, FBIOBLANK, FB_BLANK_UNBLANK);
	if (ret) {
		printf("failed to unblank framebuffer: %d\n", ret);
		//return 1;
	}

	/* Stash the size of the framebuffer and reset the damage rect */
	geometry[0] = vscrinfo.xres / scale;
	geometry[1] = vscrinfo.yres / scale;
	init_damage_rect();

	/* Set S3L the resolution */
	S3L_resolutionX = geometry[0];
	S3L_resolutionY = geometry[1];

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
		/* Work out where we need to clear the framebuffer and do it */
		off_t damaged_line_start = start_of_line(damage_rect[1][0]);
		off_t damaged_line_end = start_of_line(damage_rect[1][1] + 1);
		size_t damage_sz = damaged_line_end - damaged_line_start;
		memset(fb + damaged_line_start, 0, damaged_line_end);
		//memset(fb, 0, framebuffersz);
		reset_damage_rect();

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

#if 0
		printf("damage rect %d:%d, %d:%d\n",
			(int) damage_rect[0][0],
			(int) damage_rect[0][1],
			(int) damage_rect[1][0],
			(int) damage_rect[1][1]);
#endif

		/* Limit the FPS */
		msleep(1000 / FPS);
	}

	return 0;
}
