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

#define GLM_FORCE_RADIANS

#include <jni.h>
#include <sstream>
#include <stdlib.h>

#include "pathfinder.h"

#include "glm/gtx/string_cast.hpp"
#include "tango-gl/axis.h"
#include "tango-gl/camera.h"
#include "tango-gl/color.h"
#include "tango-gl/conversions.h"
#include "tango-gl/frustum.h"
#include "tango-gl/grid.h"
#include "tango-gl/trace.h"
#include "tango-gl/util.h"

#include "tango_data.h"

#include "tango-gl/goal_marker.h"

#include <fstream>





using namespace std;


template<typename T>
void log(std::string type,T o){
__android_log_print(ANDROID_LOG_INFO, "Debug message", type.c_str(), o);
}
void logv(std::string type){
__android_log_print(ANDROID_LOG_INFO, "Debug message", type.c_str(),"");
}


tango_gl::GoalMarker* marker;

const glm::vec3 kMarkerPosition = glm::vec3(3.0f, -3.0f, -0.80f);
glm::vec3 world_position = glm::vec3(0.0f, -1.4f, 0.0f);
  const tango_gl::Color kMarkerColor(1.0f, 0.f, 0.f);
const glm::quat kMarkerRotation = glm::quat(0.f, 0.f, 1.0f, 0.f);
const glm::vec3 kMarkerScale = glm::vec3(0.05f, 0.05f, 0.05f);
const glm::quat kZeroQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

glm::vec3 position_adf;
glm::vec3 valid_position_adf;

GLuint screen_width;
GLuint screen_height;

// Render camera's parent transformation.
// This object is a pivot transformtion for render camera to rotate around.
tango_gl::Transform* cam_parent_transform;

// Render camera.
tango_gl::Camera* cam;

// Device axis (in device frame of reference).
tango_gl::Axis* axis;

// Device frustum.
tango_gl::Frustum* frustum;

// Ground grid.
tango_gl::Grid* grid;

// Trace of motion tracking.
tango_gl::Trace* trace_motion;

// Trace of ADF.
tango_gl::Trace* trace_adf;

// Trace of trace_astar_sol.
tango_gl::Trace* trace_astar_sol;

// Trace of trace_astar.
tango_gl::Trace* trace_astar;

// Single finger touch positional values.
// First element in the array is x-axis touching position.
// Second element in the array is y-axis touching position.
float cam_start_angle[2];
float cam_cur_angle[2];

// Double finger touch distance value.
float cam_start_dist;
float cam_cur_dist;

enum CameraType {
  FIRST_PERSON = 0,
  THIRD_PERSON = 1,
  TOP_DOWN = 2
};
CameraType camera_type;

// Render and camera controlling constant values.
// Height offset is used for offset height of motion tracking
// pose data. Motion tracking start position is (0,0,0). Adding
// a height offset will give a more reasonable pose while a common
// human is holding the device. The units is in meters.
const glm::vec3 kHeightOffset = glm::vec3(0.0f, 1.3f, 0.0f);

// Render camera observation distance in third person camera mode.
const float kThirdPersonCameraDist = 7.0f;

// Render camera observation distance in top down camera mode.
const float kTopDownCameraDist = 5.0f;

// Zoom in speed.
const float kZoomSpeed = 10.0f;

// Min/max clamp value of camera observation distance.
const float kCamViewMinDist = 1.0f;
const float kCamViewMaxDist = 100.f;

// FOV set up values.
// Third and top down camera's FOV is 65 degrees.
// First person camera's FOV is 45 degrees.
const float kHighFov = 65.0f;
const float kLowFov = 45.0f;

// Color of the motion tracking trajectory.
const tango_gl::Color kTraceMotionColor(0.22f, 0.28f, 0.67f);
// Color of the motion tracking trajectory.
const tango_gl::Color kTraceADFColor(0.39f, 0.56f, 0.03f);
// Color of the astar tracking trajectory.
const tango_gl::Color kTraceASTARColor(0.9f, 0.0f, 0.9f);
// Color of the astar tracking trajectory.
const tango_gl::Color kTraceASTARSOLColor(0.9f, 0.0f, 0.03f);

// Color of the ground grid.
const tango_gl::Color kGridColor(0.85f, 0.85f, 0.85f);

// Frustum scale.
const glm::vec3 kFrustumScale = glm::vec3(0.4f, 0.3f, 0.5f);

// Astar tree.
AStarSearch<MapSearchNode> astarsearch;


