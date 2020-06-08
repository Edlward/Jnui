#include "Common.h"

QColor GetColor(int index)
{
    switch (index % CL_MAX) {
    case CL_BLUE:
        return QColor(0, 0, 255);
    case CL_AQUA:
        return QColor(0, 255, 255);
    case CL_BLUEVIOLET:
        return QColor(138, 43, 226);
    case CL_BROWN:
        return QColor(165, 42, 42);
    case CL_BURLYWOOD:
        return QColor(222, 184, 135);
    case CL_CADETBLUE:
        return QColor(95, 158, 160);
    case CL_CHARTREUSE:
        return QColor(127, 255, 0);
    case CL_CHOCOLATE:
        return QColor(210, 105, 30);
    case CL_DARKCYAN:
        return QColor(0, 139, 139);
    case CL_DARKORANGE:
        return QColor(255, 140, 0);
    case CL_RED:
        return QColor(255, 0, 0);
    case CL_YELLOW:
        return QColor(255, 255, 0);
    case CL_TAN:
        return QColor(210, 180, 140);
    case CL_SKYBLUE:
        return QColor(135, 206, 235);
    case CL_OLIVE:
        return QColor(128, 128, 0);
    case CL_MAGENTA:
        return QColor(255, 0, 255);
    case CL_LINEN:
        return QColor(250, 240, 230);
    default:
        return QColor(0, 0, 255);
    }
}

QString GetColorStyle(int index)
{
    switch (index % CL_MAX) {
    case CL_BLUE:
        return QString("rgb(0, 0, 255);");
    case CL_AQUA:
        return QString("rgb(0, 255, 255);");
    case CL_BLUEVIOLET:
        return QString("rgb(138, 43, 226);");
    case CL_BROWN:
        return QString("rgb(165, 42, 42);");
    case CL_BURLYWOOD:
        return QString("rgb(222, 184, 135);");
    case CL_CADETBLUE:
        return QString("rgb(95, 158, 160);");
    case CL_CHARTREUSE:
        return QString("rgb(127, 255, 0);");
    case CL_CHOCOLATE:
        return QString("rgb(210, 105, 30);");
    case CL_DARKCYAN:
        return QString("rgb(0, 139, 139);");
    case CL_DARKORANGE:
        return QString("rgb(255, 140, 0);");
    case CL_RED:
        return QString("rgb(255, 0, 0);");
    case CL_YELLOW:
        return QString("rgb(255, 255, 0);");
    case CL_TAN:
        return QString("rgb(210, 180, 140);");
    case CL_SKYBLUE:
        return QString("rgb(135, 206, 235);");
    case CL_OLIVE:
        return QString("rgb(128, 128, 0);");
    case CL_MAGENTA:
        return QString("rgb(255, 0, 255);");
    case CL_LINEN:
        return QString("rgb(250, 240, 230);");
    default:
        return QString("rgb(0, 0, 255);");
    }
}
