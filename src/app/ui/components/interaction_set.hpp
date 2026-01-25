#ifndef INTERACTION_SET_HPP
#define INTERACTION_SET_HPP

#include <memory>
#include <vector>

#include "image_view_interaction.hpp"
#include "view_behavior.hpp"

namespace ofeli_app {

/**
 * @brief Aggregates multiple ViewBehavior objects.
 *
 * InteractionSet is the concrete ImageViewInteraction used by ImageView.
 * It forwards each event to all registered behaviors.
 */
class InteractionSet : public ImageViewInteraction
{
public:
    InteractionSet() = default;
    ~InteractionSet() override = default;

    // Non copyable (ownership of behaviors)
    InteractionSet(const InteractionSet&) = delete;
    InteractionSet& operator=(const InteractionSet&) = delete;

    // Movable if needed
    InteractionSet(InteractionSet&&) = default;
    InteractionSet& operator=(InteractionSet&&) = default;

    void addBehavior(std::unique_ptr<ViewBehavior> behavior);

protected:
    void wheel(ImageView& view, QWheelEvent* event) override;
    void mousePress(ImageView& view, QMouseEvent* event) override;
    void mouseMove(ImageView& view, QMouseEvent* event) override;
    void mouseRelease(ImageView& view, QMouseEvent* event) override;
    void mouseDoubleClick(ImageView& view, QMouseEvent* event) override;

private:
    std::vector<std::unique_ptr<ViewBehavior>> behaviors_;
};

} // namespace ofeli_app

#endif // INTERACTION_SET_HPP
