#include "rviz/visualmarker.h"
#include "rviz/visualmarker.h"
namespace ros{
	FootMarker::FootMarker(double x, double y, double yaw): RVIZVisualMarker(){
		this->g.setX(x);
		this->g.setY(y);
		this->g.setRPYRadian(0,0,yaw);
		this->g.setSX(0.24);
		this->g.setSY(0.14);
		this->g.setSZ(0.03);
		set_color(0.9,0.9,0.9,0.8);
		init_marker();
	}
	std::string FootMarker::name(){
		return std::string("foot");
	}
	uint32_t FootMarker::get_shape(){
		return visualization_msgs::Marker::CUBE;
	}
	void FootMarker::publish(){
		marker.header.frame_id = FRAME_NAME;
		marker.lifetime = ros::Duration(ROS_DURATION);
		rviz->publish(marker, true);
		if(textHover){
			visualization_msgs::Marker cmarker = createTextMarker();
			rviz->publish(cmarker);
		}
	}

	LeftFootMarker::LeftFootMarker(double x, double y, double tz): FootMarker(x,y,tz) {
		set_color(0.9,0.1,0.0,0.8);
		init_marker();
	}
	std::string LeftFootMarker::name(){
		return std::string("foot_L");
	}
	RightFootMarker::RightFootMarker(double x, double y, double tz): FootMarker(x,y,tz) {
		set_color(0.1,0.9,0.0,0.8);
		init_marker();
	}
	std::string RightFootMarker::name(){
		return std::string("foot_R");
	}
	ColorFootMarker::ColorFootMarker(double x, double y, double tz, const char *color): FootMarker(x,y,tz) {
		struct str2num {
			const char *str;
			double r,g,b,a;
		}registerMap[] = {
		    { "red"  , 1.0, 0.0, 0.0, 1.0 },
		    { "green", 0.0, 1.0, 0.0, 1.0 },
		    { "blue" , 0.0, 0.0, 1.0, 1.0 },
		    { "white" , 1.0, 1.0, 1.0, 1.0 },
		    { "yellow" , 1.0, 1.0, 0.0, 1.0 },
		    { NULL , 0.0, 0.0, 0.0, 0.0 }
		};

		int i;
		for (i=0; registerMap[i].str != NULL; i++){
			if(strcmp(color, registerMap[i].str)==0){
				c.r=registerMap[i].r;
				c.g=registerMap[i].g;
				c.b=registerMap[i].b;
				c.a=registerMap[i].a;
			}
		}
		init_marker();
	}
	std::string ColorFootMarker::name(){
		return std::string("foot_colored");
	}
}
