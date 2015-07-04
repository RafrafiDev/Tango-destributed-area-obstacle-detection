package com.projecttango.experiments.functionality;

import net.sf.javaml.clustering.Clusterer;
import net.sf.javaml.core.Dataset;
import net.sf.javaml.tools.weka.WekaClusterer;

import weka.clusterers.XMeans;

/**
 * Created by Yassine on 02/05/2015.
 */
public class Clustering {
static Dataset[] xmeans(Dataset data){
    XMeans xm = new XMeans();
    xm.setMinNumClusters(2);
    xm.setMaxNumClusters((int)Math.sqrt(data.size()/2)+1);

    Clusterer jmlxm = new WekaClusterer(xm);
    return jmlxm.cluster(data);
    }
}
