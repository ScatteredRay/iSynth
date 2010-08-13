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
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;


public class iSynth extends Activity {
    private XYView panel;
    private Audio aud;
    private InputPosition inp;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        indexFiles();
        inp = new InputPosition();
        panel = new XYView(this, inp);
        setContentView(panel);
        registerForContextMenu(panel);

        //setContentView(R.layout.main);
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
            String[] fns = am.list("patches");
            String[] args = new String[fns.length+1];
            args[0] = "iSynth";
            
            String relpath;
            for (int i=0; i<fns.length; ++i) {
                relpath = "patches/"+fns[i];
                AssetFileDescriptor afd = am.openFd(relpath);
                relpath = relpath.substring(0, relpath.length()-4);
                addFile(relpath, afd.getStartOffset(), afd.getLength());
                afd.close();

                args[i+1]=new String(relpath);
            }
            
            setArgs(args);
            
            fns = am.list("samples");
            for (int i=0; i<fns.length; ++i) {
                relpath = "samples/"+fns[i];
                AssetFileDescriptor afd = am.openFd(relpath);
                addFile(relpath.substring(0, relpath.length()-4), afd.getStartOffset(), afd.getLength());
                afd.close();
            }

        } catch (IOException e) {
            Log.e("indexFiles", "Couldn't open apk file");
            return;
        }


    }


    //this isn't getting called for some reason, so i've worked it
    //into onKeyDown for now.
    @Override
    public void onBackPressed() {
         inp.setNextPatch(-1);
    }
    
    @Override
    public boolean onSearchRequested() {
        inp.setNextPatch(1);
        return true;
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent ev) {
        if (keyCode == KeyEvent.KEYCODE_DPAD_RIGHT) {
            inp.setNextPatch(1);
            return true;
        }
        if (keyCode == KeyEvent.KEYCODE_DPAD_LEFT ||
            keyCode == KeyEvent.KEYCODE_BACK) {
            inp.setNextPatch(-1);
            return true;
        }
        return false;
    }

    private native void setApkPath(String path);
    private native void addFile(String name, long pos, long len);
    private native void setArgs(String[] args);


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
    private int nextPatch=0;
    
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
    
    public synchronized void setNextPatch(int d) {
        this.nextPatch = d;
    }
    
    public synchronized int getNextPatch() {
        int tmp=nextPatch;
        nextPatch=0;
        return tmp;

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
    private native void nextPatch(int d);

    public void run() {
        if (at.getState() != AudioTrack.STATE_INITIALIZED) {
            Log.e("Audio", "not initialized!");
            return;
        }
        short[] buf = new short[buf_size];
        at.play();
        int np;
        for (;;) {
            inputXY(inp.getXScaled(), inp.getYScaled());
            inputDown(inp.isDown());
            produceStream(buf, 256);
            at.write(buf, 0, 512);

            if(interrupted()) {
                at.stop();
                return;
            }
            np = inp.getNextPatch();
            if (np != 0) {
                nextPatch(np);
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
