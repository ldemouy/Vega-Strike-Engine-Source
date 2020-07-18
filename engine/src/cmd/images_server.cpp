#include "cmd/images.h"

template <typename BOGUS> UnitImages<BOGUS>::~UnitImages()
{
    if (pExplosion)
    {
        delete pExplosion;
    }
    VSDESTRUCT1
}
