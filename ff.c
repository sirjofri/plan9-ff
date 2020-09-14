#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

#define UINT16_MAX 65535
#define COLOR RGB24

ulong width, height;

Memimage *dst;

ushort ntohs(ushort a)
{
	u8int c[2];
	memcpy(c, &a, 2*sizeof(u8int));

	return ((ushort) c[1]<<0) | ((ushort) c[0]<<8);
}

int
readff(void)
{
	char magic[8];
	char wh[4];
	ushort rgba[4];
	int col[4];
	int c;
	Memimage *pix;

	if (read(0, magic, 8) != 8)
		return 0;

	if (strncmp(magic, "farbfeld", 8) != 0)
		return 0;

	if (read(0, wh, 8) != 8)
		return 0;

	width  = wh[0]<<24 | wh[1]<<16 | wh[2]<<8 | wh[3];
	height = wh[4]<<24 | wh[5]<<16 | wh[6]<<8 | wh[7];

	dst = allocmemimage(Rect(0, 0, width, height), COLOR);
	pix = allocmemimage(Rect(0, 0, 1, 1), COLOR);
	memfillcolor(dst, DWhite);

	for (int x=0; x<width; x++) {
		for (int y=0; y<height; y++) {
			if (read(0, rgba, sizeof(ushort)*4) != sizeof(ushort)*4)
				return 0;
			col[0] = (int)(((float)ntohs(rgba[0]) / UINT16_MAX) * 255);
			col[1] = (int)(((float)ntohs(rgba[1]) / UINT16_MAX) * 255);
			col[2] = (int)(((float)ntohs(rgba[2]) / UINT16_MAX) * 255);
			col[3] = (int)(((float)ntohs(rgba[3]) / UINT16_MAX) * 255);

			c  = col[0]<<8*3;
			c += col[1]<<8*2;
			c += col[2]<<8*1;
			memfillcolor(pix, c);
			memimagedraw(dst,
				rectaddpt(dst->r, (Point){x, y}),
				pix,
				(Point){0, 0},
				nil,
				(Point){0, 0},
				S);
		}
	}
	freememimage(pix);
	return 1;
}

void
main(void)
{
	if (memimageinit() != 0)
		sysfatal("memimageinit failed: %r");
	if (!readff()) {
		fprint(2, "Bad file format!\n");
		exits("usage");
	}

	if (writememimage(1, dst))
		sysfatal("cannot write: %r");
	freememimage(dst);
}
