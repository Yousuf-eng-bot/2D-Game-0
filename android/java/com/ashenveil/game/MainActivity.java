package com.ashenveil.game;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;
import android.media.AudioAttributes;
import android.media.SoundPool;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import java.io.File;
import java.io.FileOutputStream;

/**
 * Platform bridge only. Simulation, pixel rendering, UI, input interpretation
 * and saves are C++.
 */
public final class MainActivity extends Activity {
  static { System.loadLibrary("ashen"); }
  static native void boot(String path);
  static native void frame(int[] pixels, float dt);
  static native void touch(int action, int id, float x, float y);
  static native void suspend();
  static native void back();
  static native int sound();
  static native int rate();
  static native int audio();
  static native int haptic();
  static native int music();
  static native int textRequest();
  static native String textValue(int kind);
  static native void submitText(int kind, String value);
  static native String report();
  static native boolean gpuAttach(android.view.Surface surface);
  static native void gpuDetach();
  static native boolean gpuPresent(float dt,int width,int height,float ox,float oy,float scale);
  static native void deviceProfile(long ramMiB,boolean lowRam,boolean saver,int thermal,boolean pressure);
  boolean gpuReady = false;
  android.view.TextureView gpuView;
  android.view.Surface gpuSurface;
  long profileTime = 0, gpuRetryAfter = 0;
  void updateDeviceProfile() { updateDeviceProfile(false); }
  void updateDeviceProfile(boolean force) {
    long now=android.os.SystemClock.uptimeMillis();
    if(!force&&now-profileTime<1000)return;
    profileTime=now;
    android.app.ActivityManager am=(android.app.ActivityManager)getSystemService(ACTIVITY_SERVICE);
    android.app.ActivityManager.MemoryInfo info=new android.app.ActivityManager.MemoryInfo();
    am.getMemoryInfo(info);
    android.os.PowerManager pm=(android.os.PowerManager)getSystemService(POWER_SERVICE);
    int thermal=android.os.Build.VERSION.SDK_INT>=29?pm.getCurrentThermalStatus():0;
    deviceProfile(info.totalMem/(1024*1024),am.isLowRamDevice(),pm.isPowerSaveMode(),thermal,info.lowMemory);
  }
  boolean dialogVisible = false;
  void showNativeDialog(int kind) {
    if (dialogVisible || isFinishing())
      return;
    dialogVisible = true;
    final boolean bn = textValue(100).equals("bn");
    if (kind == 3) {
      updateDeviceProfile(true);
      final String info = report() +
                          "\nDevice: " + android.os.Build.MANUFACTURER + " " +
                          android.os.Build.MODEL +
                          "\nAndroid: " + android.os.Build.VERSION.RELEASE;
      new android.app.AlertDialog.Builder(this)
          .setTitle(bn ? "মতামত জানান" : "Playtest feedback")
          .setMessage(info + (bn ? "\n\nকিছু আপলোড হয় না। তথ্য কপি করে আপনার মতামত বা স্ক্রিনশটের সঙ্গে পাঠান।" : "\n\nNothing is uploaded. Copy this and send it with your comments or screenshot."))
          .setPositiveButton(
              bn ? "তথ্য কপি করুন" : "Copy info",
              (d, w) -> {
                android.content.ClipboardManager cb =
                    (android.content.ClipboardManager)getSystemService(
                        CLIPBOARD_SERVICE);
                cb.setPrimaryClip(android.content.ClipData.newPlainText(
                    "Death World feedback", info));
              })
          .setNegativeButton(bn ? "বন্ধ" : "Close", null)
          .setOnDismissListener(d -> dialogVisible = false)
          .show();
      return;
    }
    final android.widget.EditText edit = new android.widget.EditText(this);
    edit.setSingleLine(true);
    edit.setInputType(kind == 2 ? android.text.InputType.TYPE_CLASS_NUMBER
                                : android.text.InputType.TYPE_CLASS_TEXT);
    edit.setFilters(new android.text.InputFilter[] {
        new android.text.InputFilter.LengthFilter(kind == 2 ? 20 : 24)});
    edit.setText(textValue(kind));
    edit.selectAll();
    edit.setHint(kind == 1 ? (bn ? "A-Z, সংখ্যা ও স্পেস" : "English letters, numbers, spaces") : (bn ? "শূন্য বা ধনাত্মক পূর্ণসংখ্যা" : "Unsigned whole number"));
    android.widget.LinearLayout box = new android.widget.LinearLayout(this);
    box.setPadding(32, 8, 32, 8);
    box.addView(edit, new android.widget.LinearLayout.LayoutParams(-1, -2));
    new android.app.AlertDialog.Builder(this)
        .setTitle(kind == 1 ? (bn ? "জগতের নাম (A-Z / 0-9)" : "World name (A-Z / 0-9)") : (bn ? "জগতের সিড" : "World seed"))
        .setView(box)
        .setPositiveButton(
            bn ? "সংরক্ষণ" : "Save", (d, w) -> submitText(kind, edit.getText().toString()))
        .setNegativeButton(bn ? "বাতিল" : "Cancel", null)
        .setOnDismissListener(d -> dialogVisible = false)
        .show();
  }
  private GameView game;
  @Override
  public void onCreate(Bundle b) {
    super.onCreate(b);
    setVolumeControlStream(android.media.AudioManager.STREAM_MUSIC);
    getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    getWindow().getDecorView().setSystemUiVisibility(5894);
    // Native renderer reads a prepared RGB565 title image; no network or
    // decoder is needed.
    try (java.io.InputStream in = getAssets().open("cover.bin");
         java.io.OutputStream out =
             new FileOutputStream(new File(getFilesDir(), "cover.bin"))) {
      byte[] block = new byte[16384];
      int count;
      while ((count = in.read(block)) != -1)
        out.write(block, 0, count);
    } catch (Exception e) {
      android.util.Log.w("DeathWorld", "Optional cover fallback", e);
    }
    boot(getFilesDir().getAbsolutePath());
    game = new GameView();
    android.widget.FrameLayout root=new android.widget.FrameLayout(this);
    gpuView=new android.view.TextureView(this);
    gpuView.setOpaque(true);
    gpuView.setSurfaceTextureListener(new android.view.TextureView.SurfaceTextureListener(){
      public void onSurfaceTextureAvailable(android.graphics.SurfaceTexture texture,int w,int h){
        gpuSurface=new android.view.Surface(texture);
        gpuReady=gpuAttach(gpuSurface);
        game.last=0;game.invalidate();
      }
      public void onSurfaceTextureSizeChanged(android.graphics.SurfaceTexture t,int w,int h){game.last=0;touch(3,0,0,0);}
      public boolean onSurfaceTextureDestroyed(android.graphics.SurfaceTexture t){
        gpuReady=false;gpuDetach();
        if(gpuSurface!=null){gpuSurface.release();gpuSurface=null;}
        return true;
      }
      public void onSurfaceTextureUpdated(android.graphics.SurfaceTexture t){}
    });
    root.addView(gpuView,new android.widget.FrameLayout.LayoutParams(-1,-1));
    root.addView(game,new android.widget.FrameLayout.LayoutParams(-1,-1));
    setContentView(root);
    updateDeviceProfile();
  }
  @Override
  protected void onPause() {
    super.onPause();
    if (game != null) {
      game.running = false;
      game.pauseAudio();
      game.last = 0;
    }
    suspend();
  }
  @Override
  protected void onResume() {
    super.onResume();
    if (game != null) {
      updateDeviceProfile(true);
      game.running = true;
      game.requestFocusAudio();
      game.resumeAudio();
      game.last = 0;
      game.invalidate();
    }
  }
  @Override
  protected void onStop() {
    if (game != null)
      game.releaseMusic();
    super.onStop();
  }
  @Override
  public void onBackPressed() {
    back();
  }
  @Override
  protected void onDestroy() {
    if (game != null)
      game.release();
    gpuReady=false;gpuDetach();
    if(gpuSurface!=null){gpuSurface.release();gpuSurface=null;}
    super.onDestroy();
  }
  final class GameView extends View {
    final Bitmap bitmap =
        Bitmap.createBitmap(640, 360, Bitmap.Config.ARGB_8888);
    final int[] pixels = new int[640 * 360];
    final Paint paint = new Paint();
    final RectF dst = new RectF();
    long last = 0, nextFrame = 0;
    boolean running = true;
    float scale = 1, ox = 0, oy = 0;
    SoundPool pool;
    int[] sounds = new int[27];
    android.util.SparseBooleanArray ready =
        new android.util.SparseBooleanArray();
    java.util.ArrayDeque<Integer> pending = new java.util.ArrayDeque<>();
    android.media.MediaPlayer musicPlayer;
    boolean musicPrepared = false, windowFocused = true, audioFocused = true;
    int musicMode = 0;
    android.media.AudioManager audioManager;
    android.media.AudioManager.OnAudioFocusChangeListener focusListener;
    int ambienceMode = 0, ambientStream = 0;
    GameView() {
      super(MainActivity.this);
      paint.setFilterBitmap(false);
      setFocusable(true);
      initSounds();
      audioManager =
          (android.media.AudioManager)getSystemService(AUDIO_SERVICE);
      focusListener = change -> {
        audioFocused = change == android.media.AudioManager.AUDIOFOCUS_GAIN;
        if (audioFocused)
          resumeAudio();
        else {
          pauseAudio();
          suspend();
        }
      };
      requestFocusAudio();
    }
    void initSounds() {
      try {
        pool = new SoundPool.Builder()
                   .setMaxStreams(12)
                   .setAudioAttributes(
                       new AudioAttributes.Builder()
                           .setUsage(AudioAttributes.USAGE_GAME)
                           .setContentType(
                               AudioAttributes.CONTENT_TYPE_SONIFICATION)
                           .build())
                   .build();
        pool.setOnLoadCompleteListener((p, id, status) -> {
          if (status == 0)
            ready.put(id, true);
        });
        for (int k = 1; k <= 26; k++) {
          android.content.res.AssetFileDescriptor fd =
              getAssets().openFd("audio/" + k + ".wav");
          sounds[k] = pool.load(fd, 1);
          fd.close();
        }
      } catch (Exception e) {
        android.util.Log.w("DeathWorld", "Optional audio unavailable", e);
      }
    }
    void updateAmbience() {
      if (pool == null || !canPlayAudio())
        return;
      int desired = audio();
      if (desired != ambienceMode) {
        if (ambientStream != 0)
          pool.stop(ambientStream);
        int[] ids = {0, 13, 14, 22, 23, 24};
        int sample = sounds[ids[Math.max(0, Math.min(5, desired))]];
        ambientStream = desired == 0 || !ready.get(sample)
                            ? 0
                            : pool.play(sample, .24f, .24f, 3, -1, 1f);
        ambienceMode = desired == 0 || ambientStream != 0 ? desired : 0;
      }
    }
    boolean canPlayAudio() { return running && windowFocused && audioFocused; }
    void requestFocusAudio() {
      if (audioManager != null)
        audioFocused =
            audioManager.requestAudioFocus(
                focusListener, android.media.AudioManager.STREAM_MUSIC,
                android.media.AudioManager.AUDIOFOCUS_GAIN) ==
            android.media.AudioManager.AUDIOFOCUS_REQUEST_GRANTED;
    }
    void pauseAudio() {
      if (pool != null)
        pool.autoPause();
      if (musicPlayer != null && musicPrepared)
        try {
          musicPlayer.pause();
        } catch (IllegalStateException ignored) {
        }
    }
    void resumeAudio() {
      if (!canPlayAudio())
        return;
      if (pool != null)
        pool.autoResume();
      if (musicPlayer != null && musicPrepared)
        try {
          musicPlayer.start();
        } catch (IllegalStateException ignored) {
        }
    }
    void releaseMusic() {
      musicPrepared = false;
      musicMode = 0;
      if (musicPlayer != null) {
        musicPlayer.release();
        musicPlayer = null;
      }
    }
    void updateMusic() {
      if (!canPlayAudio())
        return;
      int desired = music();
      if (desired == musicMode)
        return;
      releaseMusic();
      musicMode = desired;
      if (desired == 0)
        return;
      try {
        final android.media.MediaPlayer player =
            new android.media.MediaPlayer();
        musicPlayer = player;
        player.setAudioAttributes(
            new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_GAME)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                .build());
        android.content.res.AssetFileDescriptor fd =
            getAssets().openFd("music/" + desired + ".wav");
        player.setDataSource(fd.getFileDescriptor(), fd.getStartOffset(),
                             fd.getLength());
        fd.close();
        player.setLooping(true);
        player.setVolume(.32f, .32f);
        player.setOnPreparedListener(mp -> {
          if (mp == musicPlayer) {
            musicPrepared = true;
            if (canPlayAudio())
              mp.start();
          }
        });
        player.setOnErrorListener((mp, what, extra) -> {
          if (mp == musicPlayer) {
            releaseMusic();
            musicMode = desired;
          }
          return true;
        });
        player.prepareAsync();
      } catch (Exception e) {
        releaseMusic();
        musicMode = desired;
        android.util.Log.w("DeathWorld", "Optional music unavailable", e);
      }
    }
    void release() {
      releaseMusic();
      if (audioManager != null)
        audioManager.abandonAudioFocus(focusListener);
      if (pool != null)
        pool.release();
      bitmap.recycle();
    }
    int insetL = 0, insetR = 0, insetT = 0, insetB = 0;
    void layoutFrame(int w, int h) {
      int sw = Math.max(1, w - insetL - insetR),
          sh = Math.max(1, h - insetT - insetB);
      scale = Math.min(sw / 640f, sh / 360f);
      ox = insetL + (sw - 640 * scale) / 2;
      oy = insetT + (sh - 360 * scale) / 2;
      dst.set(ox, oy, ox + 640 * scale, oy + 360 * scale);
    }
    @Override
    public android.view.WindowInsets
    onApplyWindowInsets(android.view.WindowInsets insets) {
      insetL = insets.getSystemWindowInsetLeft();
      insetR = insets.getSystemWindowInsetRight();
      insetT = insets.getSystemWindowInsetTop();
      insetB = insets.getSystemWindowInsetBottom();
      if (android.os.Build.VERSION.SDK_INT >= 28 &&
          insets.getDisplayCutout() != null) {
        android.view.DisplayCutout d = insets.getDisplayCutout();
        insetL = Math.max(insetL, d.getSafeInsetLeft());
        insetR = Math.max(insetR, d.getSafeInsetRight());
        insetT = Math.max(insetT, d.getSafeInsetTop());
        insetB = Math.max(insetB, d.getSafeInsetBottom());
      }
      layoutFrame(getWidth(), getHeight());
      return insets;
    }
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
      layoutFrame(w, h);
      touch(3, 0, 0, 0);
    }
    @Override
    protected void onWindowVisibilityChanged(int visibility) {
      super.onWindowVisibilityChanged(visibility);
      last = 0;
    }
    @Override
    public void onWindowFocusChanged(boolean focus) {
      super.onWindowFocusChanged(focus);
      windowFocused = focus;
      if (!focus) {
        suspend();
        pauseAudio();
        last = 0;
      } else {
        updateDeviceProfile(true);
        requestFocusAudio();
        resumeAudio();
        last = 0;
      }
    }
    @Override
    protected void onDraw(Canvas c) {
      // Retry a lost surface/backend, not on every frame and never from a
      // second game-state thread. A failed attach still retains Canvas.
      long retryNow=android.os.SystemClock.uptimeMillis();
      if(!gpuReady&&running&&windowFocused&&gpuSurface!=null&&gpuSurface.isValid()&&retryNow>=gpuRetryAfter){
        gpuRetryAfter=retryNow+5000;
        gpuReady=gpuAttach(gpuSurface);
        last=0;
      }
      // An empty display list is transparent; CLEAR would erase the sibling TextureView.
      if(!gpuReady)c.drawColor(0xff070f14);
      updateDeviceProfile();
      long now = System.nanoTime();
      long period = 1000000000L / rate();
      if (running && last != 0 && now + 500000L < nextFrame) {
        if(!gpuReady)c.drawBitmap(bitmap, null, dst, paint);
        postInvalidateOnAnimation();
        return;
      }
      nextFrame = last == 0 || now > nextFrame + period * 2 ? now + period : nextFrame + period;
      float dt = last == 0 ? 0 : Math.min(1f, (now - last) / 1e9f);
      last = now;
      float step=running&&windowFocused?dt:0;
      boolean attempted=gpuReady;
      if(gpuReady)gpuReady=gpuPresent(step,getWidth(),getHeight(),ox,oy,scale);
      if(!gpuReady){
        if(attempted)gpuRetryAfter=android.os.SystemClock.uptimeMillis()+5000;
        frame(pixels,attempted?0:step);
        bitmap.setPixels(pixels,0,640,0,0,640,360);
        c.drawBitmap(bitmap,null,dst,paint);
      }
      updateAmbience();
      updateMusic();
      for (int k = 0; k < 4; k++) {
        int s = sound();
        if (s > 0 && s < sounds.length && pending.size() < 24)
          pending.add(s);
      }
      int waiting = pending.size();
      for (int k = 0; k < waiting; k++) {
        int s = pending.remove();
        boolean allowed = music() != 0 || audio() != 0 || s == 16;
        if (pool != null && canPlayAudio() && allowed && ready.get(sounds[s]))
          pool.play(sounds[s], s >= 17 && s <= 19 ? .3f : .62f,
                    s >= 17 && s <= 19 ? .3f : .62f, 2, 0, 1f);
        else if (canPlayAudio() && allowed)
          pending.add(s);
      }
      int request = textRequest();
      if (request != 0)
        post(() -> showNativeDialog(request));
      if (haptic() != 0)
        performHapticFeedback(android.view.HapticFeedbackConstants.LONG_PRESS);
      if (running) {
        postInvalidateOnAnimation();
      }
    }
    @Override
    public boolean onTouchEvent(MotionEvent e) {
      int a = e.getActionMasked(), i = e.getActionIndex();
      if (a == MotionEvent.ACTION_CANCEL) {
        touch(3, 0, 0, 0);
        return true;
      }
      if (a == MotionEvent.ACTION_MOVE) {
        for (int j = 0; j < e.getPointerCount(); j++)
          touch(2, e.getPointerId(j), (e.getX(j) - ox) / scale,
                (e.getY(j) - oy) / scale);
      } else if (a == MotionEvent.ACTION_DOWN ||
                 a == MotionEvent.ACTION_POINTER_DOWN)
        touch(0, e.getPointerId(i), (e.getX(i) - ox) / scale,
              (e.getY(i) - oy) / scale);
      else if (a == MotionEvent.ACTION_UP || a == MotionEvent.ACTION_POINTER_UP)
        touch(1, e.getPointerId(i), (e.getX(i) - ox) / scale,
              (e.getY(i) - oy) / scale);
      return true;
    }
  }
}
