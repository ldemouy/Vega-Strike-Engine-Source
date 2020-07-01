#include "gfx/camera.h"
#include "gfx/hud.h"
#include "gfx/matrix.h"
#include <list>
#include <vector>
using std::list;
using std::vector;
class BriefingOrder
{
  public:
    Vector vec;
    float speed;
    BriefingOrder(const Vector &v, const float s) : vec(v)
    {
        speed = s;
    }
};

class Briefing
{
  public:
    // Very reduced unit class
    class Ship
    {
        std::vector<class Mesh *> meshdata;
        Vector pos;
        std::list<BriefingOrder> orders;

      public:
        ~Ship();
        void Destroy();
        bool LoadFailed()
        {
            return meshdata.empty();
        }
        Vector Position()
        {
            return pos;
        } // return Vector (mat[12],mat[13],mat[14]);}
        void SetPosition(const Vector &pos)
        {
            this->pos = pos;
        }            // mat[12]=pos.i;mat[13]=pos.j;mat[14]=pos.k;}
        float cloak; // btw 0 and 1
        Ship(const char *filename, int32_t faction, const Vector &position);
        void Render(const Matrix &cam, double interpol);
        void Update();
        void OverrideOrder(const Vector &destination, float time);
        void EnqueueOrder(const Vector &destination, float time);
    };
    Camera cam;
    TextPlane tp;
    vector<Ship *> starships;
    void Render();
    void Update();
    //-1 returns file not found
    int AddStarship(const char *filename, int32_t faction, const Vector &);
    void RemoveStarship(uint32_t);
    void EnqueueOrder(uint32_t, const Vector &destination, float time);
    void OverrideOrder(uint32_t, const Vector &destination, float time);
    void SetPosition(uint32_t, const Vector &Position);
    Vector GetPosition(uint32_t);
    void SetCloak(uint32_t, float);
    Briefing();
    ~Briefing();
};
