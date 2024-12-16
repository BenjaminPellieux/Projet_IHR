#!/bin/bash
# Script to start Projet IHR nodes and launch files

# Load ROS environment
source /opt/ros/jazzy/setup.bash 
source /home/ros/Projet_IHR/install/setup.bash 

# Launch video publisher
ros2 run video_publisher video_pub_node &

# Launch rosbridge
ros2 launch rosbridge_server rosbridge_websocket_launch.xml &

# Wait for all background processes to complete
wait

