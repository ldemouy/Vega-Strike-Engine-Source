#include "communication.h"
#include "vs_globals.h"
#include "config_xml.h"
#include <assert.h>
#include "aldrv/audiolib.h"
#include "options.h"
#include <math.h>
#include <algorithm>

using namespace XMLSupport;
FSM::FSM(const std::string &filename)
{
    //loads a conversation finite state machine with deltaRelation weight transition from an XML?
    if (filename.empty())
    {
        nodes.push_back(Node::MakeNode("welcome to cachunkcachunk.com", 0));
        nodes.push_back(Node::MakeNode("I love you!", .1));
        nodes.push_back(Node::MakeNode("J00 0wnz m3", .08));
        nodes.push_back(Node::MakeNode("You are cool!", .06));
        nodes.push_back(Node::MakeNode("You are nice!", .05));
        nodes.push_back(Node::MakeNode("Ya you're naled! NALED PAL!", -.02));
        nodes.push_back(Node::MakeNode("i 0wnz j00", -.08));
        nodes.push_back(Node::MakeNode("I hate you!", -.1));

        nodes.push_back(Node::MakeNode("Docking operation complete.", 0));
        nodes.push_back(Node::MakeNode("Please move into a green docking box and press d.", 0));
        nodes.push_back(Node::MakeNode("Docking operation begun.", 0));
        nodes.push_back(Node::MakeNode("Clearance denied.", 0));
        nodes.push_back(Node::MakeNode("Clearance granted.", 0));
        nodes.push_back(Node::MakeNode("No.", 0));
        nodes.push_back(Node::MakeNode("Yes.", 0));
        nodes.push_back(Node::MakeNode("Prepare To Be Searched. Maintain Speed and Course.", 0));
        nodes.push_back(Node::MakeNode("No contraband detected: You may proceed.", 0));
        nodes.push_back(Node::MakeNode("Contraband detected! All units close and engage!", 0));
        nodes.push_back(Node::MakeNode("Your Course is deviating! Maintain Course!", 0));
        nodes.push_back(Node::MakeNode("Request Clearance To Land.", 0));
        nodes.push_back(Node::MakeNode("*hit*", -.2));
        vector<uint32_t> edges;

        for (size_t i = 0; i < nodes.size() - 13; i++)
        {
            edges.push_back(i);
        }
        for (size_t i = 0; i < nodes.size(); i++)
        {
            nodes[i].edges = edges;
        }
    }
    else
    {
        LoadXML(filename.c_str());
    }
}

int32_t FSM::GetUnDockNode() const
{
    return nodes.size() - 16;
}

int32_t FSM::GetFailDockNode() const
{
    return nodes.size() - 15;
}

int32_t FSM::GetDockNode() const
{
    return nodes.size() - 14;
}

int32_t FSM::GetUnAbleToDockNode() const
{
    return nodes.size() - 13;
}

int32_t FSM::GetAbleToDockNode() const
{
    return nodes.size() - 12;
}

int32_t FSM::GetNoNode() const
{
    return nodes.size() - 11;
}

int32_t FSM::GetYesNode() const
{
    return nodes.size() - 10;
}

int32_t FSM::GetContrabandInitiateNode() const
{
    return nodes.size() - 9;
}

int32_t FSM::GetContrabandUnDetectedNode() const
{
    return nodes.size() - 8;
}

int32_t FSM::GetContrabandDetectedNode() const
{
    return nodes.size() - 7;
}

int32_t FSM::GetContrabandWobblyNode() const
{
    return nodes.size() - 6;
}

int32_t FSM::GetRequestLandNode() const
{
    return nodes.size() - 5;
}

int32_t FSM::GetHitNode() const
{
    return nodes.size() - 1;
}

int32_t FSM::GetDamagedNode() const
{
    return nodes.size() - 2;
}

int32_t FSM::GetDealtDamageNode() const
{
    return nodes.size() - 3;
}

int32_t FSM::GetScoreKillNode() const
{
    return nodes.size() - 4;
}

bool nonneg(float i)
{
    return i >= 0;
}

std::string FSM::Node::GetMessage(uint32_t &multiple) const
{
    multiple = rand() % messages.size();
    return messages[multiple];
}

// createSound, implemented in unit_functions.cpp / libaudioserver.cpp
// FIXME: this variability makes it hard to use proper include files
extern int createSound(std::string file, bool val);

