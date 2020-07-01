#include "base_util.h"
#include "unit_generic.h"
#include <stdlib.h>
#include <string>
namespace BaseUtil
{
int32_t Room(std::string text)
{
    return 0;
}
void Texture(int32_t room, std::string index, std::string file, float x, float y)
{
}
bool Video(int32_t room, std::string index, std::string file, float x, float y)
{
    return false;
}
bool VideoStream(int32_t room, std::string index, std::string file, float x, float y, float w, float h)
{
    return false;
}
void PlayVideo(int32_t room, std::string index)
{
}
void StopVideo(int32_t room, std::string index)
{
}
void SetDJEnabled(bool enabled)
{
}
void Ship(int32_t room, std::string index, QVector pos, Vector Q, Vector R)
{
}
void Link(int32_t room, std::string index, float x, float y, float wid, float hei, std::string text, int32_t to)
{
    LinkPython(room, index, "", x, y, wid, hei, text, to);
}
void LinkPython(int32_t room, std::string index, std::string pythonfile, float x, float y, float wid, float hei,
                std::string text, int32_t to)
{
}
void Launch(int32_t room, std::string index, float x, float y, float wid, float hei, std::string text)
{
    LaunchPython(room, index, "", x, y, wid, hei, text);
}
void LaunchPython(int32_t room, std::string index, std::string pythonfile, float x, float y, float wid, float hei,
                  std::string text)
{
}
void EjectPython(int32_t room, std::string index, std::string pythonfile, float x, float y, float wid, float hei,
                 std::string text)
{
}
void Comp(int32_t room, std::string index, float x, float y, float wid, float hei, std::string text, std::string modes)
{
    CompPython(room, index, "", x, y, wid, hei, text, modes);
}
void CompPython(int32_t room, std::string index, std::string pythonfile, float x, float y, float wid, float hei,
                std::string text, std::string modes)
{
}
void Python(int32_t room, std::string index, float x, float y, float wid, float hei, std::string text,
            std::string pythonfile)
{
}
void Message(std::string text)
{
}
void EnqueueMessage(std::string text)
{
}
void EraseLink(int32_t room, std::string index)
{
}
void EraseObj(int32_t room, std::string index)
{
}
int32_t GetCurRoom()
{
    return 0;
}
int32_t GetNumRoom()
{
    return 1;
}
bool HasObject(int32_t room, std::string index)
{
    return false;
}
void refreshBaseComputerUI(const class Cargo *carg)
{
}
} // namespace BaseUtil
