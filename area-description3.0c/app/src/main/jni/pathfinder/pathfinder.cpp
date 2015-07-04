#include "pathfinder.h"
#include "tango_data.h"


float findDistance(glm::vec3 x, glm::vec3 y)
{
    float distance, tempx, tempy, tempz;
    tempx = (x.x - y.x);
    tempx = tempx * tempx; //compiler _might_ be able to make this faster
    tempy = (x.y - y.y);
    tempy = tempy * tempy;
    tempz = (x.z - y.z);
    tempz = tempz * tempz;
    distance = tempx + tempy + tempz;
    distance = sqrt(distance);
    return distance;
}


void MapSearchNode::PrintNodeInfo()
{

    __android_log_print(ANDROID_LOG_INFO, "MyTag",  "Node position : (%f,%f,%f)", x,y,z   );

}


bool MapSearchNode::IsSameState( MapSearchNode &rhs )
{

	// same state in a maze search is simply when (x,y) are the same
    if( (x == rhs.x) &&
            (y == rhs.y) &&
            (z == rhs.z) )
	{
		return true;
	}
	else
	{
		return false;
	}

}


// Here's the heuristic function that estimates the distance from a Node
// to the Goal.

float MapSearchNode::GoalDistanceEstimate( MapSearchNode &nodeGoal )
{
    return fabsf(x - nodeGoal.x) + fabsf(y - nodeGoal.y) +fabsf(z - nodeGoal.z);
}

bool MapSearchNode::IsGoal( MapSearchNode &nodeGoal )
{

    if( (x == nodeGoal.x) &&
              (y == nodeGoal.y) &&
              (z == nodeGoal.z) )
	{
		return true;
	}

	return false;
}


// given this node, what does it cost to move to successor. In the case
// of our map the answer is the map terrain value at this node since that is
// conceptually where we're moving

float MapSearchNode::GetCost( MapSearchNode &successor )
{
    return 1.0f;

}

// This generates the successors to the given Node. It uses a helper function called
// AddSuccessor to give the successors to the AStar class. The A* specific initialisation
// is done for each node internally, so here you just set the state information that
// is specific to the application
bool MapSearchNode::GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node )
{

    float parent_x = -1;
    float parent_y = -1;
    float parent_z = -1;

	if( parent_node )
	{
		parent_x = parent_node->x;
		parent_y = parent_node->y;
        parent_z = parent_node->z;
	}


    glm::vec3 thisnodepos(x,y,z);
    TangoData tmp=TangoData::GetInstance();
	MapSearchNode NewNode;
	unsigned int len =tmp.pos_astar.size();
	    for (unsigned int i =0   ;     i<  len ;  i++){
            if(!((parent_x ==tmp.pos_astar[i].x) && (parent_y == tmp.pos_astar[i].y)&& (parent_z == tmp.pos_astar[i].z)) &&
                    !((tmp.pos_astar[i].x == x) && (tmp.pos_astar[i].y == y)&& (tmp.pos_astar[i].z == z)) &&
                    findDistance(glm::vec3(tmp.pos_astar[i].x,tmp.pos_astar[i].y,tmp.pos_astar[i].z),thisnodepos)<0.75f)
            {
                NewNode = MapSearchNode( tmp.pos_astar[i].x, tmp.pos_astar[i].y, tmp.pos_astar[i].z);
                astarsearch->AddSuccessor( NewNode );
            }
        }


    return true;
}