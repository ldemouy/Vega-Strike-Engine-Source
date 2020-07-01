
#ifndef _BASE_UTIL_H_
#define _BASE_UTIL_H_

#include <boost/version.hpp>
#include <memory>
#include <string>

#include <boost/python/dict.hpp>
#include <boost/python/object.hpp>
typedef boost::python::dict BoostPythonDictionary;

namespace boost
{
namespace python
{
class dict;
}
} // namespace boost

#include "audio/Stream.h"
#include "audio/Types.h"

namespace BaseUtil
{

typedef boost::python::dict Dictionary;

int32_t Room(std::string text);
void Texture(int32_t room, std::string index, std::string file, float x, float y);
bool Video(int32_t room, std::string index, std::string vfile, std::string afile, float x, float y);
bool VideoStream(int32_t room, std::string index, std::string streamfile, float x, float y, float w, float h);
void SetTexture(int32_t room, std::string index, std::string file);
void SetTextureSize(int32_t room, std::string index, float w, float h);
void SetTexturePos(int32_t room, std::string index, float x, float y);
void PlayVideo(int32_t room, std::string index);
void StopVideo(int32_t room, std::string index);
void SetVideoCallback(int32_t room, std::string index, std::string callback);
void SetDJEnabled(bool enabled);
void Ship(int32_t room, std::string index, QVector pos, Vector R, Vector Q);
void LinkPython(int32_t room, std::string index, std::string pythonfile, float x, float y, float wid, float hei,
                std::string text, int32_t to);
void LaunchPython(int32_t room, std::string index, std::string pythonfile, float x, float y, float wid, float hei,
                  std::string text);
void EjectPython(int32_t room, std::string index, std::string pythonfile, float x, float y, float wid, float hei,
                 std::string text);
void CompPython(int32_t room, std::string index, std::string pythonfile, float x, float y, float wid, float hei,
                std::string text, std::string modes);
void GlobalKeyPython(std::string pythonfile);

void Link(int32_t room, std::string index, float x, float y, float wid, float hei, std::string text, int32_t to);
void Launch(int32_t room, std::string index, float x, float y, float wid, float hei, std::string text);
void Comp(int32_t room, std::string index, float x, float y, float wid, float hei, std::string text, std::string modes);
void Python(int32_t room, std::string index, float x, float y, float wid, float hei, std::string text,
            std::string pythonfile, bool front = false);
void MessageToRoom(int32_t room, std::string text);
void EnqueueMessageToRoom(int32_t room, std::string text);
void Message(std::string text);
void EnqueueMessage(std::string text);
void RunScript(int32_t room, std::string ind, std::string pythonfile, float time);
void TextBox(int32_t room, std::string ind, std::string text, float x, float y, Vector widheimult, Vector backcol,
             float backalp, Vector forecol);
void SetTextBoxText(int32_t room, std::string ind, std::string text);
void SetLinkArea(int32_t room, std::string index, float x, float y, float wid, float hei);
void SetLinkText(int32_t room, std::string index, std::string text);
void SetLinkPython(int32_t room, std::string index, std::string python);
void SetLinkRoom(int32_t room, std::string index, int to);
void SetLinkEventMask(int32_t room, std::string index,
                      std::string maskdef); // c=click, u=up, d=down, e=enter, l=leave, m=move
void EraseLink(int32_t room, std::string index);
void EraseObj(int32_t room, std::string index);
int GetCurRoom();
void SetCurRoom(int32_t room);
bool HasObject(int32_t room, std::string index);
int32_t GetNumRoom();
bool BuyShip(std::string name, bool my_fleet, bool force_base_inventory);
bool SellShip(std::string name);

// Sound streaming
std::shared_ptr<Audio::Source> CreateVideoSoundStream(const std::string &afile, const std::string &scene);
void DestroyVideoSoundStream(std::shared_ptr<Audio::Source> source, const std::string &scene);

// GUI events
void SetEventData(Dictionary data);
void SetMouseEventData(std::string type, float x, float y,
                       int32_t buttonMask); //[type], [mousex], [mousey], [mousebuttons]
void SetKeyEventData(std::string type, uint32_t keycode, uint32_t modmask = ~0);
void SetKeyStatusEventData(uint32_t modmask = ~0);
const Dictionary &GetEventData();

// GUI events (engine internals)
Dictionary &_GetEventData();

// Auxiliary
float GetTextHeight(std::string text, Vector widheimult);
float GetTextWidth(std::string text, Vector widheimult);
void LoadBaseInterface(std::string name);
void LoadBaseInterfaceAtDock(std::string name, Unit *dockat, Unit *dockee);
void refreshBaseComputerUI(const class Cargo *dirtyCarg);
void ExitGame();
} // namespace BaseUtil

#endif
