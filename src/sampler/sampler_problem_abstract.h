#pragma once
#include <Eigen/Core>

struct Proposal{
	Eigen::VectorXd q_stddev;
	Eigen::VectorXd q_constraints_low;
	Eigen::VectorXd q_constraints_high;
	Eigen::VectorXd operator()( Eigen::VectorXd &x );
	Eigen::VectorXd init();
	Eigen::VectorXd init_middle();
	Eigen::VectorXd init_rand();
};
struct ProbabilityDistribution{
	virtual double operator()(double d) = 0;
};
struct ObjectiveFunction{
	virtual double operator()(Eigen::VectorXd &x) = 0; //evaluate E(x)
	virtual Eigen::VectorXd grad(Eigen::VectorXd &x) = 0; //evaluate E(x)
	virtual void update( Eigen::VectorXd &x ) = 0; //update internal structures according to x, if necesary
};
struct AbstractSamplingProblem{
	ObjectiveFunction *E;
	ProbabilityDistribution *p;
	Proposal *q;

	virtual ObjectiveFunction* getObjectiveFunction() = 0;
	virtual ProbabilityDistribution* getProbabilityDistribution() = 0;
	virtual Proposal* getProposal() = 0;
};
struct SamplingCTOCylinder: public AbstractSamplingProblem{
	char *robot;
	SamplingCTOCylinder(char *argv, double h); 
	SamplingCTOCylinder(char *argv, double h=0.0, double m=0.0);
	ObjectiveFunction* getObjectiveFunction();
	ProbabilityDistribution* getProbabilityDistribution();
	ProbabilityDistribution* getProbabilityDistribution(double m);
	Proposal* getProposal(double h);
	Proposal* getProposal();
};
