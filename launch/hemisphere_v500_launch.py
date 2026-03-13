import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch.actions import ExecuteProcess, TimerAction

def generate_launch_description():
    # 1. Locate the config file from the share directory
    # Note: Make sure your config file is named 'config.yaml' inside the 'config' folder
    config_path = os.path.join(
        get_package_share_directory('hemisphere_gnss_v500_driver'),
        'config',
        'config.yaml'
    )

    # 2. Define the Driver Component
    hemisphere_node = ComposableNode(
        package='hemisphere_gnss_v500_driver',
        plugin='hemisphere_gnss_v500_driver::HemisphereDriverNode',
        name='hemisphere_driver',
        parameters=[config_path]
    )

    # 3. Create the Container to host the component
    container = ComposableNodeContainer(
        name='hemisphere_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[hemisphere_node],
        output='screen',
    )

    # 4. Automate Lifecycle: Configure
    # This moves the node from Unconfigured -> Inactive
    configure_cmd = ExecuteProcess(
        cmd=['ros2', 'lifecycle', 'set', '/hemisphere_driver', 'configure'],
        output='screen'
    )

    # 5. Automate Lifecycle: Activate
    # This moves the node from Inactive -> Active.
    # We wait 2 seconds to ensure 'configure' has fully completed.
    activate_cmd = TimerAction(
        period=2.0,
        actions=[
            ExecuteProcess(
                cmd=['ros2', 'lifecycle', 'set', '/hemisphere_driver', 'activate'],
                output='screen'
            )
        ]
    )

    return LaunchDescription([
        container,
        configure_cmd,
        activate_cmd
    ])
