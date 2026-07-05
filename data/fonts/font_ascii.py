import argparse
import os
import sys
import re
from PIL import Image, ImageFont, ImageDraw

def is_ascii(ch):
    """判断一个字符是否是标准 ASCII 字符"""
    return ord(ch) <= 127

def get_ascii_var_name(char, used_names):
    """根据 ASCII 字符生成不重复的变量名（全大写）"""
    special_mapping = {
        ' ': 'SPACE', '!': 'EXCLAMATION', '"': 'DOUBLE_QUOTE', '#': 'HASH',
        '$': 'DOLLAR', '%': 'PERCENT', '&': 'AMPERSAND', '\'': 'SINGLE_QUOTE',
        '(': 'LEFT_PAREN', ')': 'RIGHT_PAREN', '*': 'ASTERISK', '+': 'PLUS',
        ',': 'COMMA', '-': 'MINUS', '.': 'DOT', '/': 'SLASH',
        ':': 'COLON', ';': 'SEMICOLON', '<': 'LESS_THAN', '=': 'EQUAL',
        '>': 'GREATER_THAN', '?': 'QUESTION', '@': 'AT', '[': 'LEFT_BRACKET',
        '\\': 'BACKSLASH', ']': 'RIGHT_BRACKET', '^': 'CARET', '_': 'UNDERSCORE',
        '`': 'BACKTICK', '{': 'LEFT_BRACE', '|': 'PIPE', '}': 'RIGHT_BRACE',
        '~': 'TILDE'
    }
    
    if char.isalnum():
        base_name = f"MINI_FONT_ASCII_{char.upper()}"
    elif char in special_mapping:
        base_name = f"MINI_FONT_ASCII_{special_mapping[char]}"
    else:
        base_name = f"MINI_FONT_ASCII_{ord(char):02X}"
    
    base_name = re.sub(r'[^A-Za-z0-9_]', '', base_name)
    
    if base_name not in used_names:
        used_names[base_name] = 0
        return base_name
    else:
        counter = 1
        while True:
            test_name = f"{base_name}_{counter}"
            if test_name not in used_names:
                used_names[test_name] = 0
                return test_name
            counter += 1

def generate_ssd1306_bitmap(char, width, height, font_path):
    """利用 Pillow 绘制 ASCII 字符并按照 SSD1306 页格式（纵向扫描）提取点阵"""
    img = Image.new("1", (width, height), 0)
    draw = ImageDraw.Draw(img)
    
    try:
        font = ImageFont.truetype(font_path, min(width, height))
    except IOError:
        print(f"Error: 无法加载字体文件: {font_path}", file=sys.stderr)
        sys.exit(1)
        
    left, top, right, bottom = draw.textbbox((0, 0), char, font=font)
    w_text = right - left
    h_text = bottom - top
    x_offset = (width - w_text) // 2 - left
    y_offset = (height - h_text) // 2 - top
    
    draw.text((x_offset, y_offset), char, font=font, fill=1, fontmode="1")
    
    aligned_height = (height + 7) & ~7
    pages = aligned_height // 8
    bitmap_bytes = []
    
    for p in range(pages):
        for x in range(width):
            byte_val = 0
            for bit in range(8):
                y = p * 8 + bit
                if y < height:
                    pixel = img.getpixel((x, y))
                    if pixel > 0:
                        byte_val |= (1 << bit)
            bitmap_bytes.append(byte_val)
            
    return bitmap_bytes

def main():
    parser = argparse.ArgumentParser(
        description="从文本文件读取 ASCII 字符并转换为 SSD1306 OLED C语言点阵字库工具"
    )
    parser.add_argument("-i", "--input", required=True, help="输入的文本文件路径")
    parser.add_argument("-w", "--width", type=int, required=True, help="字模宽度")
    parser.add_argument("-g", "--height", type=int, required=True, help="字模高度")
    parser.add_argument("-f", "--font", required=True, help="TTF字体文件路径")
    
    if len(sys.argv) == 1:
        parser.print_help(sys.stderr)
        print("\n[错误] 未检测到任何命令行参数，程序已退出。", file=sys.stderr)
        sys.exit(1)
        
    args = parser.parse_args()
    
    if not os.path.isfile(args.input):
        print(f"[错误] 输入的文本文件不存在: {args.input}", file=sys.stderr)
        sys.exit(1)
        
    if not os.path.isfile(args.font):
        print(f"[错误] 指定的字体文件不存在: {args.font}", file=sys.stderr)
        sys.exit(1)
        
    if args.width <= 0 or args.height <= 0:
        print(f"[错误] 字模的宽度(-w)与高度(-g)必须为大于 0 的正整数！", file=sys.stderr)
        sys.exit(1)
        
    unique_chars = []
    with open(args.input, "r", encoding="utf-8") as f:
        content = f.read()
        for ch in content:
            if is_ascii(ch) and (ch == ' ' or not ch.isspace()) and ch not in unique_chars:
                unique_chars.append(ch)
                
    if not unique_chars:
        print("[警告] 未在输入文件中检索到任何有效的 ASCII 字符。", file=sys.stderr)
        sys.exit(0)
        
    unique_chars.sort()
        
    used_names = {}
    
    print("/* ==================== ASCII 点阵数组定义开始 ==================== */\n")
    for char in unique_chars:
        var_name = get_ascii_var_name(char, used_names)
        bytes_data = generate_ssd1306_bitmap(char, args.width, args.height, args.font)
        
        if bytes_data is None:
            continue
            
        display_char = char
        if char == "'": display_char = "\\'"
        elif char == "\\": display_char = "\\\\"
        
        print(f"const uint8_t code {var_name}[] =\n{{")
        for i in range(0, len(bytes_data), 16):
            chunk = bytes_data[i:i+16]
            hex_str = ",".join([f"0x{b:02X}" for b in chunk])
            if i + 16 < len(bytes_data):
                hex_str += ","
            print(f"    {hex_str}")
        print(f"    /* '{display_char}' ({args.width} X {args.height}) */")
        print("};\n")
        
    print("/* ==================== ASCII 点阵数组定义结束 ==================== */")

if __name__ == "__main__":
    main()
