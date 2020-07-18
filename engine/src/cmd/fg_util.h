#include "gfx/cockpit_generic.h"
#include <string>
#include <vector>

namespace fg_util
{
std::string MakeFGKey(const std::string &fgname, int32_t faction);
uint32_t ShipListOffset();
uint32_t PerShipDataSize();
bool IsFGKey(const std::string &fgcandidate);
bool CheckFG(std::vector<std::string> &data);

} // namespace fg_util
