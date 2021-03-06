#ifndef _ASTEROID_H_
#define _ASTEROID_H_
#include "cmd/asteroid_generic.h"
#include "cmd/collection.h"
#include "cmd/script/flightgroup.h"
#include "cmd/unit.h"
#include "cmd/unit_factory.h"
#include "gfx/matrix.h"
#include "gfx/quaternion.h"
#include "gfx/vec.h"

class GameAsteroid : public GameUnit<Asteroid>
{
  public:
    virtual void UpdatePhysics2(const Transformation &trans, const Transformation &old_physical_state,
                                const Vector &accel, float difficulty, const Matrix &transmat,
                                const Vector &CumulativeVelocity, bool ResolveLast, UnitCollection *uc = nullptr);

  protected:
    /** Constructor that can only be called by the UnitFactory.
     */
    GameAsteroid(const char *filename, int faction, Flightgroup *fg = nullptr, int fg_snumber = 0,
                 float difficulty = .01);

    friend class UnitFactory;

  private:
    /// default constructor forbidden
    GameAsteroid();

    /// copy constructor forbidden
    GameAsteroid(const Asteroid &);

    /// assignment operator forbidden
    GameAsteroid &operator=(const Asteroid &);
};
#endif
