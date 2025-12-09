#include <linux/fb.h>
#define S3L_PIXEL_FUNCTION	fbdevcube_pixel_func
#define S3L_MAX_PIXELS		(512 * 512)
#include "small3dlib/small3dlib.h"

static inline void fbdevcube_pixel_func(S3L_PixelInfo *p)
{

}

int main(int argc, char **argv, char **envp)
{
	struct fb_var_screeninfo vscrinfo;
	const char *fbdev_path = "/dev/fb0";
	int fbfd, ret;
	void *fb;
	size_t framebuffersz;

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

	framebuffersz = ((vscrinfo.xres * vscrinfo.yres) * vscrinfo.bits_per_pixel) / 8;
	printf("framebuffer is %d x %d @ %d bpp, %d bytes\n",
		vscrinfo.xres, vscrinfo.yres, vscrinfo.bits_per_pixel, (unsigned) framebuffersz);

	fb = mmap(0, framebuffersz, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (fb == MAP_FAILED) {
		printf("failed to map framebuffer.\n");
		return 1;
	}

	printf("framebuffer mapped to 0x%lx\n", (unsigned long) fb);

	return 0;
}
