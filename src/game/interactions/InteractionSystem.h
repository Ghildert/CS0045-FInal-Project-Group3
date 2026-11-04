#ifndef GAME_INTERACTIONS_INTERACTIONSYSTEM_H
#define GAME_INTERACTIONS_INTERACTIONSYSTEM_H

#include <string>
#include <vector>

namespace game {
namespace interactions {

struct InteractableObject {
    std::string id;
    std::string displayName;
    std::string promptLabel;    // e.g. "[E] Talk to Maya" or "[E] Inspect Sketchbook"
    std::string dialogueNodeId; // Dialogue sequence to trigger
    float       worldX;
    float       worldY;
    float       worldZ;
    float       triggerRadius;
    int         interactCount;
};

class InteractionManager {
public:
    InteractionManager();

    void clear();
    void registerInteractable(
        const std::string& id,
        const std::string& displayName,
        const std::string& promptLabel,
        const std::string& dialogueNodeId,
        float worldX, float worldY, float worldZ,
        float triggerRadius = 1.25f);

    void update(float playerX, float playerZ);
    const InteractableObject* getActiveInteractable() const;
    InteractableObject* getActiveInteractableMutable();

    // Renders a bobbing 3D interaction marker diamond & stippled ring on the active target
    void render3DIndicators(float timeSeconds, bool showAllDebug = false) const;

    // Renders a 2D prompt pill at the bottom-center of the screen when near an interactable
    void render2DPrompt(int screenWidth, int screenHeight) const;

    int getTotalInteractionsTriggered() const { return m_totalInteractions; }
    void incrementInteractionCount(InteractableObject* obj);

private:
    std::vector<InteractableObject> m_items;
    int                             m_activeIndex;
    int                             m_totalInteractions;
};

} // namespace interactions
} // namespace game

#endif // GAME_INTERACTIONS_INTERACTIONSYSTEM_H