int32_t FSM::Node::GetSound(uint8_t sex, uint32_t multiple, float &gain)
{
    uint32_t index = multiple + ((uint32_t)sex) * messages.size();
    if (index < sounds.size())
    {
        gain = gains[index];
        if (sounds[index] < 0)
        {
            sounds[index] = createSound(soundfiles[index], false);
            AUDSoundGain(sounds[index], gains[index], false);
            return sounds[index];
        }
        else
        {
            return sounds[index];
        }
    }
    else
    {
        gain = 1.0f;
        return -1;
    }
}

bool FSM::Node::StopSound(uint8_t sex)
{
    uint32_t index = ((uint32_t)sex) * messages.size();
    bool ret = false;
    for (uint32_t i = index; i < index + messages.size() && i < sounds.size(); ++i)
    {
        if (sounds[i] > 0 && AUDIsPlaying(sounds[i]))
        {
            AUDStopPlaying(sounds[i]);
            ret = true;
        }
    }
    return ret;
}

bool FSM::StopAllSounds(uint8_t sex)
{
    bool ret = false;
    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        if (nodes[i].StopSound(sex))
        {
            ret = true;
        }
    }
    return ret;
}

void FSM::Node::AddSound(std::string soundfile, uint8_t sex, float gain)
{
    static std::string emptystr;

    for (int multiple = 0;; ++multiple)
    {
        uint32_t index = ((uint32_t)sex) * messages.size() + multiple;
        while (index >= sounds.size())
        {
            sounds.push_back(-1);
            soundfiles.push_back(emptystr);
            gains.push_back(1.0f);
        }

        if (soundfiles[index].empty())
        {
            soundfiles[index] = soundfile;
            gains[index] = gain;

            // Preload sound if configured to do so
            if (game_options.comm_preload)
            {
                GetSound(sex, multiple, gain);
            }

            break;
        }
    }
}

int32_t FSM::getCommMessageMood(int32_t curstate, float mood, float randomresponse, float relationship) const
{
    const FSM::Node *n = (uint32_t)curstate < nodes.size() ? (&nodes[curstate]) : (&nodes[getDefaultState(relationship)]);
    mood += -randomresponse + 2 * randomresponse * ((float)rand()) / RAND_MAX;

    int32_t choice = 0;

    vector<uint32_t> good;
    vector<uint32_t> bad;
    static float pos_limit = XMLSupport::parse_float(vs_config->getVariable("AI",
                                                                            "LowestPositiveCommChoice",
                                                                            "0"));
    static float neg_limit = XMLSupport::parse_float(vs_config->getVariable("AI",
                                                                            "LowestNegativeCommChoice",
                                                                            "-.00001"));
    for (uint32_t i = 0; i < n->edges.size(); i++)
    {
        float md = nodes[n->edges[i]].messagedelta;
        if (md >= pos_limit)
        {
            good.push_back(i);
        }
        if (md <= neg_limit)
        {
            bad.push_back(i);
        }
    }
    if (good.size() != 0 && (relationship > 0 || (bad.size() == 0)))
    {
        choice = good[(rand() % good.size())];
    }
    else if (bad.size())
    {
        choice = bad[rand() % bad.size()];
    }
    return choice;
}

int32_t FSM::getDefaultState(float relationship) const
{
    relationship = std::clamp(relationship, -1.0f, 1.0f);

    float mood = relationship;
    float randomresponse = .01;
    int32_t curstate = 0;

    const FSM::Node *n = &nodes[curstate];
    mood += -randomresponse + 2 * randomresponse * ((float)rand()) / RAND_MAX;

    int32_t choice = 0;
    float bestchoice = 16;
    bool fitmood = false;
    for (uint32_t i = 0; i < n->edges.size(); i++)
    {
        float md = nodes[n->edges[i]].messagedelta;
        bool newfitmood = nonneg(mood) == nonneg(md);
        if ((!fitmood) || newfitmood)
        {
            float newbestchoice = pow(md - mood, 2.0);
            if ((newbestchoice <= bestchoice) || (fitmood == false && newfitmood == true))
            {
                if ((newbestchoice == bestchoice && rand() % 2) || newbestchoice < bestchoice)
                {
                    //to make sure some variety happens
                    fitmood = newfitmood;
                    choice = i;
                    bestchoice = newbestchoice;
                }
            }
        }
    } //(0,relationship,.01)
    return nodes[0].edges[choice];
}

