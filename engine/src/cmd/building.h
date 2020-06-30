#ifndef __BUILDING_H
#define __BUILDING_H

#include "unit.h"
#include "building_generic.h"

class GameBuilding : public GameUnit<Building>
{
protected:
    GameBuilding(ContinuousTerrain *parent,
                 bool vehicle,
                 const char *filename,
                 bool SubUnit,
                 int32_t faction,
                 const std::string &unitModifications = std::string(""),
                 Flightgroup *fg = nullptr);
    GameBuilding(Terrain *parent,
                 bool vehicle,
                 const char *filename,
                 bool SubUnit,
                 int32_t faction,
                 const std::string &unitModifications = std::string(""),
                 Flightgroup *fg = nullptr);

    friend class UnitFactory;

public:
    virtual void UpdatePhysics2(const Transformation &trans,
                                const Transformation &oldtranssmat,
                                const Vector &,
                                float difficulty,
                                const Matrix &,
                                const Vector &CumulativeVelocity,
                                bool ResolveLast,
                                UnitCollection *uc = nullptr);

protected:
    /// default constructor forbidden
    GameBuilding();
    /// copy constructor forbidden
    GameBuilding(const Building &);
    /// assignment operator forbidden
    GameBuilding &operator=(const Building &);
};

#endif
