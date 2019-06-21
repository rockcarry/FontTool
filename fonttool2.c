#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* BMP 对象的类型定义 */
typedef struct {
    int   width;   /* 宽度 */
    int   height;  /* 高度 */
    int   stride;  /* 行字节数 */
    void *pdata;   /* 指向数据 */
} BMP;

// 内部类型定义
typedef BYTE  uint8_t ;
typedef WORD  uint16_t;
typedef DWORD uint32_t;
#pragma pack(1)
typedef struct {
    uint16_t  bfType;
    uint32_t  bfSize;
    uint16_t  bfReserved1;
    uint16_t  bfReserved2;
    uint32_t  bfOffBits;
    uint32_t  biSize;
    uint32_t  biWidth;
    uint32_t  biHeight;
    uint16_t  biPlanes;
    uint16_t  biBitCount;
    uint32_t  biCompression;
    uint32_t  biSizeImage;
    uint32_t  biXPelsPerMeter;
    uint32_t  biYPelsPerMeter;
    uint32_t  biClrUsed;
    uint32_t  biClrImportant;
} BMPFILEHEADER;
#pragma pack()

/* 内部函数实现 */
static void bmp_create(BMP *pb, int w, int h)
{
    pb->width = w;
    pb->height= h;
    pb->stride=(w * 3 + 3) / 4 * 4;
    pb->pdata = malloc(pb->height * pb->stride);
}

static void bmp_destroy(BMP *pb)
{
    if (pb && pb->pdata) {
        free(pb->pdata);
        pb->pdata = NULL;
    }
}

static int bmp_save(BMP *pb, char *file)
{
    BMPFILEHEADER header = {0};
    int           stride = (pb->width * 3 + 3) / 4 * 4;
    FILE         *fp     = NULL;
    uint8_t      *pdata;
    int           i;

    header.bfType     = ('B' << 0) | ('M' << 8);
    header.bfSize     = sizeof(header) + stride * pb->height;
    header.bfOffBits  = sizeof(header);
    header.biSize     = 40;
    header.biWidth    = pb->width;
    header.biHeight   = pb->height;
    header.biPlanes   = 1;
    header.biBitCount = 24;
    header.biSizeImage= stride * pb->height;

    fp = fopen(file, "wb");
    if (fp) {
        fwrite(&header, sizeof(header), 1, fp);
        pdata = (uint8_t*)pb->pdata + pb->stride * pb->height;
        for (i=0; i<pb->height; i++) {
            pdata -= pb->stride;
            fwrite(pdata, stride, 1, fp);
        }
        fclose(fp);
    }

    return fp ? 0 : -1;
}

static int bmp_save16(BMP *pb, char *file)
{
    BMPFILEHEADER header = {0};
    int           stride = (pb->width * 2 + 3) / 4 * 4;
    FILE         *fp     = NULL;
    uint8_t      *pdata;
    int           r, g, b, i, j;
    uint16_t      c;
    uint32_t      mask;

    header.bfType        = ('B' << 0) | ('M' << 8);
    header.bfSize        = sizeof(header) + 3 * sizeof(uint32_t) + stride * pb->height;
    header.bfOffBits     = sizeof(header) + 3 * sizeof(uint32_t);
    header.biSize        = 40;
    header.biWidth       = pb->width;
    header.biHeight      = pb->height;
    header.biPlanes      = 1;
    header.biBitCount    = 16;
    header.biCompression = 3;
    header.biSizeImage   = stride * pb->height;

    fp = fopen(file, "wb");
    if (fp) {
        fwrite(&header, sizeof(header), 1, fp);
        mask  = 0x1f << 10; fwrite(&mask, sizeof(mask), 1, fp);
        mask  = 0x1f <<  5; fwrite(&mask, sizeof(mask), 1, fp);
        mask  = 0x1f <<  0; fwrite(&mask, sizeof(mask), 1, fp);
        pdata = (uint8_t*)pb->pdata + pb->stride * pb->height;
        for (i=0; i<pb->height; i++) {
            for (j=0; j<pb->width; j++) {
                b = *pdata++;
                g = *pdata++;
                r = *pdata++;
                c = ((r & 0x1f) << 10) | ((g & 0x1f) << 5) | ((b & 0x1f) << 0);
                fwrite(&c, sizeof(c), 1, fp);
            }
            for (j=pb->width*2; j<stride; j++) fputc(0, fp);
            pdata -= pb->width * 3;
            pdata -= pb->stride;
        }
        fclose(fp);
    }

    return fp ? 0 : -1;
}

