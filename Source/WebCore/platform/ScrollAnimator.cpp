/*
 * Copyright (C) 2014-2015 Apple Inc.  All rights reserved.
 * Copyright (c) 2010, Google Inc. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ScrollAnimator.h"

#include "FloatPoint.h"
#include "PlatformWheelEvent.h"
#include "ScrollableArea.h"
#include <algorithm>
#include <wtf/PassOwnPtr.h>

namespace WebCore {

#if !ENABLE(SMOOTH_SCROLLING) && !PLATFORM(IOS)
PassOwnPtr<ScrollAnimator> ScrollAnimator::create(ScrollableArea& scrollableArea)
{
    return adoptPtr(new ScrollAnimator(scrollableArea));
}
#endif

ScrollAnimator::ScrollAnimator(ScrollableArea& scrollableArea)
    : m_scrollableArea(scrollableArea)
    , m_currentPosX(0)
    , m_currentPosY(0)
{
}

ScrollAnimator::~ScrollAnimator()
{
}

bool ScrollAnimator::scroll(ScrollbarOrientation orientation, ScrollGranularity, float step, float multiplier)
{
    float* currentPos = (orientation == HorizontalScrollbar) ? &m_currentPosX : &m_currentPosY;
    float newPos = std::max(std::min(*currentPos + (step * multiplier), static_cast<float>(m_scrollableArea.scrollSize(orientation))), 0.0f);
    float delta = *currentPos - newPos;
    if (*currentPos == newPos)
        return false;
    *currentPos = newPos;

    notifyPositionChanged(orientation == HorizontalScrollbar ? FloatSize(delta, 0) : FloatSize(0, delta));

    return true;
}

void ScrollAnimator::scrollToOffsetWithoutAnimation(const FloatPoint& offset)
{
    FloatSize delta = FloatSize(offset.x() - m_currentPosX, offset.y() - m_currentPosY);
    m_currentPosX = offset.x();
    m_currentPosY = offset.y();
    notifyPositionChanged(delta);
}

#if ENABLE(CSS_SCROLL_SNAP) && PLATFORM(MAC)
void ScrollAnimator::processWheelEventForScrollSnap(const PlatformWheelEvent& wheelEvent)
{
    if (m_verticalScrollSnapAnimator)
        m_verticalScrollSnapAnimator->handleWheelEvent(wheelEvent);
    if (m_horizontalScrollSnapAnimator)
        m_horizontalScrollSnapAnimator->handleWheelEvent(wheelEvent);
}
#endif

bool ScrollAnimator::handleWheelEvent(const PlatformWheelEvent& e)
{
#if ENABLE(CSS_SCROLL_SNAP) && PLATFORM(MAC)
    if (m_verticalScrollSnapAnimator) {
        m_verticalScrollSnapAnimator->handleWheelEvent(e);
        if (m_verticalScrollSnapAnimator->shouldOverrideWheelEvent(e))
            return false;
    }
    if (m_horizontalScrollSnapAnimator) {
        m_horizontalScrollSnapAnimator->handleWheelEvent(e);
        if (m_horizontalScrollSnapAnimator->shouldOverrideWheelEvent(e))
            return false;
    }
#endif
#if PLATFORM(COCOA)
    // Events in the PlatformWheelEventPhaseMayBegin phase have no deltas, and therefore never passes through the scroll handling logic below.
    // This causes us to return with an 'unhandled' return state, even though this event was successfully processed.
    //
    // We receive at least one PlatformWheelEventPhaseMayBegin when starting main-thread scrolling (see FrameView::wheelEvent), which can
    // fool the scrolling thread into attempting to handle the scroll, unless we treat the event as handled here.
    if (e.phase() == PlatformWheelEventPhaseMayBegin)
        return true;
#endif

    Scrollbar* horizontalScrollbar = m_scrollableArea.horizontalScrollbar();
    Scrollbar* verticalScrollbar = m_scrollableArea.verticalScrollbar();

    // Accept the event if we have a scrollbar in that direction and can still
    // scroll any further.
    float deltaX = horizontalScrollbar ? e.deltaX() : 0;
    float deltaY = verticalScrollbar ? e.deltaY() : 0;

    bool handled = false;

    ScrollGranularity granularity = ScrollByPixel;
    IntSize maxForwardScrollDelta = m_scrollableArea.maximumScrollPosition() - m_scrollableArea.scrollPosition();
    IntSize maxBackwardScrollDelta = m_scrollableArea.scrollPosition() - m_scrollableArea.minimumScrollPosition();
    if ((deltaX < 0 && maxForwardScrollDelta.width() > 0)
        || (deltaX > 0 && maxBackwardScrollDelta.width() > 0)
        || (deltaY < 0 && maxForwardScrollDelta.height() > 0)
        || (deltaY > 0 && maxBackwardScrollDelta.height() > 0)) {
        handled = true;

        if (deltaY) {
            if (e.granularity() == ScrollByPageWheelEvent) {
                bool negative = deltaY < 0;
                deltaY = Scrollbar::pageStepDelta(m_scrollableArea.visibleHeight());
                if (negative)
                    deltaY = -deltaY;
            }
            scroll(VerticalScrollbar, granularity, verticalScrollbar->pixelStep(), -deltaY);
        }

        if (deltaX) {
            if (e.granularity() == ScrollByPageWheelEvent) {
                bool negative = deltaX < 0;
                deltaX = Scrollbar::pageStepDelta(m_scrollableArea.visibleWidth());
                if (negative)
                    deltaX = -deltaX;
            }
            scroll(HorizontalScrollbar, granularity, horizontalScrollbar->pixelStep(), -deltaX);
        }
    }
    return handled;
}

#if ENABLE(TOUCH_EVENTS)
bool ScrollAnimator::handleTouchEvent(const PlatformTouchEvent&)
{
    return false;
}
#endif

void ScrollAnimator::setCurrentPosition(const FloatPoint& position)
{
    m_currentPosX = position.x();
    m_currentPosY = position.y();
}

FloatPoint ScrollAnimator::currentPosition() const
{
    return FloatPoint(m_currentPosX, m_currentPosY);
}

void ScrollAnimator::notifyPositionChanged(const FloatSize& delta)
{
    UNUSED_PARAM(delta);
    m_scrollableArea.setScrollOffsetFromAnimation(IntPoint(m_currentPosX, m_currentPosY));
}

#if ENABLE(CSS_SCROLL_SNAP) && PLATFORM(MAC)
void ScrollAnimator::updateScrollAnimatorsAndTimers()
{
    // FIXME: Currently, scroll snap animators are recreated even though the snap offsets alone can be updated.
    if (m_scrollableArea.horizontalSnapOffsets()) {
        m_horizontalScrollSnapAnimator = std::make_unique<AxisScrollSnapAnimator>(this, m_scrollableArea.horizontalSnapOffsets(), ScrollEventAxis::Horizontal);
        m_horizontalScrollSnapTimer = std::make_unique<Timer>(*this, &ScrollAnimator::horizontalScrollSnapTimerFired);
    } else if (m_horizontalScrollSnapAnimator) {
        m_horizontalScrollSnapAnimator = nullptr;
        m_horizontalScrollSnapTimer = nullptr;
    }
    if (m_scrollableArea.verticalSnapOffsets()) {
        m_verticalScrollSnapAnimator = std::make_unique<AxisScrollSnapAnimator>(this, m_scrollableArea.verticalSnapOffsets(), ScrollEventAxis::Vertical);
        m_verticalScrollSnapTimer = std::make_unique<Timer>(*this, &ScrollAnimator::verticalScrollSnapTimerFired);
    } else if (m_verticalScrollSnapAnimator) {
        m_verticalScrollSnapAnimator = nullptr;
        m_verticalScrollSnapTimer = nullptr;
    }
}

LayoutUnit ScrollAnimator::scrollOffsetInAxis(ScrollEventAxis axis)
{
    FloatPoint currentPosition = this->currentPosition();
    return axis == ScrollEventAxis::Horizontal ? currentPosition.x() : currentPosition.y();
}

void ScrollAnimator::immediateScrollInAxis(ScrollEventAxis axis, float delta)
{
    FloatPoint currentPosition = this->currentPosition();
    if (axis == ScrollEventAxis::Horizontal)
        scrollToOffsetWithoutAnimation(FloatPoint(currentPosition.x() + delta, currentPosition.y()));
    else
        scrollToOffsetWithoutAnimation(FloatPoint(currentPosition.x(), currentPosition.y() + delta));
}

void ScrollAnimator::startScrollSnapTimer(ScrollEventAxis axis)
{
    Timer* scrollSnapTimer = axis == ScrollEventAxis::Horizontal ? m_horizontalScrollSnapTimer.get() : m_verticalScrollSnapTimer.get();
    if (!scrollSnapTimer->isActive())
        scrollSnapTimer->startRepeating(1.0 / 60.0);
}

void ScrollAnimator::stopScrollSnapTimer(ScrollEventAxis axis)
{
    Timer* scrollSnapTimer = axis == ScrollEventAxis::Horizontal ? m_horizontalScrollSnapTimer.get() : m_verticalScrollSnapTimer.get();
    if (scrollSnapTimer->isActive())
        scrollSnapTimer->stop();
}

void ScrollAnimator::horizontalScrollSnapTimerFired()
{
    m_horizontalScrollSnapAnimator->scrollSnapAnimationUpdate();
}

void ScrollAnimator::verticalScrollSnapTimerFired()
{
    m_verticalScrollSnapAnimator->scrollSnapAnimationUpdate();
}
#endif

} // namespace WebCore
