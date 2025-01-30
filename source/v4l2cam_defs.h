#ifndef V4L2CAM_DEFS_H
#define V4L2CAM_DEFS_H

#include <string>
#include <map>

enum v4l2cam_control_unit { v4l2_terminal_unit, v4l2_processing_unit };
enum v4l2cam_control_type { v4l2_integer, v4l2_boolean, v4l2_menu };

struct v4l2cam_control_defs {
    int ctrl_id;
    int ctrl_type;
    int unit;
    std::string name;
    int length;
    std::map<int, std::string> menuItems;
};

#endif // V4L2CAM_DEFS_H
