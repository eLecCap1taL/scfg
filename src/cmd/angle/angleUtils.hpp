#ifndef ANGLEUTILS
#define ANGLEUTILS
#include "../utils.hpp"
namespace ANGLE
{
    double lst_yaw;
    double lst_pitch;
    void useStdSensitivity()
    {
        event.pushEvent("sensitivity 2.2");
        event.pushEvent("m_yaw 0.022");
        event.pushEvent("m_pitch 0.022");
    }
    void useUserSensitivity()
    {
        event.pushEvent("hzCVAR_sens");
    }
    void resetAngle()
    {
        event.pushEvent("yaw 99999999999999999 1 1");
        event.pushEvent("pitch 99999999999999999 1 1");
        useStdSensitivity();
        lst_yaw = 0, lst_pitch = 0;
    }
    void relativeAngle(double yaw, double pitch)
    {
        event.pushEvent(format("yaw {:6f} 1 1", (double)(lst_yaw - yaw) / (sensitivity * m_yaw)));
        event.pushEvent(format("pitch {:6f} 1 1", (double)(pitch - lst_pitch) / (sensitivity * m_pitch)));
        useStdSensitivity();
        lst_yaw = yaw, lst_pitch = pitch;
    }
    void moveAngle(double yaw, double pitch, int t)
    {
        double d_yaw = yaw - lst_yaw;
        double d_pitch = pitch - lst_pitch;

        // choose a best direction for yaw
        if (yaw > lst_yaw)
        {
            double res = yaw - 180 + (-180 - lst_yaw);
            if (fabs(res) < fabs(d_yaw))
            {
                d_yaw = res;
            }
        }
        else
        {
            double res = yaw - (-180) + (180 - lst_yaw);
            if (fabs(res) < fabs(d_yaw))
            {
                d_yaw = res;
            }
        }

        // store angles
        lst_yaw = yaw;
        lst_pitch = pitch;

        string yaw_cmd = "";
        string pitch_cmd = "";

        if (d_yaw > 0)
            yaw_cmd = "turnleft";
        else
            yaw_cmd = "turnright";
        if (d_pitch > 0)
            pitch_cmd = "turndown";
        else
            pitch_cmd = "turnup";

        double yawspeed_need = fabs(d_yaw) / t;
        double pitchspeed_need = fabs(d_pitch) / t;

        if (yawspeed_need > max_yaw_speed || pitchspeed_need > max_pitch_speed)
        {
            luaL_error(luaL, "angle moves too fast!");
        }

        event.pushEvent(format("{} {:6f} 0 0", yaw_cmd, yawspeed_need / max_yaw_speed));
        event.pushEvent(format("{} {:6f} 0 0", pitch_cmd, pitchspeed_need / max_pitch_speed));

        event.sleep(t);

        event.pushEvent(format("-{}",yaw_cmd));
        event.pushEvent(format("-{}",pitch_cmd));
    }
}

//ANGLEUTILS
#endif