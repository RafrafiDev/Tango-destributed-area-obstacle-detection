package com.projecttango.experiments.functionality;



import net.sf.javaml.core.Dataset;
import net.sf.javaml.core.Instance;

import quickhull3d.*;

public class Qhull {

	public static Point3d[] run(Dataset vertexArray){
//	Log.i("leght ",""+vertexArray.size());
		int size=vertexArray.size();
		Point3d[] points =new Point3d[size];

		for(int i=0;i<size;i++){
			Instance ins=vertexArray.get(i);
			points[i]=new Point3d (ins.value(0),  ins.value(1), ins.value(2));
		};
	QuickHull3D hull = new QuickHull3D();
	hull.build (points);
	return hull.getVertices();
/*	   Point3d[] vertices = hull.getVertices();
	   for (int i = 0; i < vertices.length; i++)
	    { Point3d pnt = vertices[i];
	      Log.i (TAG,pnt.x + " " + pnt.y + " " + pnt.z);
	    }

	   //System.out.println ("Faces:");
	   int[][] faceIndices = hull.getFaces();
	   for (int i = 0; i < faceIndices.length; i++)
	    { for (int k = 0; k < faceIndices[i].length; k++)
	       {
	    	Log.i  (TAG,faceIndices[i][k] + " ");
	       }
	    }*/
	   }
}