// Set camera type, set render camera's parent position and rotation.
void SetCamera(CameraType camera_index) {
  camera_type = camera_index;
  cam_cur_angle[0] = cam_cur_angle[1] = cam_cur_dist = 0.0f;
  switch (camera_index) {
    case CameraType::FIRST_PERSON:
      cam->SetFieldOfView(kLowFov);
      cam_parent_transform->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam_parent_transform->SetRotation(glm::quat(1.0f, 0.0f, 0.0, 0.0f));
      break;
    case CameraType::THIRD_PERSON:
      cam->SetFieldOfView(kHighFov);
      cam->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      cam_cur_dist = kThirdPersonCameraDist;
      cam_cur_angle[0] = -M_PI / 4.0f;
      cam_cur_angle[1] = M_PI / 4.0f;
      break;
    case CameraType::TOP_DOWN:
      cam->SetFieldOfView(kHighFov);
      cam->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
      cam->SetRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
      cam_cur_dist = kTopDownCameraDist;
      cam_cur_angle[1] = M_PI / 2.0f;
      break;
    default:
      break;
  }
}

void PlaceMarker(glm::vec3 pos) {
  marker->SetPosition(pos);
}

glm::vec3 getNearestPosToPosStar(glm::vec3 pos){
TangoData tmp=TangoData::GetInstance();
unsigned int l=tmp.pos_astar.size();
glm::vec3 res=tmp.pos_astar[0];
float d = findDistance(tmp.pos_astar[0],pos);
float nd;
    for(int i=1;i<l;i++)
    {
        nd = findDistance(tmp.pos_astar[i],pos);

        if(nd<d){
            res=tmp.pos_astar[i];
            d=nd;
        }
    }
    return res;
}

void astar_callback(int f){
TangoData tmp=TangoData::GetInstance();
//if ( tmp.is_relocalized==false ) return;

      trace_astar_sol->ClearVertexArray();
		MapSearchNode nodeStart(getNearestPosToPosStar(position_adf));
		MapSearchNode nodeEnd(tmp.pos_astar[f]);
        PlaceMarker(tmp.pos_astar[f]+kHeightOffset);
		astarsearch.SetStartAndGoalStates( nodeStart, nodeEnd );
		unsigned int SearchState;
		do
		{
			SearchState = astarsearch.SearchStep();
            MapSearchNode *p = astarsearch.GetOpenListStart();
            while( p )p = astarsearch.GetOpenListNext();
            p = astarsearch.GetClosedListStart();
            while( p )p = astarsearch.GetClosedListNext();
		}
		while( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SEARCHING );

		if( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SUCCEEDED )
		{		MapSearchNode *node = astarsearch.GetSolutionStart();
         		for( ;; )
				{
				    node = astarsearch.GetSolutionNext();
					if(!node)break;
                    trace_astar_sol->UpdateVertexArray(node->toVec3());
				};
				astarsearch.FreeSolutionNodes();
		}
		astarsearch.EnsureMemoryFreed();
}
bool InitGlContent() {
  cam_parent_transform = new tango_gl::Transform();
  cam = new tango_gl::Camera();
  axis = new tango_gl::Axis();
  frustum = new tango_gl::Frustum();
  trace_motion = new tango_gl::Trace();
  trace_adf = new tango_gl::Trace();
  trace_astar = new tango_gl::Trace();
  trace_astar_sol = new tango_gl::Trace();

  TangoData::GetInstance().pos_adf.clear();
  grid = new tango_gl::Grid();

  marker = new tango_gl::GoalMarker();

  frustum->SetScale(kFrustumScale);

  // Set the parent-child camera transfromation.
  cam->SetParent(cam_parent_transform);

  SetCamera(CameraType::THIRD_PERSON);

  // Set trace's color to show motion tracking trajectory.
  trace_motion->SetColor(kTraceMotionColor);

  // Set trace's color to show motion tracking trajectory.
  trace_adf->SetColor(kTraceADFColor);

  // Set trace's color to show Astar solution trajectory.
  trace_astar_sol->SetColor(kTraceASTARSOLColor);

  // Set trace's color to show Astar solution trajectory.
  trace_astar->SetColor(kTraceASTARColor);

  grid->SetColor(kGridColor);

  marker->SetPosition(kMarkerPosition + world_position);
    marker->SetScale(kMarkerScale);
    marker->SetRotation(kMarkerRotation);
    marker->SetColor(kMarkerColor);


  return true;
}

