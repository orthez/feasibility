#include <algorithm> //max_element
#ifdef FCL_COLLISION_CHECKING
#include "fcl/traversal/traversal_node_bvhs.h"
#include "fcl/traversal/traversal_node_setup.h"
#include "fcl/collision_node.h"
#endif
#include "rviz/rviz_visualmarker.h"
#include <Eigen/Core>

#define DEBUG(x)

namespace ros{
	uint TrisTriangleObject::mesh_counter=0;
	TrisTriangleObject::TrisTriangleObject(): TriangleObject(){
	}
	TrisTriangleObject::TrisTriangleObject(std::string f, Geometry &in): TriangleObject() {
		init_object(f, in);
	}
	void TrisTriangleObject::reloadBVH(){
		this->bvh = NULL;
		this->bvh = new fcl::BVHModel< BoundingVolume >();
		this->tris2BVH(this->bvh, tris_file_name.c_str() );
	}

	void TrisTriangleObject::init_object( std::string f, Geometry &in ){
		this->g = in;

		double scale = 1.0;
		this->g.sx = scale;
		this->g.sy = scale;
		this->g.sz = scale;
		this->tris_file_name=f;

#ifdef PQP_COLLISION_CHECKING
		this->pqp_model = new PQP_Model;
		this->pqp_margin = new PQP_Model;
		this->tris2PQP( this->pqp_model, this->pqp_margin, tris_file_name.c_str() );
#endif
#ifdef FCL_COLLISION_CHECKING
		this->bvh = new fcl::BVHModel< BoundingVolume >();
		this->tris2BVH(this->bvh, tris_file_name.c_str() );
#endif
		this->tris2marker( this->marker, tris_file_name.c_str() );
		init_marker();

	}
	std::string TrisTriangleObject::name(){
		return std::string(basename(tris_file_name.c_str()));
	}
	uint32_t TrisTriangleObject::get_shape(){
		return visualization_msgs::Marker::TRIANGLE_LIST;
	}
	Color TrisTriangleObject::get_color(){
		return ros::WHITE;
	}
#ifdef PQP_COLLISION_CHECKING
	void TrisTriangleObject::tris2PQP(PQP_Model *m, const char *fname ){
		int ntris;

		FILE *fp = fopen_s(fname,"r");
		int res=fscanf(fp, "%d", &ntris);
		CHECK(res==1, fname);
		m->BeginModel();

		std::vector<fcl::Vec3f> vertices;
		std::vector<fcl::Triangle> triangles;
		double scale = 1.0; //bigger obstacles for planning phase
		for (int i = 0; i < ntris; i++){
			double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
			res=fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
			       &p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
			CHECK(res==9, "fscanf failed");

			PQP_REAL p1[3],p2[3],p3[3],p4[3],p5[3],p6[3];
			p1[0] = (PQP_REAL)scale*p1x; p1[1] = (PQP_REAL)scale*p1y; p1[2] = (PQP_REAL)scale*p1z;
			p2[0] = (PQP_REAL)scale*p2x; p2[1] = (PQP_REAL)scale*p2y; p2[2] = (PQP_REAL)scale*p2z;
			p3[0] = (PQP_REAL)scale*p3x; p3[1] = (PQP_REAL)scale*p3y; p3[2] = (PQP_REAL)scale*p3z;
			m->AddTri(p1,p2,p3,i);
			
		}
		m->EndModel();
		fclose(fp);

		DEBUG( ROS_INFO("[%s] created PQP object with %d triangles.\n", name().c_str(), m->num_tris);)

	}
	void TrisTriangleObject::tris2PQP(PQP_Model *m, PQP_Model *m_margin, const char *fname ){
		int ntris;

		FILE *fp = fopen_s(fname,"r");
		int res=fscanf(fp, "%d", &ntris);
		CHECK(res==1, fname);
		m->BeginModel();
		m_margin->BeginModel();

		std::vector<fcl::Vec3f> vertices;
		std::vector<fcl::Triangle> triangles;
		double scale = 1.0; //bigger obstacles for planning phase
		double scale_x = 1.1; //bigger obstacles for planning phase
		double scale_y = 1.1; //bigger obstacles for planning phase
		double scale_z = 1.1; //bigger obstacles for planning phase
		for (int i = 0; i < ntris; i++){
			double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
			res=fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
			       &p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
			CHECK(res==9, "fscanf failed");

			PQP_REAL p1[3],p2[3],p3[3],p4[3],p5[3],p6[3];
			p1[0] = (PQP_REAL)scale*p1x; p1[1] = (PQP_REAL)scale*p1y; p1[2] = (PQP_REAL)scale*p1z;
			p2[0] = (PQP_REAL)scale*p2x; p2[1] = (PQP_REAL)scale*p2y; p2[2] = (PQP_REAL)scale*p2z;
			p3[0] = (PQP_REAL)scale*p3x; p3[1] = (PQP_REAL)scale*p3y; p3[2] = (PQP_REAL)scale*p3z;
			m->AddTri(p1,p2,p3,i);
			p4[0] = (PQP_REAL)scale_x*p1x; p4[1] = (PQP_REAL)scale_y*p1y; p4[2] = (PQP_REAL)scale_z*p1z;
			p5[0] = (PQP_REAL)scale_x*p2x; p5[1] = (PQP_REAL)scale_y*p2y; p5[2] = (PQP_REAL)scale_z*p2z;
			p6[0] = (PQP_REAL)scale_x*p3x; p6[1] = (PQP_REAL)scale_y*p3y; p6[2] = (PQP_REAL)scale_z*p3z;
			m_margin->AddTri(p4,p5,p6,i);
			
		}
		m->EndModel();
		m_margin->EndModel();
		fclose(fp);

		DEBUG( ROS_INFO("[%s] created PQP object with %d triangles.\n", name().c_str(), m->num_tris);)

	}
#endif
#ifdef FCL_COLLISION_CHECKING
	void TrisTriangleObject::tris2BVH(fcl::BVHModel< BoundingVolume > *m, const char *fname ){
		
		int ntris;
		FILE *fp = fopen_s(fname,"r");
		int res=fscanf(fp, "%d", &ntris);
		//printf("%d\n",ntris);
		CHECK(res==1, fname);

		std::vector<fcl::Vec3f> vertices;
		std::vector<fcl::Triangle> triangles;
		for (int i = 0; i < 3*ntris; i+=3){
			double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
			res=fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
			       &p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
			CHECK(res==9, "fscanf failed");
			
			fcl::Vec3f a(p1x, p1y, p1z);
			fcl::Vec3f b(p2x, p2y, p2z);
			fcl::Vec3f c(p3x, p3y, p3z);
			vertices.push_back(a);
			vertices.push_back(b);
			vertices.push_back(c);

			fcl::Triangle t(i,i+1,i+2);
			triangles.push_back(t);
		}
		fclose(fp);

		bvh->bv_splitter.reset (new fcl::BVSplitter<BoundingVolume>(fcl::SPLIT_METHOD_MEAN));
		bvh->beginModel();
		bvh->addSubModel(vertices, triangles);
		bvh->endModel();
		//ROS_INFO("created object in FCL with %d triangles and %d vertices.\n", bvh->num_tris, bvh->num_vertices);
	}
#endif

