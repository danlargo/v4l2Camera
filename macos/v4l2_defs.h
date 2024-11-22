#ifndef V4L2_DEFS_H
#define V4L2_DEFS_H

enum v4l2_control_unit { v4l2_terminal_unit, v4l2_processing_unit };
enum v4l2_control_type { v4l2_integer, v4l2_boolean, v4l2_menu };

struct v4l2_control_defs {
    int ctrl_id;
    int ctrl_type;
    int unit;
    std::string name;
    int length;
    std::map<int, std::string> menuItems;
};

#endif // V4L2_DEFS_H