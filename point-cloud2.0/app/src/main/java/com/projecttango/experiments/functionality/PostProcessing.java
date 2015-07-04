package com.projecttango.experiments.functionality;

import com.projecttango.experiments.nativepointcloud.TangoJNINative;

import net.sf.javaml.classification.Classifier;
import net.sf.javaml.classification.KNearestNeighbors;
import net.sf.javaml.classification.NearestMeanClassifier;
import net.sf.javaml.clustering.KMeans;
import net.sf.javaml.core.Instance;
import net.sf.javaml.distance.DistanceMeasure;


public class PostProcessing {

    static private DistanceMeasure dm = new DistanceMeasure() {

    @Override
    public double measure(Instance i1, Instance i2) {
       return Math.sqrt(Math.pow(i1.value(0)-i2.value(0),2)+Math.pow(i1.value(1)-i2.value(1),2)+Math.pow(i1.value(2)-i2.value(2),2));
    }
    @Override
    public boolean compare(double d, double d1) {
        return d < d1;
    }
    @Override
    public double getMinValue() {
        return 0;
    }
    @Override
    public double getMaxValue() {
        return Double.POSITIVE_INFINITY;
    }
    };

    public static void xmeans(){
        double [] res=TangoJNINative.getDoubleDepthData();
        if (res!=null && res.length!=0){
        TangoData.Create(res);
        TangoData.clusters=Clustering.xmeans(TangoData.data);
        TangoData.decideAccordingToForm();
        }
    }

    public static void kmeansThumbRule(){
        double [] res=TangoJNINative.getDoubleDepthData();
        if (res!=null && res.length!=0){

             TangoData.Create(res);

             KMeans kmeans=new KMeans( (int)Math.sqrt(TangoData.data.size()/2)+1 , 1000 , dm);
             TangoData.clusters=kmeans.cluster(TangoData.data);
             NearestMeanClassifier nmc= new NearestMeanClassifier();
             nmc.buildClassifier(TangoData.data);

            TangoData.decideAccordingToForm();
        }
    }

    public static void kmeans(int noClusters){
        if(noClusters<1)
            noClusters=1;
        double [] res=TangoJNINative.getDoubleDepthData();
        if (res!=null && res.length!=0){

            TangoData.Create(res);

            KMeans kmeans=new KMeans( noClusters , 1000 , dm);
            TangoData.clusters=kmeans.cluster(TangoData.data);
            NearestMeanClassifier nmc= new NearestMeanClassifier();
            nmc.buildClassifier(TangoData.data);

            TangoData.decideAccordingToForm();
        }
    }


    public static void knn(int noClusters){
        //abondonne car c'est une methode d'aprentissage qui n'ecessite un nombre important de parametre en entree
        //validation croisee
        if(noClusters<1)
            noClusters=1;
        double [] res=TangoJNINative.getDoubleDepthData();
        if (res!=null && res.length!=0){
            TangoData.Create(res);
            Classifier knn = new KNearestNeighbors(5);
            knn.buildClassifier(TangoData.data);
            TangoData.decideAccordingToForm();
        }
    }




}
