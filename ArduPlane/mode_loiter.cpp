#include "mode.h"
#include "Plane.h"

bool ModeLoiter::_enter()
{
    plane.loiter.radius = abs(plane.aparm.loiter_radius);
    plane.loiter.direction = plane.aparm.loiter_radius < 0 ? -1 : 1;
    plane.loiter.navigate_last_ms = 0;
    plane.do_loiter_at_location();
    plane.setup_terrain_target_alt(plane.next_WP_loc);

    // make sure the local target altitude is the same as the nav target used for loiter nav
    // this allows us to do FBWB style stick control
    plane.set_target_altitude_current();

    plane.loiter_angle_reset();

    return true;
}

void ModeLoiter::update()
{
    plane.calc_nav_roll();
    plane.update_fbwb_speed_height();
}

bool ModeLoiter::isHeadingLinedUp(const Location loiterCenterLoc, const Location targetLoc)
{
    // Return true if current heading is aligned to vector to targetLoc.
    // Tolerance is initially 10 degrees and grows at 10 degrees for each loiter circle completed.

    const uint16_t loiterRadius = abs(plane.aparm.loiter_radius);
    if (loiterCenterLoc.get_distance(targetLoc) < loiterRadius + loiterRadius*0.05) {
        /* Whenever next waypoint is within the loiter radius plus 5%,
           maintaining loiter would prevent us from ever pointing toward the next waypoint.
           Hence break out of loiter immediately
         */
        return true;
    }

    // Bearing in centi-degrees
    const int32_t bearing_cd = plane.current_loc.get_bearing_to(targetLoc);
    return isHeadingLinedUp_cd(bearing_cd);
}


bool ModeLoiter::isHeadingLinedUp_cd(const int32_t bearing_cd)
{
    // Return true if current heading is aligned to bearing_cd.
    // Tolerance is initially 10 degrees and grows at 10 degrees for each loiter circle completed.

    // get current heading.
    const int32_t heading_cd = (wrap_360(degrees(plane.ahrs.groundspeed_vector().angle())))*100;

    const int32_t heading_err_cd = wrap_180_cd(bearing_cd - heading_cd);

    /*
      Check to see if the the plane is heading toward the land
      waypoint. We use 20 degrees (+/-10 deg) of margin so that
      we can handle 200 degrees/second of yaw.

      After every full circle, extend acceptance criteria to ensure
      aircraft will not loop forever in case high winds are forcing
      it beyond 200 deg/sec when passing the desired exit course
    */

    // Use integer division to get discrete steps
    const int32_t expanded_acceptance = 1000 * (labs(plane.loiter.sum_cd) / 36000);

    if (labs(heading_err_cd) <= 1000 + expanded_acceptance) {
        // Want to head in a straight line from _here_ to the next waypoint instead of center of loiter wp

        // 0 to xtrack from center of waypoint, 1 to xtrack from tangent exit location
        if (plane.next_WP_loc.loiter_xtrack) {
            plane.next_WP_loc = plane.current_loc;
        }
        return true;
    }
    return false;
}

void ModeLoiter::navigate()
{
    // update the WP alt from the global target adjusted by update_fbwb_speed_height
    plane.next_WP_loc.set_alt_cm(plane.target_altitude.amsl_cm, Location::AltFrame::ABSOLUTE);

    // manual loiter radius and direction control
    plane.update_loiter_radius_and_direction();

    plane.update_loiter(lrintf(plane.loiter.radius));
}
