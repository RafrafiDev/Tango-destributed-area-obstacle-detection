/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <vector>
#include <math.h>

#include "tango_data.h"

#define Pi 3.141592654f
TangoData::TangoData() : config_(nullptr) { ResetData(); }

// This callback function is called when new pose updates become available.
static void onPoseAvailable(void*, const TangoPoseData* pose) {
  int current_index = -1;
  // Set pose for device wrt start.
  // Parsing through the pose targe/base frame to set the index number
  // to the correct pose data.
  if (pose->frame.base == TANGO_COORDINATE_FRAME_START_OF_SERVICE &&
      pose->frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    current_index = 0;
  } else if (pose->frame.base == TANGO_COORDINATE_FRAME_AREA_DESCRIPTION &&
             pose->frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    // Set pose for device wrt ADF.
    current_index = 1;
  } else if (pose->frame.base == TANGO_COORDINATE_FRAME_AREA_DESCRIPTION &&
             pose->frame.target == TANGO_COORDINATE_FRAME_START_OF_SERVICE) {
    // Set pose for start wrt ADF.
    current_index = 2;
    if (pose->status_code == TANGO_POSE_VALID) {
      TangoData::GetInstance().is_relocalized = true;
    } else {
      TangoData::GetInstance().is_relocalized = false;
    }
  } else if (pose->frame.base == TANGO_COORDINATE_FRAME_PREVIOUS_DEVICE_POSE &&
             pose->frame.target == TANGO_COORDINATE_FRAME_DEVICE) {
    // Set pose for device wrt previous pose.
    current_index = 3;
  } else {
    return;
  }

  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  // Set Tango pose to cooresponding index.
  TangoData::GetInstance().tango_position[current_index] = glm::vec3(
      pose->translation[0], pose->translation[1], pose->translation[2]);

  TangoData::GetInstance().tango_rotation[current_index] =
      glm::quat(pose->orientation[3], pose->orientation[0],
                pose->orientation[1], pose->orientation[2]);

  // Calculate delta frame time.
  TangoData::GetInstance().frame_delta_time[current_index] =
      (pose->timestamp -
       TangoData::GetInstance().prev_frame_time[current_index]) *
      kSecondToMillisecond;
  TangoData::GetInstance().prev_frame_time[current_index] = pose->timestamp;

  // Set/reset frame counter.
  if (pose->status_code !=
      TangoData::GetInstance().prev_pose_status[current_index]) {
    TangoData::GetInstance().frame_count[current_index] = 0;
  }
  TangoData::GetInstance().prev_pose_status[current_index] = pose->status_code;
  ++TangoData::GetInstance().frame_count[current_index];

  // Set pose status.
  TangoData::GetInstance().current_pose_status[current_index] =
      static_cast<int>(pose->status_code);
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);
}

// Tango event callback.
static void onTangoEvent(void*, const TangoEvent* event) {
  // Update the status string for debug display.
  pthread_mutex_lock(&TangoData::GetInstance().event_mutex);
  std::stringstream string_stream;
  string_stream << event->event_key << ": " << event->event_value;
  TangoData::GetInstance().event_string = string_stream.str();
  pthread_mutex_unlock(&TangoData::GetInstance().event_mutex);
}

// Initialize Tango Service.
TangoErrorType TangoData::Initialize(JNIEnv* env, jobject activity) {
  // The initialize function perform API and Tango Service version check,
  // the there is a mis-match between API and Tango Service version, the
  // function will return TANGO_INVALID.
  return TangoService_initialize(env, activity);
}

void TangoData::ResetData() {
  for (int i = 0; i < 4; i++) {
    tango_position[i] = glm::vec3(0.0f, 0.0f, 0.0f);
    tango_rotation[i] = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    current_pose_status[i] = -4;
    prev_pose_status[i] = -4;
    frame_delta_time[i] = 0.0f;
    prev_frame_time[i] = 0.0f;
    frame_count[i] = 0;
  }
  pos_adf.push_back(glm::vec3(2000,2000,2000));
  cur_uuid = std::string();
  event_string = std::string();
  lib_version_string = std::string();
  is_relocalized = false;
}