bool SetupGraphics(int w, int h) {
  screen_width = w;
  screen_height = h;
  if (h == 0) {
    LOGE("Setup graphic height not valid");
    return false;
  }
  cam->SetAspectRatio(static_cast<float>(w) / static_cast<float>(h));
  return true;
}


// Render frustum and trace with current position and rotation
// updated from TangoData, TangoPosition and TangoRotation is updated via
// callback function OnPoseAvailable(), which is updated when new pose data
// is available.
bool RenderFrame() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glViewport(0, 0, screen_width, screen_height);

  grid->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f) - kHeightOffset);
  grid->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

  int pose_index =
      TangoData::GetInstance().current_pose_status[1] == TANGO_POSE_VALID ? 1
                                                                          : 0;

  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);

  glm::vec3 position = tango_gl::conversions::Vec3TangoToGl(
      TangoData::GetInstance().tango_position[pose_index]);


  glm::quat rotation = tango_gl::conversions::QuatTangoToGl(
      TangoData::GetInstance().tango_rotation[pose_index]);


  glm::vec3 position_motion = tango_gl::conversions::Vec3TangoToGl(
      TangoData::GetInstance().tango_position[0]);


  position_adf = tango_gl::conversions::Vec3TangoToGl(
      TangoData::GetInstance().tango_position[1]);


  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);

  if (camera_type == CameraType::FIRST_PERSON) {
    cam->SetPosition(position);
    cam->SetRotation(rotation);
  } else {
    // Get parent camera's rotation from touch.
    // Note that the render camera is a child transformation
    // of the this transformation.
    // cam_cur_angle[0] is the x-axis touch, cooresponding to y-axis rotation.
    // cam_cur_angle[0] is the y-axis touch, cooresponding to x-axis rotation.
    glm::quat parent_cam_rot =
        glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), -cam_cur_angle[0],
                    glm::vec3(0, 1, 0));
    parent_cam_rot =
        glm::rotate(parent_cam_rot, -cam_cur_angle[1], glm::vec3(1, 0, 0));

    // Set render camera parent position and rotation.
    cam_parent_transform->SetRotation(parent_cam_rot);
    cam_parent_transform->SetPosition(position);

    // Set camera view distance, based on touch interaction.
    cam->SetPosition(glm::vec3(0.0f, 0.0f, cam_cur_dist));

    frustum->SetPosition(position);
    frustum->SetRotation(rotation);
    frustum->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

    axis->SetPosition(position);
    axis->SetRotation(rotation);
    axis->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());
  }

  if (TangoData::GetInstance().is_relocalized) {
    trace_adf->UpdateVertexArray(position_adf);
   // __android_log_print(ANDROID_LOG_INFO, "MyTag", "The distance is %f", findDistance(TangoData::GetInstance().pos_adf[0],position_adf));
    if(findDistance(valid_position_adf,position_adf)>0.6f)
    {
    __android_log_print(ANDROID_LOG_INFO, "MyTag", "The distance is %f", findDistance(valid_position_adf,position_adf));
     TangoData::GetInstance().pos_adf.push_back(position_adf);

     valid_position_adf=position_adf;

    }

  } else {
    trace_motion->UpdateVertexArray(position_motion);
  }
  trace_adf->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());
  trace_motion->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());
  trace_astar_sol->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());
  trace_astar->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

  marker->SetRotation(kZeroQuat);
  marker->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());
  marker->SetRotation(kMarkerRotation);
  marker->Render(cam->GetProjectionMatrix(), cam->GetViewMatrix());

  return true;
}


void loadExtra(){
if (TangoData::GetInstance().cur_name.empty())return;
    char racine[20]="/sdcard/";
    std::string filepath;
    filepath+=racine;
    filepath+= TangoData::GetInstance().cur_name;
    ifstream ifs(filepath.c_str(), ios::binary);


    int nb;
    ifs.read((char *)&nb, sizeof(nb));
 __android_log_print(ANDROID_LOG_INFO, "MyTag", "sizeeee loadExtra %d",nb);
    for(int i=1;i<nb;i++){

    glm::vec3 v;
    ifs.read((char *)&v, sizeof(v));
    v.y=v.z;
    TangoData::GetInstance().pos_astar.push_back(v);

    //cout << p[i].x<<" " <<p[i].y<<" " <<p[i].z<<"\n";
    //__android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrra:  %f , %f , %f ",v.x,v.y,v.z );


    }

}



