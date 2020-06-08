#ifndef COMMON_H
#define COMMON_H

#pragma execution_character_set("utf-8")

#include <QColor>

const int MAX_2D_RECORD_COUNT = 1024;             // 2D子图数据条数最大值（一个图中显示多少条）
const int MAX_3D_RECORD_COUNT = 1024;             // 注意：按照一条数据5000个double值来算，占用内存为160MB
const int DEFAULT_2D_RECORD_COUNT = 200;          // 2D子图数据条数默认值（一个图中显示多少条）
const int DEFAULT_3D_RECORD_COUNT = 50;           // 3D子图数据条数默认值（一个图中显示多少条）
const int DEFAULT_2D_SUB_YAXIS_RANGE_START = 250; // 2D子图中Y轴的默认最小值
const int DEFAULT_2D_SUB_YAXIS_RANGE_END = 300;   // 2D子图中Y轴的默认最大值
const int MAX_MULTILINE_COUNT = 1024;             // 余晖模式下最多同时显示的线条数
const int DEFAULT_2D_Y2AXIS_START = -200;         // 设置基线后Y2轴的默认最小值
const int DEFAULT_2D_Y2AXIS_END = 400;            // 设置基线后Y2轴的默认最大值
const int DEFALUT_AXIS_STEP = 100;                // 时间子图时间轴最大值小于当前时间时，每次新增时间（单位s)
const int COLOR_SCALE_START = -120;               // 颜色图颜色条范围最小值
const int COLOR_SCALE_END = 40;                   // 颜色图颜色条范围最大值


const int GRAPHICS_SCENE_WIDTH = 300;             // 左边存放缩略图的scene宽度
const int GRAPHICS_ITEM_WIDTH = 270;              // item宽度
const int GRAPHICS_ITEM_HEIGHT = 200;             // item高度

const int PCM_PLAYER_BUFFER_STRIPE_SIZE = 16*1024;
const int PCM_PLAYER_BUFFER_COUNT = 8;

enum PLOT_TYPE
{
    PLOT_2D_MAIN,        // 曲线主图
    PLOT_2D_HIS_MAIN,    // 2D历史主图
    PLOT_3D_MAIN,        // 颜色主图
    PLOT_3D_HIS_MAIN,    // 历史数据3D颜色主图
    PLOT_2D_SUB,         // 曲线子图
    PLOT_2D_HIS_SUB,     // 2D历史子图  
    PLOT_3D_SUB_POS,     // 3D x轴是位置
    PLOT_3D_SUB_TIME,    // 3D x轴是时间
    PLOT_3D_HIS_POS,     // 历史数据3D x轴是位置
    PLOT_3D_HIS_TIME,    // 历史数据3D x轴是时间
    PLOT_BAIDUMAP,       // 百度地图模式
};

// 目前定义17种，参考以下网址：
// https://www.w3.org/TR/SVG11/types.html#ColorKeywords
enum COLOR_TYPE
{
    CL_BLUE = 1,    // rgb( 0, 0, 255)
    CL_AQUA,        // rgb( 0, 255, 255)
    CL_BLUEVIOLET,  // rgb(138, 43, 226)
    CL_BROWN,       // rgb(165, 42, 42)
    CL_BURLYWOOD,   // rgb(222, 184, 135)
    CL_CADETBLUE,   // rgb( 95, 158, 160)
    CL_CHARTREUSE,  // rgb(127, 255, 0)
    CL_CHOCOLATE,   // rgb(210, 105, 30)
    CL_DARKCYAN,    // rgb( 0, 139, 139)
    CL_DARKORANGE,  // rgb(255, 140, 0)
    CL_RED,         // rgb(255, 0, 0)
    CL_YELLOW,      // rgb(255, 255, 0)
    CL_TAN,         // rgb(210, 180, 140) = 13
    CL_SKYBLUE,     // rgb(135, 206, 235)
    CL_OLIVE,       // rgb(128, 128, 0)
    CL_MAGENTA,     // rgb(255, 0, 255)
    CL_LINEN,       // rgb(250, 240, 230) = 17
    CL_MAX,
};

QColor GetColor(int index);
QString GetColorStyle(int index);


#endif // COMMON_H