bool TangoData::SetConfig(bool is_learning, bool is_load_adf,const char *name) {
  // Get the default TangoConfig.
  config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (config_ == NULL) {
    LOGE("TangoService_getConfig(): Failed");
    return false;
  }

  // Define is recording or loading a map.
  if (is_learning) {
    if (TangoConfig_setBool(config_, "config_enable_learning_mode", true) !=
        TANGO_SUCCESS) {
      LOGI("config_enable_learning_mode Failed");
      return false;
    }
  }
 if (is_load_adf) {
    LoadRangeUUIDFile(name,30);//distance per meter
 }
 else
 LOGI("you need to enable adf load to enable area recognition");

  // Get library version string from service.
  char version[kVersionStringLength];
  TangoConfig_getString(config_, "tango_service_library_version", version,
                        kVersionStringLength);
  lib_version_string = std::string(version);

  return true;
}

Loc TangoData::GetNameMetadataValue(std::string uuid) {

 size_t size = 0;
 char* name;
 TangoAreaDescriptionMetadata metadata;
 // Set the event callback listener.
 if (TangoService_getAreaDescriptionMetadata(uuid.c_str(), &metadata) !=
     TANGO_SUCCESS) {
   LOGE("TangoService_getAreaDescriptionMetadata(): Failed");
 }
 if (TangoAreaDescriptionMetadata_get(metadata, "name", &size, &name) !=
     TANGO_SUCCESS) {
   LOGE("TangoAreaDescriptionMetadata_get(): Failed");
 }
 float lat,lon;
//   __android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrrb: %s ",name);
    sscanf (name,"%f,%f#",&lat,&lon);
  //__android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrrb: %f, %f ",lat,lon);

 Loc l(lat,lon);
 return l;
}


char* TangoData::GetUUIDMetadataValue(const char* uuid, const char* key) {
  size_t size = 0;
  char* name;
  TangoAreaDescriptionMetadata metadata;
  // Set the event callback listener.
  if (TangoService_getAreaDescriptionMetadata(uuid, &metadata) !=
      TANGO_SUCCESS) {
    LOGE("TangoService_getAreaDescriptionMetadata(): Failed");
  }
  if (TangoAreaDescriptionMetadata_get(metadata, key, &size, &name) !=
      TANGO_SUCCESS) {
    LOGE("TangoAreaDescriptionMetadata_get(): Failed");
  }
  return name;
}

inline float DegToRad(float x)
{
    return x / 180 * Pi;
}

float measureDistance(float lat1,float lon1,float lat2,float lon2){//meters
 float R = 6371000;
    float phi1 = DegToRad(lat1);
    float phi2 = DegToRad(lat2);
    float deltaphi = DegToRad(lat2-lat1);
    float deltalamda = DegToRad(lon2-lon1);

    float a = sin(deltaphi/2) * sin(deltaphi/2) +
              cos(phi1) * cos(phi2) *
              sin(deltalamda/2) * sin(deltalamda/2);

    float c = 2 * atan2(sqrt(a), sqrt(1-a));


return R*c;
}
bool TangoData::LoadRangeUUIDFile(const char * name, int maxdist){
    char* uuid_list;

    // uuid_list will contain a comma separated list of UUIDs.
    float lat1,lon1;
    sscanf (name,"%f,%f#",&lat1,&lon1);


// __android_log_print(ANDROID_LOG_INFO, "MyTag", "LoadRangeUUIDFile" );
//__android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrra: %f ",measureDistance(0,0.0001f,0,0) );
//__android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrra: %s ",name );
//__android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrra: %.7f, %.7f ",lat1,lon1 );


    if (TangoService_getAreaDescriptionUUIDList(&uuid_list) != TANGO_SUCCESS) {
      LOGI("TangoService_getAreaDescriptionUUIDList");
    }

    // Parse the uuid_list to get the individual uuids.
    if (uuid_list != NULL && uuid_list[0] != '\0') {
      char* parsing_char;
      char* saved_ptr;
      parsing_char = strtok_r(uuid_list, ",", &saved_ptr);

      close_adf_list.clear();
      adf_list.clear();

      while (parsing_char != NULL) {
        std::string s = std::string(parsing_char);


        Loc l = GetNameMetadataValue(s);
        loc_list.push_back(l);

 //__android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrra: %f ",measureDistance(lat1 , lon1 , l.lat , l.lon));


        if(measureDistance(lat1 , lon1 , l.lat , l.lon)<maxdist){
        close_adf_list.push_back(s);
        }

        adf_list.push_back(s);



        parsing_char = strtok_r(NULL, ",", &saved_ptr);
      }
// __android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrrc: %f, %f ",loc_list[0].lat,loc_list[0].lon);

      int list_size = close_adf_list.size();

 //__android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrrc: %d ",list_size);

      if (list_size == 0) {
        LOGE("List size is 0");
        return false;
      }


        cur_uuid = close_adf_list[0];
        cur_name = GetUUIDMetadataValue(cur_uuid.c_str(),"name");

      if (TangoConfig_setString(TangoData::GetInstance().config_, "config_load_area_description_UUID",
                                cur_uuid.c_str()) !=
          TANGO_SUCCESS) {
        LOGE("config_load_area_description_uuid Failed");
        return false;
      }
    } else {
      LOGE("No area description file available, no file loaded.");
    }
    LOGI("Area description file loaded.");
   return true;
}



