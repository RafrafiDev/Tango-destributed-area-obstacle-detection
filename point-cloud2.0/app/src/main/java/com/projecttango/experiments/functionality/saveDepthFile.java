package com.projecttango.experiments.functionality;

import android.os.Environment;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Random;

/**
 * Created by Yassine on 27/04/2015.
 */


public class saveDepthFile  {

    private static int no=0;
    public static String genKey(){

        char[] chars = "1234567890".toCharArray();
        StringBuilder sb = new StringBuilder();
        Random random = new Random();
        for (int i = 0; i < 20; i++) {
            char c = chars[random.nextInt(chars.length)];
            sb.append(c);
        }
        return sb.toString();
    }


    public static String inc(){
        no++;
        return Integer.toString(no);
    }


    public static String saveDepthfile(String str) {


        if(str.length()>0){
            try {


                File sdcard = Environment.getExternalStorageDirectory();


                File dir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).getPath()+ "/%Btango%/");
                if (!(dir.exists() && dir.isDirectory())) {
                    dir.mkdirs();
                }
                String fileName="output"+inc()+".csv";
                File file = new File(dir, fileName);
                file.delete();
                file.createNewFile();
                OutputStream fOut;

                fOut = new FileOutputStream(file);


                fOut.write(str.getBytes());
                fOut.close();

                    return "DIRECTORY_DOWNLOADS/" + fileName + " saved";

            } catch (IOException e) {

                Log.e("IOException ",
                        "depth file creation");
                e.printStackTrace();
            }

        }
        return"No point cloud available";

    }
}