	void TrisTriangleObject::tris2marker(visualization_msgs::Marker &marker, const char *fname){
		
		int ntris;
		FILE *fp = fopen_s(fname,"r");

		int res=fscanf(fp, "%d", &ntris);
		CHECK(res==1, fname);


		Color cc = get_color();
		std_msgs::ColorRGBA c;
		c.r = cc.r;
		c.g = cc.g;
		c.b = cc.b;
		c.a = cc.a;

		for (int i = 0; i < 3*ntris; i+=3){
			geometry_msgs::Point p;
			geometry_msgs::Point p1;
			geometry_msgs::Point p2;
			double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
			res=fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
			       &p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
			
			double sX = 1.0;
			double sY = 1.0;
			double sZ = 1.0;
			p.x = sX*p1x;p.y = sY*p1y;p.z = sZ*p1z;
			p1.x = sX*p2x;p1.y = sY*p2y;p1.z = sZ*p2z;
			p2.x = sX*p3x;p2.y = sY*p3y;p2.z = sZ*p3z;
			marker.points.push_back(p);
			marker.points.push_back(p1);
			marker.points.push_back(p2);
			marker.colors.push_back(c);
			marker.colors.push_back(c);
			marker.colors.push_back(c);
		}
		fclose(fp);
	}
	//Only load BVH structure for distance checking
	SweptVolumeObject::SweptVolumeObject(): TrisTriangleObject() {
	}
	SweptVolumeObject::SweptVolumeObject(std::string f, Geometry &in): TrisTriangleObject() {
		this->g = in;

		double scale = 1.0;
		this->g.sx = scale;
		this->g.sy = scale;
		this->g.sz = scale;
		this->tris_file_name=f;

		//this->bvh = new fcl::BVHModel< BoundingVolume >();
		//this->tris2BVH(this->bvh, tris_file_name.c_str() );
		this->pqp_model = new PQP_Model;
		this->tris2PQP( this->pqp_model, tris_file_name.c_str() );
		//this->tris2PQP(this->bvh, tris_file_name.c_str() );
		//this->tris2marker( this->marker, tris_file_name.c_str() );
		//init_marker();
	}

	TriangleObjectFloor::TriangleObjectFloor(double x, double y, std::string fname): TrisTriangleObject(){
		std::string object_file = get_tris_str(fname);

		Geometry object_pos;
		object_pos.x = x;
		object_pos.y = y;

		this->init_object(object_file, object_pos);
	}
	TriangleObjectFloor::TriangleObjectFloor(double x, double y, std::string fname, std::string mocap): TrisTriangleObject(){
		std::string object_file = get_tris_str(fname);

		Geometry object_pos;
		object_pos.x = x;
		object_pos.y = y;

		this->init_object(object_file, object_pos);
		this->subscribeToEvart( mocap );
	}
	TriangleObjectChair::TriangleObjectChair(std::string mocap): TrisTriangleObject(){
		std::string prefix = get_data_path();
		std::string chair_file = get_chair_str();
		Geometry chair_pos;
		chair_pos.x = 0.49;
		chair_pos.y = -0.1;
		chair_pos.z = 0.0;
		chair_pos.setYawRadian(0);
		this->init_object(chair_file, chair_pos);
		this->subscribeToEvart( mocap );
	}
	TriangleObjectRobot::TriangleObjectRobot(std::string mocap): TrisTriangleObject(){
		std::string prefix = get_data_path();
		std::string robot_file = get_robot_str();
		Geometry robot_pos;
		robot_pos.x = -2;
		robot_pos.y = 0;
		robot_pos.setYawRadian(0);
		this->init_object(robot_file, robot_pos);
		this->subscribeToEvart( mocap );
	}
}
