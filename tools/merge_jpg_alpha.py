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
    :param jpg_path: JPG图片路径
    :param alpha_path: 灰度图路径（PNG/JPEG/GIF，用作alpha通道）
    :param output_path: 输出PNG路径
    """
    try:
        # 打开JPG图片
        jpg_image = Image.open(jpg_path)
        jpg_image = jpg_image.convert('RGB')
        
        # 打开灰度图（支持PNG、JPEG、GIF）
        alpha_image = Image.open(alpha_path)
        alpha_image = alpha_image.convert('L')  # 确保是灰度模式
        
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
            # 仅处理非灰度图本身的主JPG文件
            if file.lower().endswith('.jpg') and not any(file.lower().endswith(ext) for ext in alpha_extensions):
                jpg_path = os.path.join(root, file)
                base_name = os.path.splitext(file)[0]

                # 尝试查找不同后缀的灰度图文件
                alpha_path = None
                for ext in alpha_extensions:
                    potential_path = os.path.join(root, base_name + ext)
                    if os.path.exists(potential_path):
                        alpha_path = potential_path
                        break

                if alpha_path:
                    output_name = base_name + '.png'
                    output_path = os.path.join(root, output_name)

                    print(f"匹配成功: [{file}] + [{os.path.basename(alpha_path)}]")

                    if merge_jpg_with_alpha(jpg_path, alpha_path, output_path):
                        processed_count += 1

                        # 安全删除原始文件
                        for path_to_del in [jpg_path, alpha_path]:
                            try:
                                os.remove(path_to_del)
                                deleted_count += 1
                            except Exception as e:
                                print(f"  警告: 无法删除原始文件 {path_to_del}: {e}")

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