std::string FSM::GetEdgesString(uint32_t curstate)
{
    std::string retval = "\n";
    if (nodes.size() <= curstate)
    {
        fprintf(stderr, "Error with faction relationship due to %d not being in range of faction\n", curstate);
        return "\n1. Transmit Error\n2. Transmit Error\n3. Transmit Error\n";
    }
    for (uint32_t i = 0; i < nodes[curstate].edges.size(); i++)
    {
        retval += tostring((int32_t)((i + 1) % 10)) + "." + nodes[nodes[curstate].edges[i]].messages[0] + "\n";
    }
    static bool print_docking =
        XMLSupport::parse_bool(vs_config->getVariable("graphics", "hud", "print_request_docking", "true"));
    if (print_docking)
    {
        retval += "0. Request Docking Clearance";
    }
    return retval;
}

float FSM::getDeltaRelation(int32_t prevstate, uint32_t current_state) const
{
    if (nodes.size() <= current_state)
    {
        fprintf(stderr, "Error with faction relationship due to %d not being in range of faction\n", current_state);
        return 0;
    }
    return nodes[current_state].messagedelta;
}

void CommunicationMessage::Init(Unit *send, Unit *recv)
{
    if (send == nullptr)
    {
        return;
    }
    fsm = FactionUtil::GetConversation(send->faction, recv->faction);
    sender.SetUnit(send);
    this->prevstate = this->curstate = fsm->getDefaultState(send->getRelation(recv));
    this->edgenum = -1;
}

float roundclamp(float i)
{
    float j = std::round(i);
    return std::clamp(j, 0.0f, j);
}

void CommunicationMessage::SetAnimation(std::vector<Animation *> *ani, uint8_t sex)
{
    this->sex = sex; //for audio
    if (ani)
    {
        if (ani->size() > 0)
        {
            float mood = fsm->getDeltaRelation(this->prevstate, this->curstate);
            mood += .1;
            mood *= (ani->size()) / .2;
            uint32_t index = (uint32_t)roundclamp(floor(mood));
            if (index >= ani->size())
            {
                index = ani->size() - 1;
            }
            this->ani = (*ani)[index];
        }
        else
        {
            this->ani = nullptr;
        }
    }
    else
    {
        this->ani = nullptr;
    }
}

