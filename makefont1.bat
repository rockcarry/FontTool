@rem=============================================================================
@rem 使用说明：
@rem     第一个参数：字体名称，例如 宋体、Arial
@rem     第二个参数：字体大小，实际上是字体高度，以像素为单位，比如 22
@rem     第三个参数：字体粗细，范围 1 - 1000，正常一般为 300 - 400，值越大越粗
@rem     第四个参数：背景颜色，rrggbb 格式，需要转换为十进制
@rem     第五个参数：字体颜色，rrggbb 格式，需要转换为十进制
@rem     第六个参数：包边厚度，以像素为单位，设置为 0 则无包边
@rem     第七个参数：包边颜色，rrggbb 格式，需要转换为十进制
@rem     第八个参数：字体平滑处理参数，范围 1 - 442，设置为 -1 则不做平滑处理
@rem                 通常设置为 300 即可
@rem     第九个参数：" 0123456789-:星期一二三四五六日"（你可以替换其中的字符）
@rem                 如果传入 full 则使用如下的字符集：
@rem " 0123456789-:星期一二三四五六日!"#$%&'()*+,-./;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
@rem 注意事项：
@rem     如果使用了包边，生成的图片宽高比无包边的都要多出两个像素
@rem=============================================================================

@rem=============================================================================
@rem usage:
@rem     argument1: font name, for example Arial
@rem     argument2: font size, in pixel unit, for example 22
@rem     argument3: font weight, range 1 - 1000, suggest 300 - 400
@rem     argument4: background color, rrggbb, please convert to decimal number
@rem     argument5: font color, rrggbb, please convert to decimal number
@rem     argument6: edge size, in pixel unit, if set to 0 no edge
@rem     argument7: edge color, rrggbb, please convert to decimal number
@rem     argument8: font smoothing, range 1 - 442，set to -1 no smoothing
@rem                suggest 300
@rem     argument9: " 0123456789-:星期一二三四五六日"
@rem                if pass full then using charset as below:
@rem " 0123456789-:星期一二三四五六日!"#$%&'()*+,-./;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
@rem note:
@rem     if edge used, the width and height of the generated picture are
@rem     2 pixels more than without edge
@rem=============================================================================

fonttool1.exe 黑体 48 600 0 16777215 2 65793 200 " 0123456789-:星期一二三四五六日"
fonttool1.exe 宋体 14 600 0 16777215 1 65793 200 " 0123456789-:星期一二三四五六日"

for %%i in (font*.bmp) do ( bmp24tobmp4bmp2.exe %%i font.pal4 2 )