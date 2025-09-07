/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2025 Marcin Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
/*
    Copyright (c) 2009, Thomas Jaeger <ThJaeger@gmail.com>
    Copyright (c) 2023, Daniel Kondor <kondor.dani@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
    OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
    CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#pragma once

#include "MotionTrigger.h"

namespace libinputactions
{

struct Point
{
    qreal x;
    qreal y;
    qreal t{};
    qreal alpha{};
};

class Stroke
{
public:
    Stroke() = default;
    Stroke(const std::vector<QPointF> &pointsRaw);
    Stroke(const std::vector<Point> &points);

    const std::vector<Point> &points() const;

    qreal compare(const Stroke &other) const;
    static double min_matching_score();

private:
    void finish();

    void step(const Stroke &other, int N, std::vector<qreal> &dist, std::vector<int> &prevX, std::vector<int> &prevY, int x, int y, qreal tx, qreal ty, int *k,
              int x2, int y2) const;

    /**
     * Converts the specified list of deltas to a path that starts at (0,0).
     */
    static std::vector<QPointF> deltasToPath(const std::vector<QPointF> &deltas);

    /**
     * Simplifies the specified path using the Ramer-Douglas–Peucker algorithm.
     */
    static std::vector<QPointF> simplify(const std::vector<QPointF> &points, qreal epsilon = 10);

    static qreal perpendicularDistance(const QPointF &point, const QPointF &lineStart, const QPointF &lineEnd);
    static void ramerDouglasPeucker(const std::vector<QPointF> &points, qreal epsilon, std::vector<QPointF> &out);

    std::vector<Point> m_points;
};

/**
 * A motion input action where a shape is drawn.
 */
class StrokeTrigger : public MotionTrigger
{
public:
    StrokeTrigger(std::vector<Stroke> strokes);

    const std::vector<Stroke> &strokes() const;

private:
    std::vector<Stroke> m_strokes;
};

}