void CommunicationMessage::SetCurrentState(int32_t msg, std::vector<Animation *> *ani, uint8_t sex)
{
    curstate = msg;
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

CommunicationMessage::CommunicationMessage(Unit *send,
                                           Unit *recv,
                                           int32_t messagechoice,
                                           std::vector<Animation *> *ani,
                                           uint8_t sex)
{
    Init(send, recv);
    prevstate = fsm->getDefaultState(send->getRelation(recv));
    if (fsm->nodes[prevstate].edges.size())
    {
        this->edgenum = messagechoice % fsm->nodes[prevstate].edges.size();
        curstate = fsm->nodes[prevstate].edges[this->edgenum];
    }
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

CommunicationMessage::CommunicationMessage(Unit *send,
                                           Unit *recv,
                                           int32_t laststate,
                                           int32_t thisstate,
                                           std::vector<Animation *> *ani,
                                           uint8_t sex)
{
    Init(send, recv);
    prevstate = laststate;
    curstate = thisstate;
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

CommunicationMessage::CommunicationMessage(Unit *send, Unit *recv, std::vector<Animation *> *ani, uint8_t sex)
{
    Init(send, recv);
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

CommunicationMessage::CommunicationMessage(Unit *send,
                                           Unit *recv,
                                           const CommunicationMessage &prevstate,
                                           int32_t curstate,
                                           std::vector<Animation *> *ani,
                                           uint8_t sex)
{
    Init(send, recv);
    this->prevstate = prevstate.curstate;
    if (fsm->nodes[this->prevstate].edges.size())
    {
        this->edgenum = curstate % fsm->nodes[this->prevstate].edges.size();
        this->curstate = fsm->nodes[this->prevstate].edges[edgenum];
    }
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

char tohexdigit(int32_t x)
{
    if (x <= 9 && x >= 0)
    {
        return (char)(x + '0');
    }
    else
    {
        return (char)(x - 10 + 'A');
    }
}

RGBstring colorToString(GFXColor col)
{
    uint8_t r = (uint8_t)(col.r * 255);
    uint8_t g = (uint8_t)(col.g * 255);
    uint8_t b = (uint8_t)(col.b * 255);
    RGBstring ret;
    ret.str[0] = '#';
    ret.str[7] = '\0';
    ret.str[1] = tohexdigit(r / 16);
    ret.str[2] = tohexdigit(r % 16);
    ret.str[3] = tohexdigit(g / 16);
    ret.str[4] = tohexdigit(g % 16);
    ret.str[5] = tohexdigit(b / 16);
    ret.str[6] = tohexdigit(b % 16);
    return ret;
}

RGBstring GetRelationshipRGBstring(float rel)
{
    static GFXColor color_enemy = vs_config->getColor("relation_enemy", vs_config->getColor("enemy",
                                                                                            GFXColor(1.0, 0.0, 0.0, 1.0))); // red   - like target
    static GFXColor color_friend = vs_config->getColor("relation_friend", vs_config->getColor("friend",
                                                                                              GFXColor(0.0, 1.0, 0.0, 1.0))); // green - like target
    static GFXColor color_neutral = vs_config->getColor("relation_neutral", vs_config->getColor("black_and_white",
                                                                                                GFXColor(1.0, 1.0, 1.0, 1.0))); // white - NOT like target
    GFXColor color;
    if (rel == 0)
    {
        color = color_neutral;
    }
    else
    {
        if (rel < 0)
        {
            rel = -rel;
            color = color_enemy;
        }
        else
        {
            color = color_friend;
        }
        if (rel < 1.0)
        {
            color = colLerp(color_neutral, color, rel);
        }
    }
    return colorToString(color);
}

uint32_t DoSpeech(Unit *unit, Unit *player_unit, const FSM::Node &node)
{
    static float scale_rel_color =
        XMLSupport::parse_float(vs_config->getVariable("graphics", "hud", "scale_relationship_color", "10.0"));
    static std::string ownname_RGBstr = colorToString(vs_config->getColor("player_name", GFXColor(0.0, 0.2, 1.0))).str; // bluish
    uint32_t dummy = 0;
    string speech = node.GetMessage(dummy);
    string myname("[Static]");
    if (unit != nullptr)
    {
        myname = unit->isUnit() == PLANETPTR ? unit->name : unit->getFullname();
        Flightgroup *flight_group = unit->getFlightgroup();
        if (flight_group && flight_group->name != "base" && flight_group->name != "Base")
        {
            myname = flight_group->name + " " + XMLSupport::tostring(unit->getFgSubnumber()) + ", " + unit->getFullname();
        }
        else if (myname.length() == 0)
        {
            myname = unit->name;
        }
        if (player_unit != nullptr)
        {
            if (player_unit == unit)
            {
                myname = ownname_RGBstr + myname + "#000000";
            }
            else
            {
                float relationship = unit->getRelation(player_unit);
                myname = GetRelationshipColorString(relationship) + myname + "#000000";
            }
        }
    }
    mission->msgcenter->add(myname, "all",
                            GetRelationshipColorString(node.messagedelta * scale_rel_color) + speech + "#000000"); //multiply by 2 so colors are easier to tell
    return dummy;
}

void LeadMe(Unit *unit, string directive, string speech, bool changetarget)
{
    if (unit != nullptr)
    {
        for (size_t i = 0; i < _Universe->numPlayers(); i++)
        {
            Unit *parent_unit = _Universe->AccessCockpit(i)->GetParent();
            if (parent_unit)
            {
                if (parent_unit->getFlightgroup() == unit->getFlightgroup())
                {
                    DoSpeech(unit, parent_unit, FSM::Node::MakeNode(speech, .1));
                }
            }
        }
        Flightgroup *flight_group = unit->getFlightgroup();
        if (flight_group)
        {
            if (flight_group->leader.GetUnit() != unit)
            {
                if ((!_Universe->isPlayerStarship(flight_group->leader.GetUnit())) || _Universe->isPlayerStarship(unit))
                {
                    flight_group->leader.SetUnit(unit);
                }
            }
            flight_group->directive = directive;
            if (changetarget)
            {
                flight_group->target.SetUnit(unit->Target());
            }
            if ((directive == ""))
            {
                flight_group->target.SetUnit(nullptr);
            }
        }
    }
}
