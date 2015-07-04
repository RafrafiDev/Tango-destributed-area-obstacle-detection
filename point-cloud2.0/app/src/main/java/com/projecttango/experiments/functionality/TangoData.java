package com.projecttango.experiments.functionality;

import android.util.Log;

import net.sf.javaml.core.Dataset;
import net.sf.javaml.core.DefaultDataset;
import net.sf.javaml.core.DenseInstance;

import quickhull3d.Point3d;

/**
 * Created by Yassine on 02/05/2015.
 */
public class TangoData {
    public static Dataset data;
    public static Dataset [] clusters;

    public static void Create(double[] data){
        TangoData.data = new DefaultDataset();

        for (int i=0;i< data.length; i+=3) {
            TangoData.data.add( new DenseInstance(new double[] {  data[i],data[i+1] ,data[i+2]}) );
        }
    }
    public static void Display(){
    Log.i("output_process", "dataset length " + clusters.length);
    try{

        for(int i=0;i<TangoData.clusters.length;i++){
            Log.e("output_process", "dataset "+Integer.toString(i)+" no points within "+TangoData.clusters[i].size());
            for(int j=0;j<TangoData.clusters[i].size();j++){
                //   Log.i("output_process", "x " + TangoData.clusters[i].instance(j).value(0)+" y "+ TangoData.clusters[i].instance(j).value(1) +" z "+TangoData.clusters[i].instance(j).value(2));
            }
        }

        //Log.i(" success","algo finished");
    }catch(Exception e){ Log.i("error", e.getMessage() );}
    }


    public static void checkForm(Point3d[] outline,int k, double left, double right,double top,double bottom){

        for(int j=0;j<outline.length;j++){

            if(outline[j].x>left && outline[j].x>right && outline[j].y<top){
            Log.i("output_process","collision with object "+(k+1));
            break;
            }
            //   Log.i("output_process", "x " + TangoData.clusters[i].instance(j).value(0)+" y "+ TangoData.clusters[i].instance(j).value(1) +" z "+TangoData.clusters[i].instance(j).value(2));
        }
    }

    public static void decideAccordingToForm() {
        Log.e("output_process", "We have  "+TangoData.clusters.length+" clusters");
        for(int i=0;i<TangoData.clusters.length;i++){
            checkForm(Qhull.run(TangoData.clusters[i]),i,-0.2,0.2,0.0,50.0);
        }


    }
}
