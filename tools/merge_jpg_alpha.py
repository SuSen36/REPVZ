#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
合并JPG图片和灰度图为带透明度的PNG
用于处理PvZ资源中的JPG+灰度图组合
"""

import os
import sys

from PIL import Image


def merge_jpg_with_alpha(jpg_path, alpha_path, output_path):
    """
    合并JPG和灰度图为带透明度的PNG
    :param jpg_path: JPG/PNG图片路径 (主图)
    :param alpha_path: 灰度图路径（PNG/JPEG/GIF，用作alpha通道）
    :param output_path: 输出PNG路径
    """
    try:
        # 使用上下文管理器确保文件关闭
        with Image.open(jpg_path) as jpg_image_src:
            jpg_image = jpg_image_src.convert('RGB')
        
        # 打开灰度图（支持PNG、JPEG、GIF）
        with Image.open(alpha_path) as alpha_image_src:
            # 智能判断灰度图来源：如果 alpha_image 也是 RGBA 且 RGB 全黑，则使用其 Alpha 通道作为最终 Alpha
            # 否则，使用其 RGB 亮度作为 Alpha (常规灰度图)
            if alpha_image_src.mode in ('RGBA', 'LA'):
                # 检查 RGB 是否有内容
                extrema = alpha_image_src.convert('RGB').getextrema()
                is_black = all(x == (0, 0) for x in extrema)
                
                if is_black:
                    # RGB 全黑，提取 Alpha 通道作为遮罩
                    alpha_image = alpha_image_src.split()[-1]
                else:
                    # RGB 有内容，转为灰度
                    alpha_image = alpha_image_src.convert('L')
            else:
                # 常规模式，直接转灰度
                alpha_image = alpha_image_src.convert('L')
        
        # 确保两个图片尺寸相同
        if jpg_image.size != alpha_image.size:
            print(f"警告: 图片尺寸不匹配 {jpg_path} {jpg_image.size} vs {alpha_path} {alpha_image.size}")
            alpha_image = alpha_image.resize(jpg_image.size, Image.Resampling.LANCZOS)
        
        # 创建RGBA图片
        rgba_image = Image.new('RGBA', jpg_image.size)
        
        # 复制RGB数据
        rgba_image.paste(jpg_image, (0, 0))
        
        # 设置alpha通道（灰度图作为透明度）
        rgba_image.putalpha(alpha_image)
        
        # 保存结果
        rgba_image.save(output_path, 'PNG')
        print(f"已创建: {output_path}")
        return True
        
    except Exception as e:
        print(f"错误处理 {jpg_path}: {e}")
        return False

def process_resources_directory(resources_dir):
    """
    遍历目录，查找JPG及其对应的各种格式的灰度图（_版本）
    """
    processed_count = 0
    deleted_count = 0

    # 定义支持的灰度图后缀
    alpha_extensions = ['_.png', '_.jpg', '_.jpeg', '_.gif']

    for root, dirs, files in os.walk(resources_dir):
        for file in files:
            file_lower = file.lower()
            is_alpha_file = any(file_lower.endswith(ext) for ext in alpha_extensions)

            # 1. 常规模式：处理非灰度图本身的 JPG/PNG 文件
            if (file_lower.endswith('.jpg') or file_lower.endswith('.png')) and not is_alpha_file:
                main_path = os.path.join(root, file)
                base_name = os.path.splitext(file)[0]

                # 尝试查找不同后缀的灰度图文件
                alpha_path = None
                for ext in alpha_extensions:
                    potential_path = os.path.join(root, base_name + ext)
                    if os.path.exists(potential_path):
                        alpha_path = potential_path
                        break

                if alpha_path:
                    # 如果原文件是PNG，直接覆盖；如果是JPG，生成PNG
                    output_name = base_name + '.png'
                    output_path = os.path.join(root, output_name)

                    print(f"匹配成功: [{file}] + [{os.path.basename(alpha_path)}]")

                    # 如果输出文件与输入文件相同（例如 blank.png + blank_.png -> blank.png），
                    # 需要先合并到临时文件，再替换，避免文件占用问题
                    temp_output = None
                    final_output_path = output_path
                    
                    if os.path.abspath(main_path) == os.path.abspath(output_path):
                        temp_output = output_path + ".tmp"
                        output_path = temp_output

                    if merge_jpg_with_alpha(main_path, alpha_path, output_path):
                        processed_count += 1
                        
                        if temp_output:
                            # 替换原文件
                            try:
                                if os.path.exists(final_output_path):
                                    os.remove(final_output_path)
                                os.rename(temp_output, final_output_path)
                                output_path = final_output_path # 恢复路径用于后续逻辑
                            except Exception as e:
                                print(f"  错误: 无法替换文件 {final_output_path}: {e}")

                        # 安全删除原始文件
                        for path_to_del in [main_path, alpha_path]:
                            # 如果输出路径等于要删除的路径，则跳过（防止删除生成的结果）
                            if os.path.abspath(path_to_del) == os.path.abspath(output_path):
                                continue
                                
                            try:
                                if os.path.exists(path_to_del):
                                    os.remove(path_to_del)
                                    deleted_count += 1
                            except Exception as e:
                                print(f"  警告: 无法删除原始文件 {path_to_del}: {e}")

            # 2. Note模式：处理 ZombieNoteBlack*.png (主图) + ZombieNote*.png (灰度图)
            # 例如: ZombieNoteBlack2.png + ZombieNote2.png -> ZombieNote2.png (带透明度)
            elif 'Black' in file and file_lower.endswith('.png'):
                main_path = os.path.join(root, file)
                
                # 尝试推断灰度图名称 (移除 'Black')
                alpha_name = file.replace('Black', '')
                alpha_path = os.path.join(root, alpha_name)
                
                if os.path.exists(alpha_path):
                    # 输出文件即为原本的 alpha 文件名（覆盖它）
                    output_path = alpha_path
                    
                    print(f"匹配成功(Note模式): [{file}] + [{os.path.basename(alpha_path)}]")
                    
                    if merge_jpg_with_alpha(main_path, alpha_path, output_path):
                        processed_count += 1
                        
                        # 仅删除带Black的主图，保留（已覆盖更新的）输出文件
                        try:
                            os.remove(main_path)
                            deleted_count += 1
                        except Exception as e:
                            print(f"  警告: 无法删除原始文件 {main_path}: {e}")

    return processed_count, deleted_count

def main():
    """主函数"""
    if len(sys.argv) > 1:
        resources_dir = sys.argv[1]
    else:
        resources_dir = 'Resources'
    
    if not os.path.exists(resources_dir):
        print(f"错误: 目录 {resources_dir} 不存在")
        return 1
    
    print(f"开始处理 {resources_dir} 目录中的JPG和灰度图...")
    processed_count, deleted_count = process_resources_directory(resources_dir)
    print(f"处理完成！共创建了 {processed_count} 个带透明度的PNG文件")
    print(f"共删除了 {deleted_count} 个原始文件")
    return 0

if __name__ == '__main__':
    sys.exit(main())