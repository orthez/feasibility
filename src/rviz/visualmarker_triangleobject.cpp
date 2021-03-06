#ifdef FCL_COLLISION_CHECKING
#include <algorithm> //max_element
#include "fcl/traversal/traversal_node_bvhs.h"
#include "fcl/traversal/traversal_node_setup.h"
#include "fcl/collision_node.h"
#endif
#include "rviz/visualmarker.h"
#include "rviz/visualmarker.h"
#include <Eigen/Core>

#define DEBUG(x)

namespace ros{
	PQP_Model* TriangleObject::get_pqp_ptr(){
		return this->pqp_model;
	}

	TriangleObject::TriangleObject(){

	}

	TriangleObject::~TriangleObject(){
		DEBUG(ROS_INFO("delete TRIANGLEOBJECT");)
		if(pqp_model!=NULL){
			DEBUG(
				static int k=0;
				int i = pqp_model->MemUsage(true);
				ROS_INFO("pqp model %d mem usage %d", k++,i);
			)
			delete pqp_model;
			pqp_model=NULL;
		}
		if(bvh!=NULL){ delete bvh; bvh = NULL;}
	}
	std::string TriangleObject::name(){
		return std::string(basename(tris_file_name.c_str()));
	}
	uint32_t TriangleObject::get_shape(){
		return visualization_msgs::Marker::TRIANGLE_LIST;
	}
#ifdef FCL_COLLISION_CHECKING
	void TriangleObject::set_bvh_ptr( fcl::BVHModel< BoundingVolume > *bvh_in ){
		this->bvh = bvh_in;
	}
	fcl::BVHModel< BoundingVolume >* TriangleObject::get_bvh_ptr(){
		return this->bvh;
	}
#endif


