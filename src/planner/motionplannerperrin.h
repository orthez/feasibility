#include <fast-replanning/fast-replanning-interface.hh>
#include "motionplanner.h"

//Wrapper around fastReplanning library
struct MotionPlannerPerrin: public MotionPlanner{
	fastreplanning::FastReplanningInterface *planner;

	MotionPlannerPerrin(Environment &env, int &argc, char** &argv): MotionPlanner(env){
		std::string prefix = get_data_path();

		planner = fastreplanning::fastReplanningInterfaceFactory(prefix, argc, argv);
		planner->setVerboseLevel(0); //0 5 15
		planner->mainLoop(); //init
	}

	virtual void addObjectToPlanner(ros::RVIZVisualMarker *m){
		ros::Geometry *g = m->getGeometry();
		ros::TriangleObject *t = static_cast<ros::TriangleObject*>( m );
		//the z-value has to be >0.05 --- otherwise the planner does not find a solution
		planner->addAGenericPQPModel(t->pqp_margin, g->x, g->y, 0.05, 0.0, 0.0, 0.0); 
	}

	void start_planner(){
		planner->mainLoop();
	}

	void setStart( ros::Geometry &pos ){
		std::vector<double> start;
		start.push_back(pos.x); start.push_back(pos.y); start.push_back(pos.z);
		planner->updateLocalizationProtected(start);
		//ROS_INFO("set START TO  %f %f", pos.x , pos.y );
	}

	void setGoal( ros::Geometry &pos ){
		std::vector<double> goal;
		goal.push_back(pos.x); goal.push_back(pos.y); goal.push_back(pos.z);
		planner->update3DGoalPositionProtected(goal);
		//ROS_INFO("set GOAL TO  %f %f", pos.x , pos.y );
	}

	bool success(){
		std::vector<fastreplanning::footStepInterface> fsi;
		planner->getInterfaceSteps(fsi);
		return fsi.size()>0;
	}

	void publish(){
		std::vector<double> q = planner->getArticulatedValues();
		ROS_INFO("Articulated values: %d", q.size());
		if(q.size()>0){
			for(uint i=0;i<q.size();i++){
				ROS_INFO("%f", q.at(i));
			}
		}

		//obtain footsteps from planner
		std::vector<fastreplanning::footStepInterface> fsi;
		planner->getInterfaceSteps(fsi);
		if(fsi.size()>0){
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
				///////////
				double x = fsi.at(i).data.at(0);
				double y = fsi.at(i).data.at(1);
				double t = fsi.at(i).data.at(2);
				char foot = fsi.at(i).data.at(3);

				double newX = abs_x + cos(abs_t)*x-sin(abs_t)*y;
				double newY = abs_y + sin(abs_t)*x+cos(abs_t)*y;
				double newT = t - abs_t;

				abs_x=newX;
				abs_y=newY;
				abs_t=newT;

				if(foot == 'R'){
					ros::RightFootMarker f( newX, newY, newT);
					if(i==0) f.reset(); //clear all previous footsteps
					f.publish();
				}else{
					ros::LeftFootMarker f( newX, newY, newT);
					if(i==0) f.reset(); //clear all previous footsteps
					f.publish();
				}

				//ROS_INFO("new footstep [%d] at x=%f, y=%f, theta=%f", i,newX,newY,newT);
			}
		}

	}//publish

};