bool TangoData::ConnectCallbacks() {
  // Set listening pairs. Connect pose callback.
  TangoCoordinateFramePair pairs[4] = {
      {TANGO_COORDINATE_FRAME_START_OF_SERVICE, TANGO_COORDINATE_FRAME_DEVICE},
      {TANGO_COORDINATE_FRAME_AREA_DESCRIPTION, TANGO_COORDINATE_FRAME_DEVICE},
      {TANGO_COORDINATE_FRAME_AREA_DESCRIPTION,
       TANGO_COORDINATE_FRAME_START_OF_SERVICE},
      {TANGO_COORDINATE_FRAME_PREVIOUS_DEVICE_POSE,
       TANGO_COORDINATE_FRAME_DEVICE}};
  if (TangoService_connectOnPoseAvailable(4, pairs, onPoseAvailable) !=
      TANGO_SUCCESS) {
    LOGI("TangoService_connectOnPoseAvailable(): Failed");
    return false;
  }

  // Set the event callback listener.
  if (TangoService_connectOnTangoEvent(onTangoEvent) != TANGO_SUCCESS) {
    LOGI("TangoService_connectOnTangoEvent(): Failed");
    return false;
  }

  return true;
}

// Connect to Tango Service, service will start running, and
// pose can be queried.
TangoErrorType TangoData::Connect() {
  return TangoService_connect(nullptr, config_);
}

bool TangoData::SaveADF() {
  TangoUUID uuid;
  if (TangoService_saveAreaDescription(&uuid) != TANGO_SUCCESS) {
    LOGE("TangoService_saveAreaDescription(): Failed");
    return false;
  }
  cur_uuid = std::string(uuid);
  return true;
}

// Disconnect Tango Service.
void TangoData::Disconnect() { TangoService_disconnect(); }

char* TangoData::GetAllUUIDs() {
  char* uuid;
  if (TangoService_getAreaDescriptionUUIDList(&uuid) != TANGO_SUCCESS) {
    LOGI("TangoService_getAreaDescriptionUUIDList");
  }
  return uuid;
}


void TangoData::SetUUIDMetadataValue(const char* uuid, const char* key,
                                     int value_size, const char* value) {
  TangoAreaDescriptionMetadata metadata;
  if (TangoService_getAreaDescriptionMetadata(uuid, &metadata) !=
      TANGO_SUCCESS) {
    LOGE("TangoService_getAreaDescriptionMetadata(): Failed");
  }
  if (TangoAreaDescriptionMetadata_set(metadata, key, value_size, value) !=
      TANGO_SUCCESS) {
    LOGE("TangoAreaDescriptionMetadata_set(): Failed");
  }
  if (TangoService_saveAreaDescriptionMetadata(uuid, metadata) !=
      TANGO_SUCCESS) {
    LOGE("TangoService_saveAreaDescriptionMetadata(): Failed");
  }
}

void TangoData::DeleteADF(const char* uuid) {
  TangoService_deleteAreaDescription(uuid);
}

void TangoData::LogAllUUIDs() {
  char* uuid_list;
  if (TangoService_getAreaDescriptionUUIDList(&uuid_list) != TANGO_SUCCESS) {
    LOGI("TangoService_getAreaDescriptionUUIDList");
  }
  LOGI("uuid list: %s", uuid_list);
}
