@rem=============================================================================
@rem 使用说明：
@rem     第一个参数：字体名称，例如 宋体、Arial
@rem     第二个参数：字体大小，实际上是字体高度，以像素为单位，比如 22
@rem     第三个参数：字体粗细，范围 1 - 1000，正常一般为 300 - 400，值越大越粗
@rem     第四个参数：背景颜色，rrggbb 格式，需要转换为十进制
@rem     第五个参数：字体颜色，rrggbb 格式，需要转换为十进制
@rem     第六个参数：包边颜色，rrggbb 格式，需要转换为十进制，设置为 -1 为无包边
@rem     第七个参数：字体平滑处理参数，范围 1 - 442，设置为 -1 则不做平滑处理
@rem                 通常设置为 300 即可
@rem 注意事项：
@rem     如果使用了包边，生成的图片宽高比无包边的都要多出两个像素
@rem=============================================================================

fonttool.exe 宋体 22 500 16711935 16776960 -1 300
