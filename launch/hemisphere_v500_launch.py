import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch.actions import ExecuteProcess, TimerAction, RegisterEventHandler
from launch.event_handlers import OnProcessStart

def generate_launch_description():
    # Use join for path safety
    pkg_share = get_package_share_directory('hemisphere_gnss_v500_driver')
    config_path = os.path.join(pkg_share, 'config', 'config.yaml')

    hemisphere_node = ComposableNode(
        package='hemisphere_gnss_v500_driver',
        plugin='hemisphere_gnss_v500_driver::HemisphereDriverNode',
        name='hemisphere_driver',
        parameters=[config_path]
    )

    container = ComposableNodeContainer(
        name='hemisphere_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[hemisphere_node],
        output='screen',
    )

    # 1. Configure the node 1 second after the container starts
    configure_event = TimerAction(
        period=1.5,
        actions=[
            ExecuteProcess(
                cmd=['ros2', 'lifecycle', 'set', '/hemisphere_driver', 'configure'],
                output='screen'
            )
        ]
    )

    # 2. Activate the node 3 seconds after the container starts
    activate_event = TimerAction(
        period=4.0,
        actions=[
            ExecuteProcess(
                cmd=['ros2', 'lifecycle', 'set', '/hemisphere_driver', 'activate'],
                output='screen'
            )
        ]
    )

    return LaunchDescription([
        container,
        configure_event,
        activate_event
    ])
