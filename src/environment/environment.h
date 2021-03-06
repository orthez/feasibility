#pragma once
#include <vector>
#include <boost/thread.hpp>
#include "rviz/visualmarker.h"
#include "rviz/visualmarker.h"

struct Environment{
private:
	void thread_publish();
protected:
	std::vector<ros::RVIZVisualMarker*> objects;
	std::vector<ros::RVIZVisualMarker*> decorations;
	ros::RVIZVisualMarker *goal;
	ros::RVIZVisualMarker *start;
	boost::shared_ptr<boost::thread> m_thread;
	bool changedEnv;
	virtual void setGoalObject() = 0;
	virtual void setStartObject() = 0;
	virtual void setObjects() = 0;
	virtual void setDecorations(); //optional: set decoration objects, which are not considered for planning etc.

	void thread_start();
	void thread_stop();

	static Environment *singleton;
	Environment();
	virtual ~Environment();
public:
	static Environment* getInstance(const char* iname);
	static uint Nobjects;
	static void resetInstance();
	static Environment* getSalleBauzil();
	static Environment* getTestBed();
	static Environment* get13Humanoids();
	static Environment* get13HumanoidsReal();
	void init();
	void clean();
	bool isChanged();
	void reloadObjects();
	std::vector<ros::RVIZVisualMarker*> getObjects();
	void setGoal(double x, double y);
	void setStart(double x, double y);
	void setObjectPosition(uint id, double x, double y, double yaw);
	ros::Geometry getGoal();
	ros::Geometry getStart();
};

struct EnvironmentFeasibilityTest: public Environment{
	EnvironmentFeasibilityTest(): Environment(){ }
	void setObjects();
	void setGoalObject();
	void setStartObject();
	virtual void setDecorations();
};
struct Environment13HumanoidsReal: public Environment{
	Environment13HumanoidsReal(): Environment(){ }
	void setObjects();
	void setGoalObject();
	void setStartObject();
	virtual void setDecorations();
	void createApproxObj(const char* name, double x, double y, double yaw);
};
struct Environment13Humanoids: public Environment{
	Environment13Humanoids(): Environment(){ }
	void setObjects();
	void setGoalObject();
	void setStartObject();
	virtual void setDecorations();
};
struct EnvironmentSalleBauzil: public Environment{
	EnvironmentSalleBauzil(): Environment(){ }
	void setObjects();
	void setGoalObject();
	void setStartObject();
	virtual void setDecorations();
};

