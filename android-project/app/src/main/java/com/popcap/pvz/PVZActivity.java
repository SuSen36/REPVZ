package com.popcap.pvz;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.widget.Toast;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class PVZActivity extends SDLActivity {

    static {
        System.loadLibrary("bass");
        System.loadLibrary("SDL2");
        System.loadLibrary("re-plants-vs-zombies");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 直接复制资产，不请求权限
        copyAssetsToExternalStorage();
    }

    private void copyAssetsToExternalStorage() {
        AssetManager assetManager = getAssets();
        String[] files;
        try {
            files = assetManager.list(""); // 列出根目录下的文件
            if (files != null && files.length > 0) {
                // 创建数据文件夹
                createDataFolder();

                for (String filename : files) {
                    // 使用递归方式复制文件和目录
                    copyAssetFile(assetManager, filename, "AssetsFolder/" + filename);
                }
                Toast.makeText(this, "所有文件复制完成！", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "没有可供复制的文件。", Toast.LENGTH_SHORT).show();
            }
        } catch (IOException e) {
            e.printStackTrace();
            Toast.makeText(this, "获取 assets 文件时出错！", Toast.LENGTH_SHORT).show();
        }
    }

    private void createDataFolder() {
        File dataDir = new File(getExternalFilesDir(null), "AssetsFolder");
        if (!dataDir.exists()) {
            if (dataDir.mkdirs()) {
                Toast.makeText(this, "数据文件夹创建成功！", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "数据文件夹创建失败！", Toast.LENGTH_SHORT).show();
            }
        }
    }

    private void copyAssetFile(AssetManager assetManager, String filename, String destPath) {
        try {
            // 检查是否是目录
            if (assetManager.list(filename).length > 0) { // 检查是否是文件夹
                // 创建目标目录
                File dir = new File(getExternalFilesDir(null), destPath);
                if (!dir.exists()) {
                    dir.mkdirs();
                }

                // 递归遍历子目录
                String[] subFiles = assetManager.list(filename);
                for (String subFile : subFiles) {
                    copyAssetFile(assetManager, filename + "/" + subFile, destPath + "/" + subFile);
                }
                return; // 返回，处理目录完毕
            }

            // 在复制文件之前检查目标文件是否已存在
            File outFile = new File(getExternalFilesDir(null), destPath);
            if (outFile.exists()) {
                Toast.makeText(this, "文件已存在，跳过复制: " + filename, Toast.LENGTH_SHORT).show();
                return; // 跳过该文件的复制
            }

            // 打开 assets 中的文件
            InputStream in = assetManager.open(filename);
            FileOutputStream out = new FileOutputStream(outFile); // 使用目标路径

            // 复制文件
            byte[] buffer = new byte[1024];
            int read;
            Toast.makeText(this, "正在复制文件: " + filename, Toast.LENGTH_SHORT).show(); // 增加提示

            while ((read = in.read(buffer)) != -1) {
                out.write(buffer, 0, read);
            }
            out.flush();

            // 关闭流
            out.close();
            in.close();
            Toast.makeText(this, "成功复制文件: " + filename, Toast.LENGTH_SHORT).show(); // 复制成功提示
        } catch (IOException e) {
            e.printStackTrace();
            Toast.makeText(this, "复制文件 " + filename + " 时出错！", Toast.LENGTH_SHORT).show();
        }
    }
}