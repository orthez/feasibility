#pragma once
#include <vector>
#include <boost/thread.hpp>
#include "ros_util.h"

struct Environment{
protected:
	std::vector<ros::RVIZVisualMarker*> objects;
	ros::RVIZVisualMarker *goal;
	ros::RVIZVisualMarker *start;
	boost::shared_ptr<boost::thread> m_thread;
	bool changedEnv;

private:
	void publish(){
		ros::Rate r(10); //Hz
		while(1){
			bool change = false;
			std::vector<ros::RVIZVisualMarker*>::iterator obj;
			for(obj = objects.begin();obj!=objects.end();obj++){
				(*obj)->publish();
				change |= (*obj)->isChanged();
			}
			this->goal->publish();
			this->start->publish();
			change |= this->start->isChanged();
			change |= this->goal->isChanged();

			if(change && !changedEnv){
				changedEnv = true;
				ROS_INFO("environment changed!");
			}

			boost::this_thread::interruption_point();
			r.sleep();
		}
	}
protected:

	void startThread(){
		assert(!m_thread);
		ROS_INFO("starting environment thread");
		m_thread = boost::shared_ptr<boost::thread>(new boost::thread(&Environment::publish, this) );
	}

	virtual void setGoalObject() = 0;
		//ROS_INFO("Environment setGoalObject not yet implemented");
	virtual void setStartObject() = 0;
	virtual void setObjects() = 0;
public:
	bool isChanged(){
		if(changedEnv){
			changedEnv = false;
			return true;
		}else{
			return false;
		}
	}
	Environment(){
		goal = NULL;
		start = NULL;
		objects.empty();
		changedEnv = true;
	}

	void init(){
		setGoalObject();
		setStartObject();
		setObjects();
		CHECK(goal!=NULL, "goal state was not initialized in your Environment!");
		CHECK(start!=NULL, "start state was not initialized in your Environment!");
		CHECK(objects.size()>0, "objects were not initialized in your Environment!");
		startThread();
	}

	~Environment(){
		clean();
		if(this->m_thread!=NULL){
			this->m_thread->interrupt();
			std::string id = boost::lexical_cast<std::string>(this->m_thread->get_id());
			ROS_INFO("waiting for thread %s to terminate", id.c_str());
			this->m_thread->join();
		}
	}
	void clean(){
		std::vector<ros::RVIZVisualMarker*>::iterator obj;
		for(obj = objects.begin();obj!=objects.end();obj++){
			delete (*obj);
		}
		objects.clear();
	}
	std::vector<ros::RVIZVisualMarker*> getObjects(){
		return objects;
	}
	ros::Geometry getGoal(){
		assert(goal);
		return goal->g;
	}
	ros::Geometry getStart(){
		assert(start);
		return start->g;
	}
};

struct EnvironmentSalleBauzil: public Environment{
	EnvironmentSalleBauzil(): Environment(){
	}
	void setObjects(){
		//ros::TriangleObjectFloor *chair = new ros::TriangleObjectFloor(0.8, 0.5, "chairLabo.tris", "/evart/chair2/PO");

		ros::BlenderMeshTriangleObject *chair = new ros::BlenderMeshTriangleObject("package://feasibility/data/AluminumChair.dae","chairLabo.tris",0.8,0.5,0);
		chair->addText("/evart/chair2/PO");
		//ros::BlenderMesh *mesh = new ros::BlenderMesh("file://home/aorthey/git/feasibility/data/kellkore.stl",0,0,0);
		//mesh->addText("<Fresh Blender Export>");

		objects.push_back(chair);
	}
	void setGoalObject(){
		goal = new ros::SphereMarker(2.5, 1.5, 0.2, 0.0);
		goal->addText("goal");
		goal->subscribeToEvart("/evart/helmet2/PO");
	}
	void setStartObject(){
		//start = new ros::BlenderMesh("package://romeo_description/meshes/Torso.dae",0,0,0);
		//start->addText("start");
		//start->subscribeToEvart("/evart/robot/PO");
		start = new ros::SphereMarker(0.0, 0.0, 0.2, 0.0);
		start->addText("start");
		start->subscribeToEvart("/evart/robot/PO");


	}

};