static void bmp_setpixel(BMP *pb, int x, int y, int r, int g, int b)
{
    uint8_t *pbyte = pb->pdata;
    if (x >= pb->width || y >= pb->height) return;
    r = r < 0 ? 0 : r < 255 ? r : 255;
    g = g < 0 ? 0 : g < 255 ? g : 255;
    b = b < 0 ? 0 : b < 255 ? b : 255;
    pbyte[x * 3 + 0 + y * pb->stride] = r;
    pbyte[x * 3 + 1 + y * pb->stride] = g;
    pbyte[x * 3 + 2 + y * pb->stride] = b;
}

static void bmp_getpixel(BMP *pb, int x, int y, int *r, int *g, int *b)
{
    uint8_t *pbyte = pb->pdata;
    if (x >= pb->width || y >= pb->height) {
        *r = *g = *b = 0;
        return;
    }
    *r = pbyte[x * 3 + 0 + y * pb->stride];
    *g = pbyte[x * 3 + 1 + y * pb->stride];
    *b = pbyte[x * 3 + 2 + y * pb->stride];
}

static double color_distance(int r1, int g1, int b1, int r2, int g2, int b2)
{
    return sqrt((r1 - r2) * (r1 - r2) + (g1 - g2) * (g1 - g2) + (b1 - b2) * (b1 - b2));
}

