// source/editor/world/world.h
//
// A MinionRush level as a static scene in the editor's viewport. The game library loads the level the way the game does
// for the main menu (GS_Loading with GoTo_MainMenu, the path level.bclara is loaded by), then the menu state is dropped
// before anything runs. No gameplay state is ever entered: no minion, no HUD, no game camera, no level update. The view
// draws only the entities that have placed visual content in the level file (a Deco3d model, or an Object's deco). The
// editor's own orbit camera frames them; the game's camera controllers are never used.
//
// The game is a process-wide singleton, so one world can be opened per editor process. Close() only frees the frame
// and the sound system.
#pragma once

#include <string>
#include <vector>

namespace clara {
class Entity;
}

namespace jet::scene {
class Model;
}

namespace jet::video {
class Display;
}

namespace jeteditor::world {

// What an entity of the level file contributes to the view
enum class Content {
    NonVisual,  // no visual content: a camera, a light, a gameplay definition, a mount, an input definition
    Model,      // a model placed by the level file: drawn in the view at its transform
    Runtime,    // its content is not drawn: a level track template (built at gameplay start), or a class not ported yet
};

// An entity of the level file, as the file places it. Nested entities (the parameters of other entities) are not
// listed: only the entities of the file's folders.
struct LevelEntity {
    std::string name;   // the name in the level file; empty when it has none
    std::string type;   // the template the entity is an instance of, e.g. "ObstacleTemplate"
    float x = 0, y = 0, z = 0;  // the position of the entity as the level gives it (the transform the file gives, after links)
    bool placed = false;        // false for an entity that has no transform
    Content content = Content::NonVisual;
    std::string contentNote;    // what the view does with the entity and why, for the inspector and the log
    std::string model;          // the model's name when content is Model
    bool hasBounds = false;     // the model's box in world space (Model only)
    float boundsMin[3] = {0, 0, 0}, boundsMax[3] = {0, 0, 0};
    jet::scene::Model *drawn = nullptr;  // the model the view draws (Model only)
    clara::Entity *source = nullptr;     // the entity itself, for its transform; null when it is not a placed entity
};

class World {
public:
    // The frame's size: the phone's portrait screen, as the game's own screenshots (480x854). The panel shows it
    // scaled to fit.
    static constexpr unsigned kWidth = 480;
    static constexpr unsigned kHeight = 854;

    World() = default;
    ~World();
    World(const World &) = delete;
    World &operator=(const World &) = delete;

    // dataDir must hold levels/level.bclara and profiles.json. glDisplay is the display whose GL context is current;
    // the frame is drawn into it. The process working directory becomes dataDir. Loading takes some Update() calls.
    bool Open(const std::string &dataDir, jet::video::Display *glDisplay, std::string &error);
    void Close();
    bool IsOpen() const { return open_; }
    // True once the level is loaded and the menu state is dropped: from then on Update() only draws the static scene.
    bool IsLoaded() const { return loaded_; }

    // One editor frame: a loading step while the level loads, then the static scene.
    void Update();

    // The GL name of the last frame, 0 before the world is open.
    unsigned Texture() const;
    const std::string &DataDir() const { return dataDir_; }
    unsigned long long Frames() const { return frames_; }
    // The game state that is running: "GS_Loading" while the level loads, "no gameplay state" once it is loaded.
    std::string StateName() const;

    // The entities of the level file, as loaded.
    const std::vector<LevelEntity> &Entities() const { return entities_; }
    // The number of entities the view draws (content Model).
    size_t DrawnCount() const;
    // The selected entity's index in Entities(), -1 for none.
    int Selected() const { return selected_; }
    void Select(int index) { selected_ = index; }
    // The selected entity's box in frame pixels (origin top-left). False when nothing is selected, the entity is not
    // drawn, or its box is behind the camera.
    bool SelectedScreenBox(float &x0, float &y0, float &x1, float &y1) const;
    // The drawn entity under a point of the frame (frame pixels, origin top-left): the nearest one whose projected box
    // contains the point. -1 for none.
    int PickAt(float fx, float fy) const;
    // Sets one axis (0 x, 1 y, 2 z) of the selected entity's position in the level's data. Changes stay in memory.
    // False when nothing is selected or the entity has no transform.
    bool SetSelectedAxis(int axis, float value);

    // Orbit (dx, dy in pixels of the frame), zoom (notches, positive is in), and framing of the drawn entities.
    void Orbit(float dx, float dy);
    void Zoom(float notches);
    // Frames the box of the drawn entities; with none, the origin.
    void FrameLevel();
    // The orbit's target moves to the selected entity (its position, not its box)
    void FocusSelected();

private:
    void FinishLoading();
    void RenderEditFrame();
    void RefreshEntity(size_t index);

    bool open_ = false;
    bool loaded_ = false;
    std::string dataDir_;
    unsigned long long frames_ = 0;
    std::vector<LevelEntity> entities_;
    int selected_ = -1;
};

}  // namespace jeteditor::world
