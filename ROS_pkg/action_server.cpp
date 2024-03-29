//用来接收moveit发出的路径规划结果



#include <ros/ros.h>
#include <actionlib/server/simple_action_server.h>
#include <control_msgs/FollowJointTrajectoryAction.h>
#include <trajectory_msgs/JointTrajectory.h>
using namespace std;

class JointTrajectoryActionServer
{
public:
JointTrajectoryActionServer(std::string name):
		as_(nh_, name, false), action_name_(name)
{
	// register callback for goal
	as_.registerGoalCallback(boost::bind(&JointTrajectoryActionServer::goalCallback, this));
	as_.start();
}
~JointTrajectoryActionServer(void){}

// when a trajectory command comes, this function will be called.
void goalCallback()
{
	boost::shared_ptr<const control_msgs::FollowJointTrajectoryGoal> goal;
	goal=as_.acceptNewGoal();
	cout<<"trajectory point size:"<< goal->trajectory.points.size()<<endl;
	// tell motion control hardware to execute
    // do something
	// when finished, return result
	as_.setSucceeded(result_);
}


// void goalCallback()
// {
// 	//do somethine else.
// 	control_msgs::FollowJointTrajectoryGoal::_trajectory_type trajectory;
// 	trajectory_msgs::JointTrajectory::_points_type::iterator iter;
// 	trajectory = as_.acceptNewGoal()->trajectory;
// 	for(iter= trajectory.points.begin(); iter!=trajectory.points.end(); iter++) 
// 		cout<<iter->positions[0]<<” ”;
// 	cout<<endl;
// 	for(iter=trajectory.points.begin(); iter!=trajectory.points.end(); iter++)
// 		cout<<iter->positions[1]<<””;
// 	cout<<endl;
// 	for(iter=trajectory.points.begin(); iter!=trajectory.points.end(); iter++)
// 		cout<<iter->time_from_start<<” ”;
// 	cout<<endl;
// 	//do somethine else. 
// }



protected:
ros::NodeHandle nh_;
actionlib::SimpleActionServer<control_msgs::FollowJointTrajectoryAction> as_;
actionlib::SimpleActionServer<control_msgs::FollowJointTrajectoryAction>::Result result_;
std::string action_name_;
};

int main(int argc, char** argv)
{
	ros::init(argc,argv, "arm_controller");
	JointTrajectoryActionServer srv("follow_joint_trajectory_controller/follow_joint_trajectory");
	ros::spin();
	return 0;
}
