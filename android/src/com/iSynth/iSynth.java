package com.iSynth;

import java.io.IOException;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;


public class iSynth extends Activity {
    private XYView panel;
    private Audio aud;
    private InputPosition inp;
    private String[] patches;
    
    private static final int PATCH_GID = 1;
    private static final int SCALE_GID = 2;
    private static final int KEY_GID = 3;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        indexFiles();
        inp = new InputPosition();
        panel = new XYView(this, inp);
        setContentView(panel);
    }

    @Override
    public void onStart() {
        super.onStart();
        aud = new Audio(inp);
        aud.setPriority(Thread.MAX_PRIORITY);
        aud.start();
    }

    @Override
    public void onStop() {
        super.onStop();
        aud.interrupt();
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Menu patchmenu = menu.addSubMenu(Menu.NONE, Menu.NONE, 0, "patch");
        for (String p: patches) {
            patchmenu.add(PATCH_GID, Menu.NONE, Menu.NONE, p.substring(0,p.length()-8));
        }
        
        Menu scalemenu = menu.addSubMenu(Menu.NONE, Menu.NONE, 0, "scale");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Major");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Minor");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Dorian");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Phrygian");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Lydian");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Mixolydian");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Locrian");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Pentatonic");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "PentMinor");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Chromatic");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Whole");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Minor 3rd");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "3rd");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "4th");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "5th");
        scalemenu.add(SCALE_GID, Menu.NONE, Menu.NONE, "Octave");

        Menu keymenu = menu.addSubMenu(Menu.NONE, Menu.NONE, 0, "key");
        keymenu.add(KEY_GID, 0, Menu.NONE, "C");
        keymenu.add(KEY_GID, 1, Menu.NONE, "C#/Db");
        keymenu.add(KEY_GID, 2, Menu.NONE, "D");
        keymenu.add(KEY_GID, 3, Menu.NONE, "D#/Eb");
        keymenu.add(KEY_GID, 4, Menu.NONE, "E");
        keymenu.add(KEY_GID, 5, Menu.NONE, "F");
        keymenu.add(KEY_GID, 6, Menu.NONE, "F#/Gb");
        keymenu.add(KEY_GID, 7, Menu.NONE, "G");
        keymenu.add(KEY_GID, 8, Menu.NONE, "G#/Ab");
        keymenu.add(KEY_GID, 9, Menu.NONE, "A");
        keymenu.add(KEY_GID, 10, Menu.NONE, "A#/Bb");
        keymenu.add(KEY_GID, 11, Menu.NONE, "B");

        return true;
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getGroupId()) {
        case PATCH_GID:
            setPatch(item.getTitle()+".pat");
            return true;
        case SCALE_GID:
            setScale(item.getTitle().toString().toLowerCase());
            return true;
        case KEY_GID:
            setKey(item.getItemId());
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }
    /**
     * Provides a list of packaged patches and samples for the jni side, along with
     * lengths and offsets within the apk file.
     * 
     * This currently assumes all packaged patch and sample filenames have an extra
     * .png extension -- this is a cheap hack to bypass file compression. A better way
     * to do this would be to use the -0 option of the aapk command, but there's no
     * way to configure this for the eclipse plugin.
     */
    private void indexFiles() {
        String apkpath;

        try {
            apkpath = getPackageManager().getApplicationInfo("com.iSynth",0).sourceDir;
            setApkPath(apkpath);
        } catch (NameNotFoundException e){
            Log.e("indexFiles", "Couldn't find apk file");
            return;
        }
        AssetManager am = getAssets();
        try {
            patches = am.list("patches");
            
            String relpath;
            for (String patch: patches) {
                relpath = "patches/"+patch;
                AssetFileDescriptor afd = am.openFd(relpath);
                relpath = relpath.substring(0, relpath.length()-4);
                addFile(relpath, afd.getStartOffset(), afd.getLength());
                afd.close();
            }
            
            String[] fns = am.list("samples");
            for (String fn: fns) {
                relpath = "samples/"+fn;
                AssetFileDescriptor afd = am.openFd(relpath);
                addFile(relpath.substring(0, relpath.length()-4), afd.getStartOffset(), afd.getLength());
                afd.close();
            }

        } catch (IOException e) {
            Log.e("indexFiles", "Couldn't open apk file");
            return;
        }


    }
    
    

    private native void setApkPath(String path);
    private native void addFile(String name, long pos, long len);
    private native void setPatch(String patch);
    private native void setScale(String scale);
    private native void setKey(int key);
//    private native void setOctave(int octave);


    static {
        System.loadLibrary("iSynth");
    }
}

class InputPosition {
    private float x=0;
    private float y=0;
    private boolean down=false;
    private int w=600;
    private int h=600;
    
    public synchronized float getXScaled() {
        x = Math.max(0, Math.min(w, x));
        return x*2/w - 1;
    }

    public synchronized void setXY(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public synchronized float getYScaled() {
        y = Math.max(0, Math.min(h, y));
        return 1 - y*2/h;
    }
    public synchronized boolean isDown() {
        return down;
    }
    public synchronized void setDown(boolean down) {
        this.down = down;
    }
    public synchronized void setDimensions(int w, int h) {
        this.w = w;
        this.h = h;
    }
}

class Audio extends Thread {
    private AudioTrack at;
    private int buf_size; // in bytes!
    private InputPosition inp;
    private static final int samplerate = 44100;

    public Audio(InputPosition inp) {
        super();
        this.inp = inp;

        buf_size = AudioTrack.getMinBufferSize(samplerate,
                AudioFormat.CHANNEL_CONFIGURATION_STEREO,
                AudioFormat.ENCODING_PCM_16BIT);
        Log.d("Audio", "bufsize="+buf_size);
        at = new AudioTrack(AudioManager.STREAM_MUSIC,
                samplerate,
                AudioFormat.CHANNEL_CONFIGURATION_STEREO,
                AudioFormat.ENCODING_PCM_16BIT,
                buf_size,
                AudioTrack.MODE_STREAM);
 
    }

    private native short[] produceStream(short[] buffer, int count);
    private native void inputXY(float X, float Y);
    private native void inputDown(boolean d);

    public void run() {
        if (at.getState() != AudioTrack.STATE_INITIALIZED) {
            Log.e("Audio", "not initialized!");
            return;
        }
        short[] buf = new short[buf_size];
        at.play();
        for (;;) {
            inputXY(inp.getXScaled(), inp.getYScaled());
            inputDown(inp.isDown());
            produceStream(buf, 256);
            at.write(buf, 0, 512);

            if(interrupted()) {
                at.stop();
                return;
            }
        }
    }
}





class XYView extends SurfaceView implements SurfaceHolder.Callback {
    private InputPosition inp;

    public XYView(Context context, InputPosition inp) {
        super(context);
        this.inp = inp;
        getHolder().addCallback(this);
    }

    @Override
    public boolean onTouchEvent(final MotionEvent ev) {
        inp.setXY(ev.getX(), ev.getY());
        inp.setDown(ev.getAction() != MotionEvent.ACTION_UP);
        return true;
    }


    public void surfaceCreated(SurfaceHolder sh) {}
    public void surfaceDestroyed(SurfaceHolder sh) {}
    public void surfaceChanged(SurfaceHolder sh, int fmt, int w, int h)	{
        inp.setDimensions(w, h);
    }

}
