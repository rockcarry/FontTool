#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct {
    int   width;   /* 宽度 */
    int   height;  /* 高度 */
    int   stride;  /* 行字节数 */
    void *pdata;   /* 指向数据 */
} BMP;

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

static void bmp_create(BMP *pb, int w, int h)
{
    pb->width = w;
    pb->height= h;
    pb->stride=(w * 3 + 3) / 4 * 4;
    pb->pdata = calloc(1, pb->height * pb->stride);
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
        for (i = 0; i < pb->height; i++) {
            pdata -= pb->stride;
            fwrite(pdata, stride, 1, fp);
        }
        fclose(fp);
    }

    return fp ? 0 : -1;
}

static void bmp_setpixel(BMP *pb, int x, int y, int r, int g, int b)
{
    uint8_t *pbyte = pb->pdata;
    if (x < 0 || x >= pb->width || y < 0 || y >= pb->height) return;
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
    if (x < 0 || x >= pb->width || y < 0 || y >= pb->height) {
        *r = *g = *b = 0; return;
    }
    *r = pbyte[x * 3 + 0 + y * pb->stride];
    *g = pbyte[x * 3 + 1 + y * pb->stride];
    *b = pbyte[x * 3 + 2 + y * pb->stride];
}

static void bmp_bitblt(BMP *dstpb, int dstx, int dsty, int w, int h, BMP *srcpb, int srcx, int srcy)
{
    int  i, j, r, g, b;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            bmp_getpixel(srcpb, srcx + j, srcy + i, &r, &g, &b);
            bmp_setpixel(dstpb, dstx + j, dsty + i,  r,  g,  b);
        }
    }
}

static float color_distance(int r1, int g1, int b1, int r2, int g2, int b2)
{
    return sqrtf((r1 - r2) * (r1 - r2) + (g1 - g2) * (g1 - g2) + (b1 - b2) * (b1 - b2));
}

static void bmp_binaryzation(BMP *pb, int distparam)
{
    int x, y, r, g, b;
    for (y = 0; y < pb->height; y++) {
        for (x = 0; x < pb->width; x++) {
            bmp_getpixel(pb, x, y, &r, &g, &b);
            if (color_distance(r, g, b, 255, 255, 255) < distparam) {
                bmp_setpixel(pb, x, y, 255, 255, 255);
            } else {
                bmp_setpixel(pb, x, y, 0, 0, 0);
            }
        }
    }
}

static int get_text_width(HDC hdc, char *str)
{
    RECT rect = {};
    DrawText(hdc, str, -1, &rect, DT_CALCRECT);
    return rect.right;
}

#ifdef _TOOL1_
static void bmp_dilation(BMP *pb)
{
    static const uint8_t filter[9] = { 0, 1, 0, 1, 1, 1, 0, 1, 0 };
    uint8_t data[9];
    int     x, y, i;
    BMP     bmp = {};
    bmp_create(&bmp, pb->width, pb->height);
    for (y = 0; y < pb->height; y++) {
        for (x = 0; x < pb->width; x++) {
            int r, g, b;
            bmp_getpixel(pb, x - 1, y - 1, &r, &g, &b); data[0] = r;
            bmp_getpixel(pb, x + 0, y - 1, &r, &g, &b); data[1] = r;
            bmp_getpixel(pb, x + 1, y - 1, &r, &g, &b); data[2] = r;
            bmp_getpixel(pb, x - 1, y - 0, &r, &g, &b); data[3] = r;
            bmp_getpixel(pb, x + 0, y - 0, &r, &g, &b); data[4] = r;
            bmp_getpixel(pb, x + 1, y - 0, &r, &g, &b); data[5] = r;
            bmp_getpixel(pb, x - 1, y + 1, &r, &g, &b); data[6] = r;
            bmp_getpixel(pb, x + 0, y + 1, &r, &g, &b); data[7] = r;
            bmp_getpixel(pb, x + 1, y + 1, &r, &g, &b); data[8] = r;
            for (i = 0; i < 9; i++) {
                if (filter[i] && data[i]) {
                    bmp_setpixel(&bmp, x, y, 255, 255, 255);
                    break;
                }
            }
        }
    }
    bmp_bitblt(pb, 0, 0, pb->width, pb->height, &bmp, 0, 0);
    bmp_destroy(&bmp);
}

