package com.popcap.pvz;

import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.widget.Toast;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;

public class PVZActivity extends SDLActivity {

    static {
        System.loadLibrary("SDL2");
        System.loadLibrary("bass");
        System.loadLibrary("re-plants-vs-zombies");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE); // 强制为横屏
        // 直接复制资产，不请求权限
        copyAssetsToExternalStorage();
    }

    private void copyAssetsToExternalStorage() {
        AssetManager assetManager = getAssets();
        String[] files;

        try {
            files = assetManager.list(""); // 列出根目录下的文件

            if (files != null && files.length > 0) {
                String dataDirPath = getExternalFilesDir(null) + "/AssetsFolder";
                createDataFolder(dataDirPath);

                int fileCount = files.length; // 统计文件数量
                for (String filename : files) {
                    copyAssetFile(assetManager, filename, dataDirPath + "/" + filename);
                }
                Toast.makeText(this, "所有文件复制完成！共 " + fileCount + " 个文件夹。", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "没有可供复制的文件。", Toast.LENGTH_SHORT).show();
            }
        } catch (IOException e) {
            e.printStackTrace();
            Toast.makeText(this, "获取 assets 文件时出错！", Toast.LENGTH_SHORT).show();
        }
    }

    private void createDataFolder(String dataDirPath) {
        File dataDir = new File(dataDirPath);
        if (!dataDir.exists() && dataDir.mkdirs()) {
            Toast.makeText(this, "数据文件夹创建成功！", Toast.LENGTH_SHORT).show();
        } else if (!dataDir.exists()) {
            Toast.makeText(this, "数据文件夹创建失败！", Toast.LENGTH_SHORT).show();
        }
    }

    private void copyAssetFile(AssetManager assetManager, String filename, String destPath) {
        try {
            // 检查是否是目录
            String[] subFiles = assetManager.list(filename);
            if (subFiles != null && subFiles.length > 0) {
                // 确保目标目录存在
                File dir = new File(destPath);
                if (!dir.exists()) {
                    dir.mkdirs(); // 创建目录
                }

                // 递归遍历子目录
                for (String subFile : subFiles) {
                    copyAssetFile(assetManager, filename + "/" + subFile, destPath + "/" + subFile);
                }
                return; // 返回，处理目录完毕
            }

            // 在复制文件之前检查目标文件是否已存在
            File outFile = new File(destPath);
            if (outFile.exists()) {
                return; // 跳过该文件的复制
            }

            // 使用 InputStream 和 OutputStream 进行文件复制
            try (InputStream in = assetManager.open(filename);
                 OutputStream out = Files.newOutputStream(outFile.toPath())) {

                byte[] buffer = new byte[1024];
                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }

                // 复制成功后的进一步处理（如日志）可以在此处添加

            } catch (IOException e) {
                e.printStackTrace();
                Toast.makeText(this, "复制文件 " + filename + " 时出错：" + e.getMessage(), Toast.LENGTH_SHORT).show();
            }

        } catch (IOException e) {
            e.printStackTrace();
            Toast.makeText(this, "访问文件 " + filename + " 时出错：" + e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }

}