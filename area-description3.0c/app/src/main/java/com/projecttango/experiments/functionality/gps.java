package com.projecttango.experiments.functionality;

import android.content.Context;
import android.location.Location;
import android.location.LocationManager;
import android.util.Log;


public class gps {
	private String TAG = "Location";
	public Context AppContext = null;
	public gps(Context c){
		AppContext = c;
	}

	private Location getLocationByProvider(String provider) {
		Location location = null;

		LocationManager locationManager = (LocationManager) AppContext
				.getSystemService(Context.LOCATION_SERVICE);
		try {
			if (locationManager.isProviderEnabled(provider)) {
				location = locationManager.getLastKnownLocation(provider);
			}
		} catch (IllegalArgumentException e) {
			Log.d(TAG, "Cannot acces Provider " + provider);
		}
		return location;
	}


	public Location getBestLocation() {
	    Location gpslocation = getLocationByProvider(LocationManager.GPS_PROVIDER);
	    Location networkLocation = getLocationByProvider(LocationManager.NETWORK_PROVIDER);
	    // if we have only one location available, the choice is easy
	    if (gpslocation == null) {
	        Log.d(TAG, "No GPS Location available.");
	        return networkLocation;
	    }
	    if (networkLocation == null) {
	        Log.d(TAG, "No Network Location available");
	        return gpslocation;
	    }
	    networkLocation = null;
	    gpslocation = null;
	    return null;

	}

	public Location getOneBestLocation(){
		Location loc = null;
		int trytime=0;
		while (loc == null && trytime<500){
			loc = getBestLocation();
			trytime++;
		}

		return loc;
	}

	public static String toString(gps L){

		Location loc = L.getOneBestLocation() ;
		if(loc!=null)
		{

			return (loc.getLatitude()+","+loc.getLongitude()+"#");
		}

		else	return ("-200,-200#");

	}

}
