@rem=============================================================================
@rem ʹ��˵����
@rem     ��һ���������������ƣ����� ���塢Arial
@rem     �ڶ��������������С��ʵ����������߶ȣ�������Ϊ��λ������ 22
@rem     �����������������ϸ����Χ 1 - 1000������һ��Ϊ 300 - 400��ֵԽ��Խ��
@rem     ���ĸ�������������ɫ��rrggbb ��ʽ����Ҫת��Ϊʮ����
@rem     �����������������ɫ��rrggbb ��ʽ����Ҫת��Ϊʮ����
@rem     ���������������ߺ�ȣ�������Ϊ��λ������Ϊ 0 ���ް���
@rem     ���߸�������������ɫ��rrggbb ��ʽ����Ҫת��Ϊʮ����
@rem     �ڰ˸�����������ƽ�������������Χ 1 - 442������Ϊ -1 ����ƽ������
@rem                 ͨ������Ϊ 300 ����
@rem ע�����
@rem     ���ʹ���˰��ߣ����ɵ�ͼƬ��߱��ް��ߵĶ�Ҫ�����������
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
@rem     argument8: font smoothing, range 1 - 442��set to -1 no smoothing
@rem                suggest 300
@rem note:
@rem     if edge used, the width and height of the generated picture are
@rem     2 pixels more than without edge
@rem=============================================================================

fonttool1.exe ���� 48 600 0 16777215 2 65793 200
fonttool1.exe ���� 14 600 0 16777215 1 65793 200

for %%i in (font*.bmp) do ( bmp24tobmp4bmp2.exe %%i font.pal4 2 )