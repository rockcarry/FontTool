#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FONT_TYPE_BM  (('B' << 0) | ('M' << 8) | ('F' << 16) | ('T' << 24))
#define FONT_TYPE_DZ  (('D' << 0) | ('Z' << 8) | ('F' << 16) | ('T' << 24))

#pragma pack(1)
typedef struct {
    int32_t  width;   /* 宽度 */
    int32_t  height;  /* 高度 */
    int32_t  stride;  /* 行字节数 */
    void    *pdata;   /* 指向数据 */
} BMP;

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

typedef struct {
    int32_t reserved1;
    int32_t reserved2;
    int32_t reserved3;
    int32_t row_spacing;
    int32_t col_spacing;
    int32_t reserved4;
    int32_t reserved5;
    int32_t asc_yoffset;
    int32_t char_num;
    BMP     font_bmp[0];
} BMFONT;

typedef struct {
    int32_t reserved1;
    int32_t reserved2;
    int32_t reserved3;
    int32_t row_spacing;
    int32_t col_spacing;
    int32_t reserved4;
    int32_t reserved5;
    int32_t asc_yoffset;
    int32_t asc_width;
    int32_t asc_height;
    int32_t asc_offset;
    int32_t hzk_width;
    int32_t hzk_height;
    int32_t hzk_offset;
} DZFONT;
#pragma pack()

#ifdef _TOOL1_
static int bmp_load(BMP *pb, char *file)
{
    BMPFILEHEADER header = {0};
    FILE         *fp     = NULL;
    uint8_t      *pdata  = NULL;
    int           ret, i;

    fp = fopen(file, "rb");
    if (!fp) {
        printf("bmp_load failed: %s !\n", file);
        return -1;
    }

    (void)ret;
    ret = fread(&header, 1, sizeof(header), fp);
    pb->width  = header.biWidth;
    pb->height = header.biHeight;
    pb->stride = ((pb->width * header.biBitCount + 7) / 8 + 3) & ~3;
    pb->pdata  = malloc(pb->stride * pb->height);
    if (pb->pdata) {
        fseek(fp, header.bfOffBits, SEEK_SET);
        pdata  = (uint8_t*)pb->pdata + pb->stride * pb->height;
        for (i = 0; i < pb->height; i++) {
            pdata -= pb->stride;
            ret = fread(pdata, pb->stride, 1, fp);
        }
    }

    fclose(fp);
    return pb->pdata ? 0 : -1;
}

static void bmp_destroy(BMP *pb)
{
    if (pb && pb->pdata) {
        free(pb->pdata);
        pb->pdata = NULL;
    }
}

int main(int argc, char *argv[])
{
    char   *outname = "font.bin";
    char   *bmppath = ".";
    int     fontsize= 16;
    char    filename[256];
    int     row_spacing = 1, col_spacing = 0, asc_yoffset = 1;
    int     char_num, data_size, ret, i;
    BMFONT *font = NULL;

    for (i=0; i<argc; i++) {
        if      (strstr(argv[i], "--path=") == argv[i]) bmppath = argv[i] + strlen("--path=");
        else if (strstr(argv[i], "--out=" ) == argv[i]) outname = argv[i] + strlen("--out=" );
        else if (strstr(argv[i], "--size=") == argv[i]) fontsize= atoi(argv[i] + strlen("--size="));
        else if (strstr(argv[i], "--row_spacing=") == argv[i]) row_spacing = atoi(argv[i] + strlen("--row_spacing="));
        else if (strstr(argv[i], "--col_spacing=") == argv[i]) col_spacing = atoi(argv[i] + strlen("--row_spacing="));
        else if (strstr(argv[i], "--asc_yoffset=") == argv[i]) asc_yoffset = atoi(argv[i] + strlen("--row_spacing="));
    }
    printf("outname    : %s\n", outname    );
    printf("bmppath    : %s\n", bmppath    );
    printf("fontsize   : %d\n", fontsize   );
    printf("row_spacing: %d\n", row_spacing);
    printf("col_spacing: %d\n", col_spacing);
    printf("asc_yoffset: %d\n", asc_yoffset);

    for (i = 0, char_num = 0, data_size = 0; i < 100; i++) {
        BMP bmp = {};
        snprintf(filename, sizeof(filename), "%s/font_%d_%03d.bmp", bmppath, fontsize, i);
        ret = bmp_load(&bmp, filename);
        if (ret != 0) break;
        char_num += 1;
        data_size+= bmp.height * bmp.stride;
        bmp_destroy(&bmp);
    }
    printf("char_num: %d, data_size: %d\n", char_num, data_size);

    font = calloc(1, sizeof(BMFONT) + char_num * sizeof(BMP));
    if (font) {
        FILE *fp = fopen(outname, "wb");
        font->reserved1   = FONT_TYPE_BM;
        font->row_spacing = row_spacing;
        font->col_spacing = col_spacing;
        font->asc_yoffset = asc_yoffset;
        font->char_num    = char_num;
        for (i = 0; i < char_num; i++) {
            snprintf(filename, sizeof(filename), "%s/font_%d_%03d.bmp", bmppath, fontsize, i);
            ret = bmp_load(&(font->font_bmp[i]), filename);
        }
        if (fp) {
            fwrite(font, 1, sizeof(BMFONT) + char_num * sizeof(BMP), fp);
            for (i = 0; i < char_num; i++) {
                fwrite(font->font_bmp[i].pdata, 1, font->font_bmp[i].height * font->font_bmp[i].stride, fp);
                bmp_destroy(&(font->font_bmp[i]));
            }
            fclose(fp);
        }
        free(font);
    }

    return 0;
}
#endif

