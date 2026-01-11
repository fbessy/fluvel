#ifndef PHI_EDITOR_HPP
#define PHI_EDITOR_HPP

#include <vector>

#include "matrix.hpp"
#include "contour_data.hpp"
#include "active_contour.hpp"

#include <QObject>

namespace ofeli_app
{

class PhiEditor : public QObject
{
    Q_OBJECT
public:
    PhiEditor(int width, int height);
    ~PhiEditor() = default;

    void setPhiFromLists(const std::vector<ofeli_ip::ContourPoint>& Lout,
                         const std::vector<ofeli_ip::ContourPoint>& Lin);


    void applyShapeFromLists(const std::vector<ofeli_ip::ContourPoint>& lout,
                             const std::vector<ofeli_ip::ContourPoint>& lin,
                             bool add);

    void clear();

    // ─────────────────────────────
    // Shape (forme temporaire)
    // ─────────────────────────────
    void setRectangle(float cx, float cy, float w, float h);
    void setEllipse  (float cx, float cy, float w, float h);

    // ─────────────────────────────
    // Edition
    // ─────────────────────────────
    void addShape(const ofeli_ip::Matrix<signed char>& shape);
    void subtractShape(const ofeli_ip::Matrix<signed char>& shape);

    // ─────────────────────────────
    // Accès lecture
    // ─────────────────────────────
    const ofeli_ip::Matrix<signed char>& phi() const { return phi_; }

    const std::vector<ofeli_ip::ContourPoint>& Lout() const { return Lout_; }
    const std::vector<ofeli_ip::ContourPoint>& Lin()  const { return Lin_;  }

    const std::vector<ofeli_ip::ContourPoint>& shapeLout() const { return Lout_shape_; }
    const std::vector<ofeli_ip::ContourPoint>& shapeLin()  const { return Lin_shape_;  }

    int width()  const { return width_; }
    int height() const { return height_; }

signals:
    void phiChanged();

private:
    // ─────────────────────────────
    // Internals
    // ─────────────────────────────
    void rebuildLists();

    void floodFillFromLists(const std::vector<ofeli_ip::ContourPoint>& Lout,
                            const std::vector<ofeli_ip::ContourPoint>& Lin,
                            ofeli_ip::Matrix<signed char>& phi) const;

    bool findRedundantPoint(const ofeli_ip::Matrix<signed char>& phi,
                            int offset) const;

    void do_flood_fill_from_lists(const std::vector<ofeli_ip::ContourPoint>& Lout, const std::vector<ofeli_ip::ContourPoint>& Lin,
                                  ofeli_ip::Matrix<signed char>& phi);

    void do_flood_fill(ofeli_ip::Matrix<signed char>& phi, int offset_seed,
                       ofeli_ip::PhiValue target_value, ofeli_ip::PhiValue replacement_value);

private:
    int width_  = 0;
    int height_ = 0;

    ofeli_ip::Matrix<signed char> phi_;
    ofeli_ip::Matrix<signed char> shape_;

    std::vector<ofeli_ip::ContourPoint> Lout_;
    std::vector<ofeli_ip::ContourPoint> Lin_;

    std::vector<ofeli_ip::ContourPoint> Lout_shape_;
    std::vector<ofeli_ip::ContourPoint> Lin_shape_;
};

} // namespace

#endif // PHI_EDITOR_HPP
