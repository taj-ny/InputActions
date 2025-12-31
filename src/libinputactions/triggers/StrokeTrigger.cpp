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
    Copyright (c) 2008-2009, Thomas Jaeger <ThJaeger@gmail.com>
    Copyright (c) 2020-2023, Daniel Kondor <kondor.dani@gmail.com>
    Copyright (c) 2024, Jakob Petsovits <jpetso@petsovits.com>

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

#include "StrokeTrigger.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace InputActions
{

static const qreal SHARP_TURN_ANGLE_THRESHOLD = 30 * M_PI / 180;

StrokeTrigger::StrokeTrigger(std::vector<Stroke> strokes)
    : MotionTrigger(TriggerType::Stroke)
    , m_strokes(std::move(strokes))
{
}

const std::vector<Stroke> &StrokeTrigger::strokes() const
{
    return m_strokes;
}

constexpr double stroke_infinity = 0.2;
#define EPS 0.000001

Stroke::Stroke(const std::vector<QPointF> &deltas)
{
    for (auto &point : simplify(deltasToPath(deltas))) {
        m_points.push_back({
            .x = point.x(),
            .y = point.y(),
        });
    }
    finish();
}

Stroke::Stroke(const std::vector<Point> &points)
{
    m_points = points;
}

const std::vector<Point> &Stroke::points() const
{
    return m_points;
}

/* To compare two gestures, we use dynamic programming to minimize (an
 * approximation) of the integral over square of the angle difference among
 * (roughly) all reparametrizations whose slope is always between 1/2 and 2.
 */
qreal Stroke::compare(const Stroke &other) const
{
    const int M = m_points.size();
    const int N = other.m_points.size();
    const int m = M - 1;
    const int n = N - 1;

    std::vector<double> dist(M * N);
    std::vector<int> prev_x(M * N);
    std::vector<int> prev_y(M * N);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            dist[i * N + j] = stroke_infinity;
    dist[M * N - 1] = stroke_infinity;
    dist[0] = 0.0;

    for (int x = 0; x < m; x++) {
        for (int y = 0; y < n; y++) {
            if (dist[x * N + y] >= stroke_infinity)
                continue;
            double tx = m_points[x].t;
            double ty = other.m_points[y].t;
            int max_x = x;
            int max_y = y;
            int k = 0;

            while (k < 4) {
                if (m_points[max_x + 1].t - tx > other.m_points[max_y + 1].t - ty) {
                    max_y++;
                    if (max_y == n) {
                        step(other, N, dist, prev_x, prev_y, x, y, tx, ty, &k, m, n);
                        break;
                    }
                    for (int x2 = x + 1; x2 <= max_x; x2++)
                        step(other, N, dist, prev_x, prev_y, x, y, tx, ty, &k, x2, max_y);
                } else {
                    max_x++;
                    if (max_x == m) {
                        step(other, N, dist, prev_x, prev_y, x, y, tx, ty, &k, m, n);
                        break;
                    }
                    for (int y2 = y + 1; y2 <= max_y; y2++)
                        step(other, N, dist, prev_x, prev_y, x, y, tx, ty, &k, max_x, y2);
                }
            }
        }
    }

    const auto cost = dist[M * N - 1];
    if (cost >= stroke_infinity) {
        return 0;
    }
    return std::max(1.0 - 2.5 * cost, 0.0);
}

double Stroke::min_matching_score()
{
    return 0.7;
}

std::vector<QPointF> Stroke::deltasToPath(const std::vector<QPointF> &deltas)
{
    std::vector<QPointF> result{QPointF(0, 0)};
    QPointF total;
    for (const auto &delta : deltas) {
        total += delta;
        result.push_back(total);
    }
    return result;
}

std::vector<QPointF> Stroke::simplify(const std::vector<QPointF> &points, qreal epsilon)
{
    std::vector<QPointF> result;
    ramerDouglasPeucker(points, epsilon, result);
    return result;
}

qreal Stroke::perpendicularDistance(const QPointF &point, const QPointF &lineStart, const QPointF &lineEnd)
{
    QPointF vec1(point.x() - lineStart.x(), point.y() - lineStart.y());
    QPointF vec2(lineEnd.x() - lineStart.x(), lineEnd.y() - lineStart.y());
    auto d_vec2 = std::sqrt(vec2.x() * vec2.x() + vec2.y() * vec2.y());
    float cross_product = vec1.x() * vec2.y() - vec2.x() * vec1.y();
    float d = abs(cross_product / d_vec2);
    return d;
}

// TODO: This algorithm is not suitable for strokes and should be replaced with something else: https://github.com/taj-ny/InputActions/issues/335
void Stroke::ramerDouglasPeucker(const std::vector<QPointF> &points, qreal epsilon, std::vector<QPointF> &out)
{
    if (points.size() < 2) {
        out = points;
        return;
    }

    // Find the point with the maximum distance from line between start and end or where a sharp turn occurs
    auto dmax = 0.0;
    int32_t splitIndex = -1;
    size_t end = points.size() - 1;
    for (size_t i = 1; i < end; i++) {
        const auto lineStart = points[0];
        const auto previous = points[i - 1];
        const auto current = points[i];
        const auto next = points[i + 1];
        const auto lineEnd = points[end];

        // Preserve sharp turns (workaround)
        if (angle(previous, current, next) <= SHARP_TURN_ANGLE_THRESHOLD) {
            splitIndex = i;
            break;
        }

        double d = perpendicularDistance(current, lineStart, lineEnd);
        if (d > dmax) {
            if (dmax > epsilon) {
                splitIndex = i;
            }
            dmax = d;
        }
    }

    // Recursively simplify
    if (splitIndex != -1) {
        // Recursive call
        std::vector<QPointF> recResults1;
        std::vector<QPointF> recResults2;
        std::vector<QPointF> firstLine(points.begin(), points.begin() + splitIndex + 1);
        std::vector<QPointF> lastLine(points.begin() + splitIndex, points.end());
        ramerDouglasPeucker(firstLine, epsilon, recResults1);
        ramerDouglasPeucker(lastLine, epsilon, recResults2);

        // Build the result list
        out.assign(recResults1.begin(), recResults1.end() - 1);
        out.insert(out.end(), recResults2.begin(), recResults2.end());
        //        if(out.size()<2)
        //            throw runtime_error("Problem assembling output");
    } else {
        // Just return start and end points
        out.clear();
        out.push_back(points[0]);
        out.push_back(points[end]);
    }
}

qreal Stroke::angle(const QPointF &a, const QPointF &b, const QPointF &c)
{
    const auto ba = a - b;
    const auto bc = c - b;

    const auto dot = ba.x() * bc.x() + ba.y() * bc.y();
    const auto mag = std::hypot(ba.x(), ba.y()) * std::hypot(bc.x(), bc.y());

    if (mag == 0) {
        return 0;
    }
    return std::acos(std::clamp(dot / mag, -1.0, 1.0));
}

static inline double angle_difference(double alpha, double beta)
{
    double d = alpha - beta;
    if (d < -1.0)
        d += 2.0;
    else if (d > 1.0)
        d -= 2.0;
    return d;
}

void Stroke::finish()
{
    int n = m_points.size() - 1;
    double total = 0.0;
    m_points[0].t = 0.0;
    for (int i = 0; i < n; i++) {
        total += hypot(m_points[i + 1].x - m_points[i].x, m_points[i + 1].y - m_points[i].y);
        m_points[i + 1].t = total;
    }
    for (int i = 0; i <= n; i++)
        m_points[i].t /= total;
    double minX = m_points[0].x, minY = m_points[0].y, maxX = minX, maxY = minY;
    for (int i = 1; i <= n; i++) {
        if (m_points[i].x < minX)
            minX = m_points[i].x;
        if (m_points[i].x > maxX)
            maxX = m_points[i].x;
        if (m_points[i].y < minY)
            minY = m_points[i].y;
        if (m_points[i].y > maxY)
            maxY = m_points[i].y;
    }
    double scaleX = maxX - minX;
    double scaleY = maxY - minY;
    double scale = (scaleX > scaleY) ? scaleX : scaleY;
    if (scale < 0.001)
        scale = 1;
    for (int i = 0; i <= n; i++) {
        m_points[i].x = (m_points[i].x - (minX + maxX) / 2) / scale + 0.5;
        m_points[i].y = (m_points[i].y - (minY + maxY) / 2) / scale + 0.5;
    }

    for (int i = 0; i < n; i++) {
        m_points[i].alpha = atan2(m_points[i + 1].y - m_points[i].y, m_points[i + 1].x - m_points[i].x) / M_PI;
    }
}

inline static double sqr(double x)
{
    return x * x;
}

void Stroke::step(const Stroke &other, int N, std::vector<qreal> &dist, std::vector<int> &prevX, std::vector<int> &prevY, int x, int y, qreal tx, qreal ty,
                  int *k, int x2, int y2) const
{
    double dtx = m_points[x2].t - tx;
    double dty = other.m_points[y2].t - ty;
    if (dtx >= dty * 2.2 || dty >= dtx * 2.2 || dtx < EPS || dty < EPS)
        return;
    (*k)++;

    double d = 0.0;
    int i = x, j = y;
    double next_tx = (m_points[i + 1].t - tx) / dtx;
    double next_ty = (other.m_points[j + 1].t - ty) / dty;
    double cur_t = 0.0;

    for (;;) {
        double ad = sqr(angle_difference(m_points[i].alpha, other.m_points[j].alpha));
        double next_t = next_tx < next_ty ? next_tx : next_ty;
        bool done = next_t >= 1.0 - EPS;
        if (done)
            next_t = 1.0;
        d += (next_t - cur_t) * ad;
        if (done)
            break;
        cur_t = next_t;
        if (next_tx < next_ty)
            next_tx = (m_points[++i + 1].t - tx) / dtx;
        else
            next_ty = (other.m_points[++j + 1].t - ty) / dty;
    }
    double new_dist = dist[x * N + y] + d * (dtx + dty);
    if (new_dist >= dist[x2 * N + y2])
        return;

    prevX[x2 * N + y2] = x;
    prevY[x2 * N + y2] = y;
    dist[x2 * N + y2] = new_dist;
}

}