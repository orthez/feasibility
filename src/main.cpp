#include <ros/ros.h>
#include <ros/time.h>
#include <std_msgs/String.h>
#include <visualization_msgs/Marker.h>
#include <vector>
#include <iostream>

#include <fcl/shape/geometric_shapes.h>
#include <fcl/math/vec_3f.h>
#include <fcl/BVH/BVH_model.h>

#include <fast-replanning/fast-replanning-interface.hh>

#include "ros_util.h"
#include "util.h"

int main( int argc, char** argv )
{
	ros::init(argc, argv, "footstep_planner");
	ros::NodeHandle n;
	ros::Rate r(1);

	std::string prefix = get_data_path();
	std::string chair_file = get_chair_str();
	std::string robot_file = get_robot_str();


	// TODO:
	// planner, objects (FCL), rviz
	//
	// classes needed: Environment (incl. object, FCLInterface), Visualizer
	// (RViz interface, connection to environment?), Planner (either
	// fast-replanner, obstacle-planner)


	//######################################################
	// set robot start
	//######################################################
	std::vector<double> start;
	start.push_back(0.0);
	start.push_back(0.0);
	start.push_back(0.0);
	SphereMarker ss(998, start.at(0), start.at(1), 0.05);
	ss.rviz_publish();

	double cx=0.49,cy=0.05,cz=0.0;
	double rx=1.0,ry=0.00,rz=0.0;
	TriangleObject chair(chair_file, cx, cy, cz);
	TriangleObject robot(robot_file, rx, ry, rz);

	//######################################################
	// fastPlanner
	//######################################################
	fastreplanning::FastReplanningInterface *planner 
		= fastreplanning::fastReplanningInterfaceFactory(prefix, argc, argv);

	planner->setVerboseLevel(0); //0 5 15
	planner->mainLoop(); //init

	planner->addObstacleFromDatabase(CHAIR, cx, cy, cz, 1,1,1); 
	//planner->addObstacleFromDatabase(BOX, 0.49, 0.21, 0, 1,1,1); 
	planner->updateLocalizationProtected(start);


	while (ros::ok())
	{

		std::vector<double> goal;
		goal.push_back(1.5); goal.push_back(-1.0); goal.push_back(0.0);
		planner->update3DGoalPositionProtected(goal);

		//Logger log("steps2.dat");

		ROS_INFO("foot step planner START->");
		planner->mainLoop();

		chair.rviz_publish();
		ROS_INFO("published chair and sv");
		
		std::vector<fastreplanning::footStepInterface> fsi;
		planner->getInterfaceSteps(fsi);
		ROS_INFO("NUMBER OF FOOTSTEPS: %d", fsi.size());

		double abs_x = 0.0;
		double abs_y = 0.0;
		double abs_t = 0.0;
		for(uint i=0;i<fsi.size();i++){

			//half-foot-step-format v.3.0:
			// 1: x
			// 2: y
			// 3: theta
			// 4: ascii code for L or R
			// 5-11: do not know
			// 
			/*
			for(uint j=0;j<fsi.at(i).data.size();j++){
				printf("%f - ",fsi.at(i).data.at(j));
			}
			printf("\n");
			log("%f %f %f %f %f %f %f %f %f %f %f", fsi.at(i).data.at(0), fsi.at(i).data.at(1), 
					fsi.at(i).data.at(2), fsi.at(i).data.at(3),
					fsi.at(i).data.at(4), fsi.at(i).data.at(5),
					fsi.at(i).data.at(6), fsi.at(i).data.at(7),
					fsi.at(i).data.at(8), fsi.at(i).data.at(9),
					fsi.at(i).data.at(10)
					);
			*/
			///////////
			double x = fsi.at(i).data.at(0);
			double y = fsi.at(i).data.at(1);
			double t = fsi.at(i).data.at(2);
			char foot = fsi.at(i).data.at(3);

			double newX = abs_x + cos(abs_t)*x-sin(abs_t)*y;
			double newY = abs_y + sin(abs_t)*x+cos(abs_t)*y;
			double newT = abs_t + t;
			FootStepObject f(i, newX,newY,newT);
			if(i==0) f.reset(); //clear all previous footsteps
			abs_x=newX;
			abs_y=newY;
			abs_t=newT;

			if(foot == 'R'){
				f.changeColor(1,0,0);
			}else{
				f.changeColor(0,1,0);
			}

			if(i==0 || i==1){
				f.changeColor(1,1,0);
			}
			if(i==fsi.size()-1 || i==fsi.size()-2){
				f.changeColor(1,0.6,0);
			}

			f.rviz_publish();
			ROS_INFO("new footstep [%d] at x=%f, y=%f, theta=%f", i,newX,newY,newT);
		}

		//######################################################
		// set goal 
		//######################################################
		std::vector<double> curgoal;
		planner->getCurrentSetGoal(curgoal);
		ROS_INFO("curgoal[%d]: %f %f", curgoal.size(), curgoal.at(0), curgoal.at(1));

		SphereMarker sm(999, goal.at(0), goal.at(1), 0.05);
		sm.rviz_publish();
		//######################################################

		r.sleep();
	}
}
