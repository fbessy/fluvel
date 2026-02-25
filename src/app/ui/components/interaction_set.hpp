#ifndef INTERACTION_SET_HPP
#define INTERACTION_SET_HPP

#include <memory>
#include <vector>

#include <QtCore/Qt>

#include "image_view_interaction.hpp"
#include "view_behavior.hpp"

namespace ofeli_app
{

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

    const ViewBehavior* capturingBehavior() const;

    Qt::CursorShape cursor(const ImageView& view, bool hasImage, bool isPanRelevant,
                           const QMouseEvent* e) const;

protected:
    bool wheel(ImageView& view, QWheelEvent* event) override;
    bool mousePress(ImageView& view, QMouseEvent* event) override;
    bool mouseMove(ImageView& view, QMouseEvent* event) override;
    bool mouseRelease(ImageView& view, QMouseEvent* event) override;
    bool mouseDoubleClick(ImageView& view, QMouseEvent* event) override;

    Qt::CursorShape cursorForEvent(const ImageView& view, bool hasImage, bool isPanRelevant,
                                   const QMouseEvent* event) const override;

private:
    std::vector<std::unique_ptr<ViewBehavior>> behaviors_;
};

} // namespace ofeli_app

#endif // INTERACTION_SET_HPP