int main(int argc, char *argv[])
{
    char        file[MAX_PATH];
    char        fontname[256] = "Arial";
    int         fontsize      = 16;
    int         fontweight    = 500;
    DWORD       fbkgcolor     = RGB(255, 0  , 255);
    DWORD       fontcolor     = RGB(255, 255, 255);
    DWORD       fedgcolor     = (DWORD)-1;
    int         distparam     = 288;
    HWND        hDeskTopWnd   = GetDesktopWindow();
    HDC         hDeskTopDC    = GetDC(hDeskTopWnd);
    HDC         hMemDC        = CreateCompatibleDC(hDeskTopDC);
    BITMAPINFO  bmpinfo       = {0};
    HBITMAP     hBitmap       = NULL;
    BYTE       *pbmpbuf       = NULL;
    BMP         mybmp1        = {0};
    BMP         mybmp2        = {0};
    HBRUSH      hBrush1       = NULL;
    HBRUSH      hBrush2       = NULL;
    LOGFONT     logfont       = {0};
    HFONT       hFont         = NULL;
    RECT        rect          = { 0, 0, 255, 255 };
    int         br, bb, bg, fr, fg, fb, er, eg, eb;
    int         x, y, i;
    char       *str = "0123456789-: ";

    if (argc < 2) {
        MessageBox(NULL, "usage:\n\nfonttool fontname fontsize fontweight bkcolor fontcolor edgecolor distparam\n", "FontTool", MB_OK);
    }

    if (argc >= 2) strcpy(fontname, argv[1]);
    if (argc >= 3) fontsize   = atoi(argv[2]);
    if (argc >= 4) fontweight = atoi(argv[3]);
    if (argc >= 5) fbkgcolor  = atoi(argv[4]);
    if (argc >= 6) fontcolor  = atoi(argv[5]);
    if (argc >= 7) fedgcolor  = atoi(argv[6]);
    if (argc >= 8) distparam  = atoi(argv[7]);

    br = (fbkgcolor >> 0 ) & 0xff;
    bg = (fbkgcolor >> 8 ) & 0xff;
    bb = (fbkgcolor >> 16) & 0xff;

    fr = (fontcolor >> 0 ) & 0xff;
    fg = (fontcolor >> 8 ) & 0xff;
    fb = (fontcolor >> 16) & 0xff;

    er = (fedgcolor >> 0 ) & 0xff;
    eg = (fedgcolor >> 8 ) & 0xff;
    eb = (fedgcolor >> 16) & 0xff;

    // bitmap
    bmpinfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmpinfo.bmiHeader.biWidth       = 256;
    bmpinfo.bmiHeader.biHeight      =-256;
    bmpinfo.bmiHeader.biPlanes      = 1;
    bmpinfo.bmiHeader.biBitCount    = 24;
    bmpinfo.bmiHeader.biCompression = BI_RGB;
    hBitmap = CreateDIBSection(hMemDC, &bmpinfo, DIB_RGB_COLORS, (void**)&pbmpbuf, NULL, 0);
    mybmp1.width = 256;
    mybmp1.height=(fedgcolor == (DWORD)-1) ? fontsize : fontsize + 2;
    mybmp1.stride= 256 * 3;
    mybmp1.pdata = pbmpbuf;
    bmp_create(&mybmp2, mybmp1.width, mybmp1.height * strlen(str));

    // brush
    hBrush1 = CreateSolidBrush(RGB(0 , 0 , 0 ));
    hBrush2 = CreateSolidBrush(RGB(bb, bg, br));

    // font
    strcpy(logfont.lfFaceName, fontname);
    logfont.lfHeight = fontsize ;
    logfont.lfWeight = fontweight;
    hFont = CreateFontIndirectA(&logfont);

    SelectObject(hMemDC, hBitmap);
    SelectObject(hMemDC, hFont  );
    SetBkMode   (hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, RGB(255, 255, 255));

    // fill background color
    for (y=0; y<mybmp2.height; y++) {
        for (x=0; x<mybmp2.width; x++) {
            bmp_setpixel(&mybmp2, x, y, br, bg, bb);
        }
    }

    for (i=0; str[i]; i++) {
        GetCharWidthA(hMemDC, str[i], str[i], &mybmp1.width);
        GetCharWidthA(hMemDC, str[i], str[i], &mybmp2.width);
        if (fedgcolor != (DWORD)-1) {
            mybmp1.width += 2;
            mybmp2.width += 2;
        }

        if (distparam == -1) {
            FillRect(hMemDC, &rect, hBrush2);
            if (fedgcolor != (DWORD)-1) {
                SetTextColor(hMemDC, RGB(eb, eg, er));
                TextOutA(hMemDC, 0, 0, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 1, 0, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 2, 0, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 0, 1, (LPCSTR)&(str[i]), 1);
//              TextOutA(hMemDC, 1, 1, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 2, 1, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 0, 2, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 1, 2, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 2, 2, (LPCSTR)&(str[i]), 1);
            }

            SetTextColor(hMemDC, RGB(fb, fg, fr));
            if (fedgcolor != (DWORD)-1) {
                TextOutA(hMemDC, 1, 1, (LPCSTR)&(str[i]), 1);
            } else {
                TextOutA(hMemDC, 0, 0, (LPCSTR)&(str[i]), 1);
            }

            for (y=0; y<mybmp1.height; y++) {
                for (x=0; x<mybmp1.width; x++) {
                    int r, g, b;
                    bmp_getpixel(&mybmp1, x, y, &r, &g, &b);
                    bmp_setpixel(&mybmp2, x, y + i * mybmp1.height, r, g, b);
                }
            }
        } else {
            if (fedgcolor != (DWORD)-1) {
                FillRect(hMemDC, &rect, hBrush1);
                TextOutA(hMemDC, 0, 0, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 1, 0, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 2, 0, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 0, 1, (LPCSTR)&(str[i]), 1);
//              TextOutA(hMemDC, 1, 1, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 2, 1, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 0, 2, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 1, 2, (LPCSTR)&(str[i]), 1);
                TextOutA(hMemDC, 2, 2, (LPCSTR)&(str[i]), 1);
                for (y=0; y<mybmp1.height; y++) {
                    for (x=0; x<mybmp1.width; x++) {
                        int r, g, b;
                        bmp_getpixel(&mybmp1, x, y, &r, &g, &b);
                        if (color_distance(r, g, b, 0, 0, 0) > distparam) {
                            bmp_setpixel(&mybmp2, x, y + i * mybmp1.height, er, eg, eb);
                        }
                    }
                }
            }

            FillRect(hMemDC, &rect, hBrush1);
            if (fedgcolor != (DWORD)-1) {
                TextOutA(hMemDC, 1, 1, (LPCSTR)&(str[i]), 1);
            } else {
                TextOutA(hMemDC, 0, 0, (LPCSTR)&(str[i]), 1);
            }
            for (y=0; y<mybmp1.height; y++) {
                for (x=0; x<mybmp1.width; x++) {
                    int r, g, b;
                    bmp_getpixel(&mybmp1, x, y, &r, &g, &b);
                    if (color_distance(r, g, b, 0, 0, 0) > distparam) {
                        bmp_setpixel(&mybmp2, x, y + i * mybmp1.height, fr, fg, fb);
                    }
                }
            }
        }
    }

    if (hMemDC ) DeleteDC(hMemDC);
    if (hBitmap) DeleteObject(hBitmap);
    if (hFont  ) DeleteObject(hFont  );
    if (hBrush1) DeleteObject(hBrush1);
    if (hBrush2) DeleteObject(hBrush2);
    sprintf(file, "font%d.bmp", fontsize);
    bmp_save(&mybmp2, file);
    bmp_destroy(&mybmp2);
    return 0;
}
