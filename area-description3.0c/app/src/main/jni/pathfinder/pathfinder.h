#ifndef PATHFINDER_H_
#define PATHFINDER_H_
#include "stlastar.h"
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <string>
#include "tango-gl/util.h"
class MapSearchNode
{
public:
    float x;	 // the (x,y) positions of the node
    float y;
 	float z;
	glm::vec3 toVec3(){return glm::vec3(x,y,z);}
    MapSearchNode() { x = y = z = 0; }
    MapSearchNode(glm::vec3 p) { x=p.x; y=p.y; z=p.z;}
	MapSearchNode( float px, float py, float pz ) { x=px; y=py; z=pz;}


	float GoalDistanceEstimate( MapSearchNode &nodeGoal );
	bool IsGoal( MapSearchNode &nodeGoal );
	bool GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node );
	float GetCost( MapSearchNode &successor );
	bool IsSameState( MapSearchNode &rhs );

	void PrintNodeInfo();


};

float findDistance(glm::vec3 x, glm::vec3 y);
#endif
