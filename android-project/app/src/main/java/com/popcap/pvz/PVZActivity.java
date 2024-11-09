package com.popcap.pvz;

import android.content.pm.ActivityInfo;
import android.content.res.AssetManager;
import android.os.Bundle;
import org.libsdl.app.SDLActivity;


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
        initializeAssetManager();
    }


    public native void initializeAssetManager();

    public AssetManager getAssetManager() {
        return this.getAssets();
    }
}