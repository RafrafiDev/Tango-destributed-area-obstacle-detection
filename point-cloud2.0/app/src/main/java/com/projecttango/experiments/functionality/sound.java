package com.projecttango.experiments.functionality;

import android.content.Context;
import android.media.MediaPlayer;

import com.projecttango.experiments.nativepointcloud.R;
import com.projecttango.experiments.nativepointcloud.TangoJNINative;
// this class get decision from z-average values given by the c++ program

public class sound {
    private final int[] sounds ={R.raw.beep1, R.raw.beep2};
    private Context context;
    private android.os.Handler customHandler ;
    public static boolean isPause=true;



    public Runnable updateTimerThread = new Runnable() {
                                                            public void run()
                                                            {
                                                                // Log.e("tango_jni_DECISION", "" + TangoJNINative.getDecision());

                                                                start(TangoJNINative.getDecision());

                                                                customHandler.postDelayed(this, 1000);

                                                            }
                                                        };

    public sound(Context context){
        sound.isPause=false;
        this.context=context;
        customHandler = new android.os.Handler();
        customHandler.postDelayed(updateTimerThread, 0);
    }

    public void start(int dec){
       if(!sound.isPause && dec>-1)
        try{MediaPlayer.create(context , sounds[dec]).start();}catch (Exception e){}
    }


}
