// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "radar.h"
#include "bubble_display.h"
#include "null_display.h"
#include "plane_display.h"
#include "sensor.h"
#include "sphere_display.h"
#include <cassert>
#include <stdexcept>

namespace Radar
{

std::unique_ptr<Display> Factory(Type::Value type)
{
    switch (type)
    {
    case Type::NullDisplay:
        return std::unique_ptr<Display>(new NullDisplay);

    case Type::SphereDisplay:
        return std::unique_ptr<Display>(new SphereDisplay);

    case Type::BubbleDisplay:
        return std::unique_ptr<Display>(new BubbleDisplay);

    case Type::PlaneDisplay:
        return std::unique_ptr<Display>(new PlaneDisplay);

    default:
        assert(false);
        throw std::invalid_argument("Unknown radar type");
    }
}

} // namespace Radar