	void TriangleObject::set_pqp_ptr( PQP_Model* pqp_in ){
		this->pqp_model = pqp_in;
	}
	double TriangleObject::pqp_distance_to(TriangleObject &rhs){
		this->g.lock();
		rhs.g.lock();

		//tried to stay as close as possible to the implementation in fast-replanning from Perrin,2012
		PQP_REAL R1[3][3], R2[3][3], T1[3], T2[3];
		double t = g.getYawRadian();
		double tr = rhs.g.getYawRadian();

		MRotZ(R1, t);
		MRotZ(R2, tr);

		T1[0] = g.getX();
		T1[1] = g.getY();
		T1[2] = g.getZ();

		T2[0] = rhs.g.getX();
		T2[1] = rhs.g.getY();
		T2[2] = rhs.g.getZ();

		this->g.unlock();
		rhs.g.unlock();

		PQP_CollideResult cres;
		DEBUG(ROS_INFO("[PQP] %d <-> %d", this->pqp_model->num_tris, rhs.pqp_model->num_tris);)
		PQP_Collide( &cres, R1, T1, this->pqp_model, R2, T2, rhs.pqp_model, PQP_FIRST_CONTACT);
		if(cres.Colliding()){
			return -1;
		}else{
			return 1;
		}
	}
	double TriangleObject::fast_distance_to(TriangleObject &rhs){
		
#ifdef FCL_COLLISION_CHECKING
		this->g.lock();
		rhs.g.lock();
		double t = g.getYawRadian();
		double tr = rhs.g.getYawRadian();
		fcl::Matrix3f r1 (cos(t),-sin(t),0,
				  sin(t),cos(t) ,0,
				  0     ,0      ,1);
		fcl::Matrix3f r2 (cos(tr),-sin(tr),0,
				  sin(tr),cos(tr) ,0,
				  0     ,0      ,1);

		fcl::Vec3f d1(g.getX(),g.getY(),g.getZ());
		fcl::Vec3f d2(rhs.g.getX(),rhs.g.getY(),rhs.g.getZ());
		this->g.unlock();
		rhs.g.unlock();

		fcl::Transform3f Tlhs(r1, d1);
		fcl::Transform3f Trhs(r2, d2);

		//########################################################################
		//distance checker
		//########################################################################
		fcl::DistanceRequest request;
		fcl::DistanceResult result;
		double d = fcl::distance (this->bvh, Tlhs, rhs.bvh, Trhs, request, result);

		return d;

#else
		ABORT("This executable is not compiled with the FCL collision library");
#endif
	}
	double TriangleObject::distance_to(TriangleObject &rhs, Eigen::VectorXd &derivative){
		//rotation z-axis (as visualized in rviz)
		
#ifdef FCL_COLLISION_CHECKING
		double t = g.getYawRadian();
		double tr = rhs.g.getYawRadian();
		fcl::Matrix3f r1 (cos(t),-sin(t),0,
				  sin(t),cos(t) ,0,
				  0     ,0      ,1);
		fcl::Matrix3f r2 (cos(tr),-sin(tr),0,
				  sin(tr),cos(tr) ,0,
				  0     ,0      ,1);

		fcl::Vec3f d1(g.getX(),g.getY(),g.getZ());
		fcl::Vec3f d2(rhs.g.getX(),rhs.g.getY(),rhs.g.getZ());

		fcl::Transform3f Tlhs(r1, d1);
		fcl::Transform3f Trhs(r2, d2);

		DEBUG(ROS_INFO("objects positions %f %f %f and %f %f %f", g.getX(), g.getY(), g.getZ(), rhs.g.getX(), rhs.g.getY(), rhs.g.getZ());)

		//########################################################################
		//distance checker
		//########################################################################
		fcl::DistanceRequest request;
		request.enable_nearest_points = true;
		fcl::DistanceResult result;
		double d = fcl::distance (this->bvh, Tlhs, rhs.bvh, Trhs, request, result);
		fcl::Vec3f np[2] = result.nearest_points;

		//double dn = sqrtf(fcl::details::dot_prod3(np[0].data, np[1].data)); //distance between nearest points

		double rx = np[0][0];
		double ry = np[0][1];
		double rz = np[0][2];
		double ox = np[1][0];
		double oy = np[1][1];
		double oz = np[1][2];

		double dn = sqrtf( (rx-ox)*(rx-ox) + (ry-oy)*(ry-oy) +(rz-oz)*(rz-oz));
		//double dn = sqrtf( (ox-rx)*(ox-rx) + (oy-ry)*(oy-ry) +(oz-rz)*(oz-rz));
		//double dx = (ox-rx)/dn; double dy = (oy-ry)/dn; double dz = (oz-rz)/dn;

		DEBUG(ROS_INFO("norm: %f -- distance %f", dn,d);)
		if(dn < 0.001){
			DEBUG(ROS_INFO("nearest points: %f %f %f -- %f %f %f", rx, ry, rz, ox, oy, oz);)
			derivative << 0.0, 0.0, 0.0, 0.0;

		}else{
			//double dd = computeDerivativeFromNearestPoints( np[0], np[1] );
			double dx = (rx-ox)/dn; double dy = (ry-oy)/dn; double dz = (rz-oz)/dn;
			DEBUG(ROS_INFO("nearest points: %f %f %f -- %f %f %f", rx, ry, rz, ox, oy, oz);)

			derivative << dx, dy, 0.0, 0.0;
			//let d be the distance between nearest points
			//d = dn;
		}


		//########################################################################
		///collision checker (returns penetration depth)
		//########################################################################
		fcl::CollisionResult cresult;
		fcl::CollisionRequest crequest;
		crequest.enable_cost=true;
		crequest.enable_contact = true;
		crequest.use_approximate_cost = false;

		fcl::collide (this->bvh, Tlhs, rhs.bvh, Trhs, crequest, cresult);

		uint numContacts = cresult.numContacts();
		std::vector<double> pd(numContacts);
		for (unsigned int j = 0; j < numContacts; ++j) {
			const fcl::Contact& contact = cresult.getContact(j);
			fcl::Vec3f n = contact.normal;
			derivative[0] = n[0];
			derivative[1] = n[1];
			pd.push_back(contact.penetration_depth);
			DEBUG(ROS_INFO("contact %d depth %f global dist %f",j,contact.penetration_depth,d);)
		}
		double returnValue=d;
		if(pd.empty()){
		}else{
			double maxd= *std::max_element(pd.begin(), pd.end());
			DEBUG(ROS_INFO("max depth %f",maxd);)
			returnValue=-10*maxd;
		}
		DEBUG(ROS_INFO("gradient: %f %f %f", derivative[0],derivative[1],derivative[2]);)
		DEBUG(ROS_INFO("return value %f",returnValue);)
		return returnValue;


#else
		ABORT("This executable is not compiled with the FCL collision library");
#endif
	}
	double computeDerivativeFromNearestPoints( fcl::Vec3f &a, fcl::Vec3f &b){
          return 0;
	}
	void TriangleObject::tris2marker(visualization_msgs::Marker &marker, const char *fname, bool mirror_y){
		double sm=1;
		if(mirror_y){
			sm = -1;
		}
		
		int ntris;
		FILE *fp = fopen_s(fname,"r");

		int res=fscanf(fp, "%d", &ntris);
		CHECK(res==1, fname);

		ROS_INFO("loading tris file /w %d triangles", ntris);

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
			double sY = sm*1.0;
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
}
