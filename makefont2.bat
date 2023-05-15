@rem=============================================================================
@rem 使用说明：
@rem     第一个参数：字体名称，例如 宋体、Arial
@rem     第二个参数：字体大小，实际上是字体高度，以像素为单位，比如 22
@rem     第三个参数：字体粗细，范围 1 - 1000，正常一般为 300 - 400，值越大越粗
@rem     第四个参数：字体平滑处理参数，范围 1 - 442，设置为 -1 则不做平滑处理
@rem                通常设置为 300 即可
@rem=============================================================================

@rem=============================================================================
@rem usage:
@rem     argument1: font name, for example Arial
@rem     argument2: font size, in pixel unit, for example 22
@rem     argument3: font weight, range 1 - 1000, suggest 300 - 400
@rem     argument4: font smoothing, range 1 - 442，set to -1 no smoothing
@rem                suggest 300
@rem=============================================================================

fonttool2.exe 宋体 16 600 200
pause