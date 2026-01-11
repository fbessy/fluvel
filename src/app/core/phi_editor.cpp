#include "phi_editor.hpp"

#include "boundary_builder.hpp"

#include <stack>

namespace ofeli_app
{

PhiEditor::PhiEditor(int width, int height)
    : width_(width)
    , height_(height)
    , phi_(width, height)
    , shape_(width, height)
{
    clear();
}

void PhiEditor::setPhiFromLists(const std::vector<ofeli_ip::ContourPoint>& Lout,
                                const std::vector<ofeli_ip::ContourPoint>& Lin)
{
    Lout_ = Lout;
    Lin_  = Lin;

    floodFillFromLists(Lout_, Lin_, phi_);
}

// ─────────────────────────────
// Reset
// ─────────────────────────────

void PhiEditor::clear()
{
    phi_.memset(ofeli_ip::PhiValue::OUTSIDE_REGION);
    Lout_.clear();
    Lin_.clear();
}

// ─────────────────────────────
// Shape definition
// ─────────────────────────────

void PhiEditor::setRectangle(float cx, float cy, float w, float h)
{
    Lout_shape_.clear();
    Lin_shape_.clear();

    ofeli_ip::BoundaryBuilder builder(width_, height_,
                                      Lout_shape_, Lin_shape_);

    builder.get_rectangle_points(cx + 0.5f - 0.5f * w,
                                 cy + 0.5f - 0.5f * h,
                                 cx + 0.5f + 0.5f * w,
                                 cy + 0.5f + 0.5f * h);
}

void PhiEditor::setEllipse(float cx, float cy, float w, float h)
{
    Lout_shape_.clear();
    Lin_shape_.clear();

    ofeli_ip::BoundaryBuilder builder(width_, height_,
                                      Lout_shape_, Lin_shape_);

    builder.get_ellipse_points(cx + 0.5f,
                               cy + 0.5f,
                               0.5f * w,
                               0.5f * h);
}

// ─────────────────────────────
// Edition
// ─────────────────────────────

void PhiEditor::addShape(const ofeli_ip::Matrix<signed char>& shape)
{
    bool changed = false;
    const int size = phi_.get_width() * phi_.get_height();

    for (int i = 0; i < size; ++i) {
        if (shape[i] == ofeli_ip::PhiValue::INSIDE_REGION &&
            phi_[i] == ofeli_ip::PhiValue::OUTSIDE_REGION) {
            phi_[i] = ofeli_ip::PhiValue::INSIDE_REGION;
            changed = true;
        }
    }

    if (changed) {
        rebuildLists();
        emit phiChanged();
    }
}

void PhiEditor::subtractShape(const ofeli_ip::Matrix<signed char>& shape)
{
    bool changed = false;

    const int size = phi_.get_width() * phi_.get_height();
    for (int i = 0; i < size; ++i)
    {
        if (shape[i] == ofeli_ip::PhiValue::INSIDE_REGION &&
            phi_[i]   == ofeli_ip::PhiValue::INSIDE_REGION)
        {
            phi_[i] = ofeli_ip::PhiValue::OUTSIDE_REGION;
            changed = true;
        }
    }

    if (changed) {
        rebuildLists();
        emit phiChanged();
    }
}

// ─────────────────────────────
// Reconstruction
// ─────────────────────────────

void PhiEditor::rebuildLists()
{
    Lout_.clear();
    Lin_.clear();

    const int size = width_ * height_;
    int x, y;

    for (int offset = 0; offset < size; ++offset)
    {
        phi_.get_position(offset, x, y);

        if (phi_[offset] < ofeli_ip::PhiValue::ZERO_LEVEL_SET)
        {
            if (!findRedundantPoint(phi_, offset))
                Lin_.emplace_back(offset, x);
        }
        else
        {
            if (!findRedundantPoint(phi_, offset))
                Lout_.emplace_back(offset, x);
        }
    }
}

// ─────────────────────────────
// Flood fill
// ─────────────────────────────

void PhiEditor::floodFillFromLists(const std::vector<ofeli_ip::ContourPoint>& Lout,
                                   const std::vector<ofeli_ip::ContourPoint>& Lin,
                                   ofeli_ip::Matrix<signed char>& phi) const
{
    phi.memset(ofeli_ip::PhiValue::OUTSIDE_REGION);

    for (const auto& p : Lout)
        phi[p.get_offset()] = ofeli_ip::PhiValue::EXTERIOR_BOUNDARY;

    for (const auto& p : Lin)
    {
        std::stack<int> stack;
        stack.push(p.get_offset());

        while (!stack.empty())
        {
            int offset = stack.top();
            stack.pop();

            if (phi[offset] != ofeli_ip::PhiValue::OUTSIDE_REGION)
                continue;

            phi[offset] = ofeli_ip::PhiValue::INSIDE_REGION;

            int x, y;
            phi.get_position(offset, x, y);

            if (x > 0)               stack.push(phi.get_offset(x - 1, y));
            if (x < width_  - 1)     stack.push(phi.get_offset(x + 1, y));
            if (y > 0)               stack.push(phi.get_offset(x, y - 1));
            if (y < height_ - 1)     stack.push(phi.get_offset(x, y + 1));
        }
    }

    for (const auto& p : Lout)
        phi[p.get_offset()] = ofeli_ip::PhiValue::OUTSIDE_REGION;
}

// ─────────────────────────────
// Redundancy test
// ─────────────────────────────

bool PhiEditor::findRedundantPoint(const ofeli_ip::Matrix<signed char>& phi,
                                   int offset) const
{
    int x, y;
    phi.get_position(offset, x, y);

    const signed char v = phi[offset];

    auto check = [&](int nx, int ny)
    {
        return (phi(nx, ny) * v) < 0;
    };

    if (x > 0 && check(x - 1, y)) return false;
    if (x < width_ - 1 && check(x + 1, y)) return false;
    if (y > 0 && check(x, y - 1)) return false;
    if (y < height_ - 1 && check(x, y + 1)) return false;

    return true;
}

void PhiEditor::applyShapeFromLists(const std::vector<ofeli_ip::ContourPoint>& lout,
                                    const std::vector<ofeli_ip::ContourPoint>& lin,
                                    bool add)
{
    // 1. créer une matrice temporaire
    ofeli_ip::Matrix<signed char> shape(phi_.get_width(), phi_.get_height());
    shape.memset(ofeli_ip::PhiValue::OUTSIDE_REGION);

    // 2. réutiliser EXACTEMENT ton code existant
    do_flood_fill_from_lists(lout, lin, shape);

    // 3. appliquer à phi_
    const int size = phi_.get_width() * phi_.get_height();
    for (int i = 0; i < size; ++i)
    {
        if (add)
        {
            if (shape[i] == ofeli_ip::PhiValue::INSIDE_REGION)
                phi_[i] = ofeli_ip::PhiValue::INSIDE_REGION;
        }
        else
        {
            if (shape[i] == ofeli_ip::PhiValue::INSIDE_REGION)
                phi_[i] = ofeli_ip::PhiValue::OUTSIDE_REGION;
        }
    }

    // 4. reconstruction OBLIGATOIRE
    rebuildLists();
}

void PhiEditor::do_flood_fill(ofeli_ip::Matrix<signed char>& phi, int offset_seed,
                              ofeli_ip::PhiValue target_value, ofeli_ip::PhiValue replacement_value)
{
    if( target_value != replacement_value &&
        offset_seed < phi.get_width()*phi.get_height()-1 )
    {
        std::stack<int> offset_seeds;
        // top seed coordinates (x_ts,y_ts) and x for scan the row
        int x_ts, y_ts, x;
        bool span_up, span_down;

        offset_seeds.push(offset_seed);

        while( !offset_seeds.empty() )
        {
            // unstack the top seed
            phi.get_position(offset_seeds.top(),
                             x_ts,y_ts); // x_ts and y_ts passed by reference
            offset_seeds.pop();

            // x initialization at the left-most point of the seed
            x = x_ts;
            while( x > 0 &&
                   phi(x-1,y_ts) == target_value )
            {
                x--;
            }

            span_up = false;
            span_down = false;

            // pixels are treated row-wise
            while( x < phi.get_width() &&
                   phi(x,y_ts) == target_value )
            {
                phi(x,y_ts) = replacement_value;

                if( !span_up &&
                    y_ts > 0 &&
                    phi(x,y_ts-1) == target_value )
                {
                    offset_seeds.push( phi.get_offset(x,y_ts-1) );
                    span_up = true;
                }
                else if( span_up &&
                         y_ts > 0 &&
                         phi(x,y_ts-1) != target_value )
                {
                    span_up = false;
                }

                if( !span_down &&
                    y_ts < phi.get_height()-1 &&
                    phi(x,y_ts+1) == target_value )
                {
                    offset_seeds.push( phi.get_offset(x,y_ts+1) );
                    span_down = true;
                }
                else if( span_down &&
                         y_ts < phi.get_height()-1
                         && phi(x,y_ts+1) != target_value )
                {
                    span_down = false;
                }

                x++;
            }
        }
    }
}

void PhiEditor::do_flood_fill_from_lists(const std::vector<ofeli_ip::ContourPoint>& Lout,
                                         const std::vector<ofeli_ip::ContourPoint>& Lin,
                                         ofeli_ip::Matrix<signed char>& phi)
{
    phi.memset(ofeli_ip::PhiValue::OUTSIDE_REGION);

    for( const auto& point : Lout )
    {
        phi[ point.get_offset() ] = ofeli_ip::PhiValue::EXTERIOR_BOUNDARY;
    }

    for( const auto& point : Lin )
    {
        do_flood_fill( phi,
                      point.get_offset(),
                      ofeli_ip::PhiValue::OUTSIDE_REGION,
                      ofeli_ip::PhiValue::INSIDE_REGION );
    }

    for( const auto& point : Lout )
    {
        phi[ point.get_offset() ] = ofeli_ip::PhiValue::OUTSIDE_REGION;
    }
}

} // namespace