#ifdef _TOOL2_
static int get_file_size(char *file)
{
    FILE *fp = fopen(file, "rb");
    if (!fp) return 0;
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fclose(fp);
    return size;
}

static void write_file(FILE *dstfp, char *file)
{
    FILE *srcfp = fopen(file, "rb");
    int   c;
    if (srcfp) {
        while ((c = fgetc(srcfp)) != EOF) fputc(c, dstfp);
        fclose(srcfp);
    } else {
        printf("failed to open file %s !\n", file);
    }
}

int main(int argc, char *argv[])
{
    char   *outname = "dzfont.bin";
    char   *ascfile = "asc_08x16.bin";
    char   *hzkfile = "hzk_16x16.bin";
    DZFONT  font    = { FONT_TYPE_DZ, 2, 1, 1, 8, 16, 0, 16, 16, 0 };
    FILE   *fp      = NULL;
    int     i;

    for (i = 0; i < argc; i++) {
        if      (strstr(argv[i], "--ascfile="    ) == argv[i]) ascfile = argv[i] + strlen("--ascfile=");
        else if (strstr(argv[i], "--hzkfile="    ) == argv[i]) hzkfile = argv[i] + strlen("--hzkfile=");
        else if (strstr(argv[i], "--out="        ) == argv[i]) outname = argv[i] + strlen("--out=");
        else if (strstr(argv[i], "--asc_width="  ) == argv[i]) font.asc_width   = atoi(argv[i] + strlen("--asc_width="  ));
        else if (strstr(argv[i], "--asc_height=" ) == argv[i]) font.asc_height  = atoi(argv[i] + strlen("--asc_height=" ));
        else if (strstr(argv[i], "--hzk_width="  ) == argv[i]) font.hzk_width   = atoi(argv[i] + strlen("--hzk_width="  ));
        else if (strstr(argv[i], "--hzk_height=" ) == argv[i]) font.hzk_height  = atoi(argv[i] + strlen("--hzk_height=" ));
        else if (strstr(argv[i], "--row_spacing=") == argv[i]) font.row_spacing = atoi(argv[i] + strlen("--row_spacing="));
        else if (strstr(argv[i], "--col_spacing=") == argv[i]) font.col_spacing = atoi(argv[i] + strlen("--col_spacing="));
        else if (strstr(argv[i], "--asc_yoffset=") == argv[i]) font.asc_yoffset = atoi(argv[i] + strlen("--asc_yoffset="));
    }

    font.asc_offset = sizeof(DZFONT);
    font.hzk_offset = font.asc_offset + get_file_size(ascfile);

    printf("ascfile    : %s\n"   , ascfile);
    printf("hzkfile    : %s\n"   , hzkfile);
    printf("outname    : %s\n"   , outname);
    printf("row_spacing: %d\n"   , font.row_spacing);
    printf("col_spacing: %d\n"   , font.col_spacing);
    printf("asc_yoffset: %d\n"   , font.asc_yoffset);
    printf("asc_size   : %dx%d\n", font.asc_width, font.asc_height);
    printf("asc_offset : %d\n"   , font.asc_offset);
    printf("hzk_size   : %dx%d\n", font.hzk_width, font.hzk_height);
    printf("hzk_offset : %d\n"   , font.hzk_offset);

    fp = fopen(outname, "wb");
    if (fp) {
        fwrite(&font, 1, sizeof(font), fp);
        write_file(fp, ascfile);
        write_file(fp, hzkfile);
        fclose(fp);
    }
    return 0;
}
#endif

