#include "alphacurve.h"

//GET ALPHA CURVE
//***************************************************
//parametric Bezier curve spline for 3 points, 2 segments
//X(T) Y(T), where X(0) = X0, and X(1) = X1. <same for y>, hence no need for X calculations concernign Y return. but here just for kicks.
//***************************************************
int32_t get_alpha(int32_t query,
                  int32_t maxrez_x,
                  int32_t min,
                  int32_t max,
                  double focus,
                  double concavity,
                  int32_t tail_mode_start,
                  int32_t tail_mode_end)
{

    //FIX DATA
    if (min < 0)
    {
        min = 0; //error-test
    }
    if (max > 255)
    {
        max = 255; //error-test
    }
    if (max < min)
    {
        min = 0;
        max = 255;
    } //error-test
    if (concavity > 1.0)
    {
        concavity = 1; //error-test
    }
    if (concavity < -1.0)
    {
        concavity = -1; //error-test
    }
    if (query > maxrez_x)
    {
        query = maxrez_x; //error-test
    }
    if (query < 0)
    {
        query = 0; //error-test
    }
    if (focus < .2)
    {
        focus = .2; //error-test
    }
    if (focus > .8)
    {
        focus = .8; //error-test
    }
    //TAIL MODES CAN BE NEGATIVE BECAUSE THAT IS IGNORED
    //EXCESSIVE POSITICE VALUES WILL MAKE 255|0 RESULTS.
    //DEPENDING ON THE RESOLUTION AND LIMITS, DIFFERENT SLOPES WILL HIT THE RANGE AT DIFFERNT SETTINGS
    //I LEAVE THIS UNCAPPED, AND UP TO AN INTELLIGENT PERSON TO UNDERSTAND WHY IT BEHAVES SO IF THEY ENTER 10000000000
    int32_t half = int32_t(maxrez_x * focus); //half-the-work-point
    double _t = 0.0;
    if (query > half)
    {
        _t = double(query - half) / double(maxrez_x - half); //set parameter to second half
    }
    else
    {
        _t = double(query) / double(half); //set parameter to first half
    }
    int32_t center_y = int32_t(((double(max - min) / double(maxrez_x)) * (half)) + min);
    int32_t delta = 0; //difference from linear
    if (concavity < 0)
    {
        delta = max - center_y; //go down by concavity
    }
    else
    {
        delta = center_y - min; //go up by convexity (-concavity)
    }
    //FIX DATA

    //POINTS

    double y0 = min; //start point Y

    double y1 = center_y - (concavity * delta);

    double y2 = max; //end point Y
    if (y1 > max)
    {
        y1 = max; //error-test
    }
    if (y1 < min)
    {
        y1 = min; //error-test
    }
    //POINTS

    //SLOPES

    double vy0 = double(y1 - y0);

    concavity = fabs(concavity);

    double vy1 = (double(y2 - y0) / double(2)) * (double(1.0) - concavity);

    double vy2 = double(y2 - y1); //end point slope Y
    if (!(tail_mode_start < 0))
    {
        vy0 = tail_mode_start; //for over riding slopes 0 = flat, 100 or so = vertical
    }
    if (!(tail_mode_end < 0))
    {
        vy2 = tail_mode_end; //for over riding slopes
    }
    //SLOPES

    //INTERPOLATE
    int32_t yt = 0;
    if (query <= half)
    {
        double _t0 = _t;
        yt = int32_t(y0 + (vy0 * _t0) + (((3 * (y1 - y0)) - ((2 * vy0) + vy1)) * pow(_t0, 2)) + (((2 * (y0 - y1)) + (vy0 + vy1)) * pow(_t0, 3)));
    }
    else
    {
        double _t1 = _t;
        yt = int32_t(y1 + (vy1 * _t1) + (((3 * (y2 - y1)) - ((2 * vy1) + vy2)) * pow(_t1, 2)) + (((2 * (y1 - y2)) + (vy1 + vy2)) * pow(_t1, 3)));
    }
    int32_t return_alpha = yt;
    if (return_alpha < min)
    {
        return_alpha = min; //error-test
    }
    if (return_alpha > max)
    {
        return_alpha = max; //error-test
    }
    //INTERPOLATE

    return return_alpha;
}
//***************************************************