#ifdef __cplusplus
extern "C" {
#endif





JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_testfunc(
    JNIEnv*, jobject) {
astar_callback(5);
 }
JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_testfuncstring(
    JNIEnv* env, jobject obj) {
/*log("%d",50);
log("%f",50.6f);
*/

  return env->NewStringUTF( glm::to_string(position_adf).c_str());

 }


JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_initGlContent(
    JNIEnv*, jobject) {
  InitGlContent();
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_initialize(
    JNIEnv* env, jobject, jobject activity) {
  TangoErrorType err = TangoData::GetInstance().Initialize(env, activity);
  if (err != TANGO_SUCCESS) {
    if (err == TANGO_INVALID) {
      LOGE("Tango Service version mis-match");
    } else {
      LOGE("Tango Service initialize internal error");
    }
  }
  return static_cast<int>(err);
}


JNIEXPORT jboolean JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_connectCallbacks(
    JNIEnv*, jobject) {
  if (!TangoData::GetInstance().ConnectCallbacks()) {
    LOGE("Tango ConnectCallbacks() failed");
    return false;
  }

  return true;
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_setupConfigAndConnect(
    JNIEnv* env, jobject obj , bool is_learning, bool is_load_adf,jstring gpsvalues) {
  //LOGI("leanring:%d, adf:%d", is_learning, is_load_adf);

const char *gpsnativeString = env->GetStringUTFChars(gpsvalues, JNI_FALSE);


if (!TangoData::GetInstance().SetConfig(is_learning, is_load_adf,gpsnativeString)) {LOGE("Tango set config failed");}


//strcat(filename, strdup(.c_str() ) );


__android_log_print(ANDROID_LOG_INFO, "MyTag", "strrrrrrr loadExtra: %s ",gpsnativeString );

env->ReleaseStringUTFChars(gpsvalues, gpsnativeString);
int err =    TangoData::GetInstance().Connect();
    if (err != 0) {

          LOGE("Tango Service connect error");
    }

loadExtra();
}





JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_disconnect(
    JNIEnv*, jobject) {
  TangoData::GetInstance().Disconnect();
}


JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_freeGLContent(
    JNIEnv*, jobject) {
  delete cam;
  delete axis;
  delete grid;
  delete frustum;
  delete trace_adf;
  delete trace_motion;
  delete trace_astar_sol;
  delete trace_astar;
  TangoData::GetInstance().ResetData();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_setupGraphic(
    JNIEnv*, jobject, jint width, jint height) {
  SetupGraphics(width, height);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_render(
    JNIEnv*, jobject) {
  RenderFrame();
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_setCamera(
    JNIEnv*, jobject, int camera_index) {




  SetCamera(static_cast<CameraType>(camera_index));
}

JNIEXPORT bool JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_saveADF(
    JNIEnv*, jobject) {

// Save ADF.
  return TangoData::GetInstance().SaveADF();


}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_saveExtra(
    JNIEnv* env, jobject, jstring uuid) {
  const char* uuid_ = env->GetStringUTFChars(uuid, NULL);

    char filepath[128]="/sdcard/";
    strcat(filepath, strdup(uuid_));
    ofstream ofs(filepath, ios::binary);

    unsigned int len=TangoData::GetInstance().pos_adf.size();
    ofs.write((char *)&len, sizeof(len));
	//	   __android_log_print(ANDROID_LOG_INFO, "MyTag", "sizeeee save %d",len);

  for(unsigned int i=1;i<len;i++){
  glm::vec3 v=TangoData::GetInstance().pos_adf[i];
     ofs.write((char *)&v, sizeof(v));
  }

  TangoData::GetInstance().pos_adf.clear();
}


JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getUUID(
    JNIEnv* env, jobject) {
  return env->NewStringUTF(TangoData::GetInstance().cur_uuid.c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getAllUUIDs(
    JNIEnv* env, jobject) {
  return env->NewStringUTF(TangoData::GetInstance().GetAllUUIDs());
}

JNIEXPORT jint JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getADFCount(
    JNIEnv*, jobject) {
  return TangoData::GetInstance().adf_list.size();
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getUUIDMetadataValue(
    JNIEnv* env, jobject, jstring uuid, jstring metadata_key) {
  const char* uuid_ = env->GetStringUTFChars(uuid, NULL);
  const char* metadata_key_ = env->GetStringUTFChars(metadata_key, NULL);
  return env->NewStringUTF(
      TangoData::GetInstance().GetUUIDMetadataValue(uuid_, metadata_key_));
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_setUUIDMetadataValue(
    JNIEnv* env, jobject, jstring uuid, jstring metadata_key, jint value_size,
    jstring metadata_value) {
  const char* uuid_ = env->GetStringUTFChars(uuid, NULL);
  const char* metadata_key_ = env->GetStringUTFChars(metadata_key, NULL);
  const char* metadata_value_ = env->GetStringUTFChars(metadata_value, NULL);
  TangoData::GetInstance().SetUUIDMetadataValue(uuid_, metadata_key_,
                                                value_size, metadata_value_);
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_deleteADF(
    JNIEnv* env, jobject, jstring uuid) {
  const char* uuid_ = env->GetStringUTFChars(uuid, NULL);
  TangoData::GetInstance().DeleteADF(uuid_);
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getPoseString(
    JNIEnv* env, jobject, int index) {
  std::stringstream string_stream;
  if (TangoData::GetInstance().current_pose_status[index] == -4) {
    string_stream
        << "status: N/A, count: N/A, delta time (ms): N/A, position (m): "
           "N/A, orientation: N/A";
    return env->NewStringUTF(string_stream.str().c_str());
  }

  const char* status;
  switch (TangoData::GetInstance().current_pose_status[index]) {
    case TANGO_POSE_INITIALIZING:
      status = "initializing";
      break;
    case TANGO_POSE_VALID:
      status = "valid";
      break;
    case TANGO_POSE_INVALID:
      status = "invalid";
      break;
    case TANGO_POSE_UNKNOWN:
      status = "unknown";
      break;
    default:
      status = "status_code_invalid";
      break;
  }

  string_stream.setf(std::ios_base::fixed, std::ios_base::floatfield);
  string_stream.precision(3);
  pthread_mutex_lock(&TangoData::GetInstance().pose_mutex);
  string_stream << "status: " << status
                << ", count: " << TangoData::GetInstance().frame_count[index]
                << ", delta time (ms): "
                << TangoData::GetInstance().frame_delta_time[index]
                << ", position (m): ["
                << TangoData::GetInstance().tango_position[index].x << ", "
                << TangoData::GetInstance().tango_position[index].y << ", "
                << TangoData::GetInstance().tango_position[index].z << "]"
                << ", orientation: ["
                << TangoData::GetInstance().tango_rotation[index].x << ", "
                << TangoData::GetInstance().tango_rotation[index].y << ", "
                << TangoData::GetInstance().tango_rotation[index].z << ", "
                << TangoData::GetInstance().tango_rotation[index].w << "]";
  pthread_mutex_unlock(&TangoData::GetInstance().pose_mutex);
  return env->NewStringUTF(string_stream.str().c_str());
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getVersionString(
    JNIEnv* env, jobject) {
  if (TangoData::GetInstance().lib_version_string.empty()) {
    return env->NewStringUTF("No version string available.");
  } else {
    return env->NewStringUTF(
        TangoData::GetInstance().lib_version_string.c_str());
  }
}

JNIEXPORT jstring JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_getEventString(
    JNIEnv* env, jobject) {
  if (TangoData::GetInstance().event_string.empty()) {
    return env->NewStringUTF("No event string available.");
  } else {
    pthread_mutex_lock(&TangoData::GetInstance().event_mutex);
    std::string event_string_cpy = TangoData::GetInstance().event_string;
    pthread_mutex_unlock(&TangoData::GetInstance().event_mutex);
    return env->NewStringUTF(event_string_cpy.c_str());
  }
}

// Touching GL interface.
JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_startSetCameraOffset(
    JNIEnv*, jobject) {
  if (cam != NULL) {
    cam_start_angle[0] = cam_cur_angle[0];
    cam_start_angle[1] = cam_cur_angle[1];
    cam_start_dist = cam->GetPosition().z;
  }
}

JNIEXPORT void JNICALL
Java_com_projecttango_experiments_nativearealearning_TangoJNINative_setCameraOffset(
    JNIEnv*, jobject, float rotation_x, float rotation_y, float dist) {
  if (cam != NULL) {
    cam_cur_angle[0] = cam_start_angle[0] + rotation_x;
    cam_cur_angle[1] = cam_start_angle[1] + rotation_y;
    dist = tango_gl::util::Clamp(cam_start_dist + dist * kZoomSpeed,
                                 kCamViewMinDist, kCamViewMaxDist);
    cam_cur_dist = dist;
  }
}
#ifdef __cplusplus
}
#endif