static char *s_full_charset = " 0123456789-:星期一二三四五六日!\"#$%&'()*+,-./;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

int main(int argc, char *argv[])
{
    char        file[MAX_PATH];
    char       *fontstr       = " 0123456789-:星期一二三四五六日";
    char        fontname[256] = "Arial";
    int         fontsize      = 16;
    int         fontweight    = 500;
    int         edgesize      = 0;
    DWORD       fbkgcolor     = RGB(255, 0  , 255);
    DWORD       fontcolor     = RGB(255, 255, 255);
    DWORD       fedgcolor     = RGB(0  , 0  , 0  );
    int         distparam     = 288;
    HWND        hDeskTopWnd   = GetDesktopWindow();
    HDC         hDeskTopDC    = GetDC(hDeskTopWnd);
    HDC         hMemDC        = CreateCompatibleDC(hDeskTopDC);
    BITMAPINFO  bmpinfo       = {0};
    HBITMAP     hBitmap       = NULL;
    BYTE       *pbmpbuf       = NULL;
    BMP         mybmp1        = {0};
    BMP         mybmp2        = {0};
    BMP         mybmp3        = {0};
    LOGFONT     logfont       = {0};
    HFONT       hFont         = NULL;
    RECT        rect          = { 0, 0, 255, 255 };
    int         br, bb, bg, fr, fg, fb, er, eg, eb;
    int         x, y, i, j, n;

    if (argc <  2) MessageBox(NULL, "usage:\n\nfonttool fontname fontsize fontweight bkcolor fontcolor edgesize edgecolor distparam\n", "FontTool", MB_OK);
    if (argc >= 2) strcpy(fontname, argv[1]);
    if (argc >= 3) fontsize   = atoi(argv[2]);
    if (argc >= 4) fontweight = atoi(argv[3]);
    if (argc >= 5) fbkgcolor  = atoi(argv[4]);
    if (argc >= 6) fontcolor  = atoi(argv[5]);
    if (argc >= 7) edgesize   = atoi(argv[6]);
    if (argc >= 8) fedgcolor  = atoi(argv[7]);
    if (argc >= 9) distparam  = atoi(argv[8]);
    if (argc >=10) fontstr    = argv[9];
    if (strcmp(fontstr, "full") == 0) fontstr = s_full_charset;

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
    mybmp1.height= fontsize + 2 * edgesize;
    mybmp1.stride= 256 * 3;
    mybmp1.pdata = pbmpbuf;
    bmp_create(&mybmp2, mybmp1.width, mybmp1.height);
    bmp_create(&mybmp3, mybmp1.width, mybmp1.height);

    // font
    strcpy(logfont.lfFaceName, fontname);
    logfont.lfHeight = fontsize ;
    logfont.lfWeight = fontweight;
    hFont = CreateFontIndirectA(&logfont);

    SelectObject(hMemDC, hBitmap);
    SelectObject(hMemDC, hFont  );
    SetBkMode   (hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, RGB(255, 255, 255));

    for (i = 0, n = 0; fontstr[i]; n++) {
        int  chinese = (unsigned)fontstr[i] > 0xA0;
        char str[3]  = { (unsigned)fontstr[i] };
        if (chinese && i + 1 < strlen(fontstr)) str[1] = (unsigned)fontstr[i + 1];
        i += chinese ? 2 : 1;
        if (str[0] != '&') mybmp1.width = mybmp2.width = mybmp3.width = get_text_width(hMemDC, str) + 2 * edgesize;

        FillRect(hMemDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        TextOutA(hMemDC, edgesize, edgesize, str, strlen(str));

        bmp_binaryzation(&mybmp1, distparam);
        bmp_bitblt(&mybmp2, 0, 0, mybmp2.width, mybmp2.height, &mybmp1, 0, 0);
        for (j = 0; j < edgesize; j++) bmp_dilation(&mybmp2);
        for (y = 0; y < mybmp1.height; y++) {
            for (x = 0; x < mybmp1.width; x++) {
                int f, e;
                bmp_getpixel(&mybmp1, x, y, &f, &f, &f);
                bmp_getpixel(&mybmp2, x, y, &e, &e, &e);
                if      (f) bmp_setpixel(&mybmp3, x, y, fr, fg, fb);
                else if (e) bmp_setpixel(&mybmp3, x, y, er, eg, eb);
                else        bmp_setpixel(&mybmp3, x, y, br, bg, bb);
            }
        }

        sprintf(file, "font_%02d_%03d.bmp", mybmp3.height, n);
        bmp_save(&mybmp3, file);
    }

    bmp_destroy(&mybmp2);
    bmp_destroy(&mybmp3);
    if (hMemDC ) DeleteDC(hMemDC);
    if (hBitmap) DeleteObject(hBitmap);
    if (hFont  ) DeleteObject(hFont  );
    return 0;
}
#endif

#ifdef _TOOL2_
#include <iconv.h>
static void string_charset_convert(char *dstbuf, size_t dstlen, char *dstcharset, char *srcbuf, size_t srclen, char *srccharset)
{
    iconv_t hiconv = iconv_open(dstcharset, srccharset);
    if (hiconv) {
        iconv(hiconv, &srcbuf, &srclen, &dstbuf, &dstlen);
        iconv_close(hiconv);
    }
}

#define GB2313_TOTAL_QU  87
#define GB2313_TOTAL_WEI 94
int gb2312_qu_to_byte0 (int qu ) { return qu  + 0xA1; }
int gb2312_wei_to_byte1(int wei) { return wei + 0xA1; }

#define BIG5_TOTAL_QU  87
#define BIG5_TOTAL_WEI 157
int big5_qu_to_byte0 (int qu ) { return qu  < 75 ? qu  + 0xA1 : qu  - 75 + 0xC9; }
int big5_wei_to_byte1(int wei) { return wei < 63 ? wei + 0x40 : wei - 63 + 0xA1; }

static int bmp_to_bits_array(BMP *pb, uint8_t *buf, int len)
{
    int total_bits = (pb->width * pb->height + 7) & ~7;
    int byte_offset, bit_offset, current_bit, x, y, c;
    if (len < total_bits / 8) {
        printf("insufficient buffer size !\n");
        return -1;
    }
    memset(buf, 0, total_bits / 8);
    for (y = 0; y < pb->height; y++) {
        for (x = 0; x < pb->width; x++) {
            bmp_getpixel(pb, x, y, &c, &c, &c);
            current_bit = y * pb->width + x;
            byte_offset = current_bit / 8;
            bit_offset  = current_bit % 8;
            if (c) buf[byte_offset] |= (1 << bit_offset);
        }
    }
    return total_bits / 8;
}

int main(int argc, char *argv[])
{
    char        file[MAX_PATH];
    char        fontname[256] = "Arial";
    int         fontsize      = 16;
    int         fontweight    = 500;
    int         distparam     = 288;
    HWND        hDeskTopWnd   = GetDesktopWindow();
    HDC         hDeskTopDC    = GetDC(hDeskTopWnd);
    HDC         hMemDC        = CreateCompatibleDC(hDeskTopDC);
    BITMAPINFO  bmpinfo       = {0};
    HBITMAP     hBitmap       = NULL;
    BYTE       *pbmpbuf       = NULL;
    BMP         mybmp         = {0};
    LOGFONT     logfont       = {0};
    HFONT       hFont         = NULL;
    RECT        rect          = { 0, 0, 255, 255 };
    uint8_t     buf[32 * 256] = {0};
    FILE       *fp            = NULL;
    char       *charset       = "gb2312";
    int         i, j, n;

    if (argc <  2) MessageBox(NULL, "usage:\n\nfonttool fontname fontsize fontweight distparam charset\n", "FontTool", MB_OK);
    if (argc >= 2) strcpy(fontname, argv[1]);
    if (argc >= 3) fontsize   = atoi(argv[2]);
    if (argc >= 4) fontweight = atoi(argv[3]);
    if (argc >= 5) distparam  = atoi(argv[4]);
    if (argc >= 6) charset    = argv[5];

    // bitmap
    bmpinfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmpinfo.bmiHeader.biWidth       = 256;
    bmpinfo.bmiHeader.biHeight      =-256;
    bmpinfo.bmiHeader.biPlanes      = 1;
    bmpinfo.bmiHeader.biBitCount    = 24;
    bmpinfo.bmiHeader.biCompression = BI_RGB;
    hBitmap = CreateDIBSection(hMemDC, &bmpinfo, DIB_RGB_COLORS, (void**)&pbmpbuf, NULL, 0);
    mybmp.width = 256;
    mybmp.height= fontsize;
    mybmp.stride= 256 * 3;
    mybmp.pdata = pbmpbuf;

    // font
    strcpy(logfont.lfFaceName, fontname);
    logfont.lfHeight = fontsize ;
    logfont.lfWeight = fontweight;
    hFont = CreateFontIndirectA(&logfont);

    SelectObject(hMemDC, hBitmap);
    SelectObject(hMemDC, hFont  );
    SetBkMode   (hMemDC, TRANSPARENT);
    SetTextColor(hMemDC, RGB(255, 255, 255));

    mybmp.width = get_text_width(hMemDC, " ");
    snprintf(file, sizeof(file), "asc_%02dx%02d.bin", mybmp.width, mybmp.height);
    printf("\ncreate file %s ...\n", file);
    fp = fopen(file, "wb");
    for (i = ' '; i <= '~'; i++) {
        char str[3] = { i };
        printf("\r%s", str); fflush(stdout);
        FillRect(hMemDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        TextOutA(hMemDC, 0, 0, str, strlen(str));
        bmp_binaryzation(&mybmp, distparam);
        n = bmp_to_bits_array(&mybmp, buf, sizeof(buf));
        if (n == -1) break;
        if (fp) fwrite(buf, 1, n, fp);
    }
    if (fp) { fclose(fp); fp = NULL; }

    mybmp.width = get_text_width(hMemDC, "汉");
    snprintf(file, sizeof(file), "hzk_%02dx%02d.bin", mybmp.width, mybmp.height);
    printf("\ncreate file %s ...\n", file);
    fp = fopen(file, "wb");
    if (strcmp(charset, "gb2312") == 0) {
        for (i = 0; i < GB2313_TOTAL_QU; i++) {
            for (j = 0; j < GB2313_TOTAL_WEI; j++) {
                char   str_gb2312[3] = "";
                char   str_utf16 [3] = "";
                size_t len_gb2312    = sizeof(str_gb2312);
                size_t len_utf16     = sizeof(str_utf16 );
                str_gb2312[0] = gb2312_qu_to_byte0 (i);
                str_gb2312[1] = gb2312_wei_to_byte1(j);
                printf("\r%s", str_gb2312);
                string_charset_convert(str_utf16, len_utf16, "utf-16le", str_gb2312, len_gb2312, charset);
                FillRect(hMemDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                TextOutW(hMemDC, 0, 0, (LPCWSTR)str_utf16, 1);
                bmp_binaryzation(&mybmp, distparam);
                n = bmp_to_bits_array(&mybmp, buf, sizeof(buf));
                if (n == -1) break;
                if (fp) fwrite(buf, 1, n, fp);
            }
        }
    } else if (strcmp(charset, "big5") == 0) {
        for (i = 0; i < BIG5_TOTAL_QU; i++) {
            for (j = 0; j < BIG5_TOTAL_WEI; j++) {
                char   str_big5 [3] = "";
                char   str_utf16[3] = "";
                size_t len_big5     = sizeof(str_big5 );
                size_t len_utf16    = sizeof(str_utf16);
                str_big5[0] = big5_qu_to_byte0 (i);
                str_big5[1] = big5_wei_to_byte1(j);
                printf("\r%s", str_big5);
                string_charset_convert(str_utf16, len_utf16, "utf-16le", str_big5, len_big5, charset);
                FillRect(hMemDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                TextOutW(hMemDC, 0, 0, (LPCWSTR)str_utf16, 1);
                bmp_binaryzation(&mybmp, distparam);
                n = bmp_to_bits_array(&mybmp, buf, sizeof(buf));
                if (n == -1) break;
                if (fp) fwrite(buf, 1, n, fp);
            }
        }
    }
    if (fp) { fclose(fp); fp = NULL; }

    if (hMemDC ) DeleteDC(hMemDC);
    if (hBitmap) DeleteObject(hBitmap);
    if (hFont  ) DeleteObject(hFont  );
    return 0;
}
#endif
