import os

def convert_to_utf8_no_bom(root_dir):
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.endswith(('.cpp', '.hpp')):
                full_path = os.path.join(dirpath, filename)
                try:
                    # 先用通用编码尝试打开，避免因编码错误崩溃
                    with open(full_path, 'rb') as f:
                        raw = f.read()

                    # 尝试用 utf-8 解码，如果失败则用 gbk 试试（常见中文编码）
                    try:
                        text = raw.decode('utf-8')
                    except UnicodeDecodeError:
                        text = raw.decode('gbk')

                    # 写回时用 utf-8 无 BOM
                    with open(full_path, 'w', encoding='utf-8') as f:
                        f.write(text)

                    print(f"Converted: {full_path}")
                except Exception as e:
                    print(f"Failed: {full_path}, error: {e}")

if __name__ == "__main__":
    root_directory = input("请输入要转换编码的根目录路径: ").strip()
    if os.path.isdir(root_directory):
        convert_to_utf8_no_bom(root_directory)
    else:
        print("路径不存在或不是目录")
