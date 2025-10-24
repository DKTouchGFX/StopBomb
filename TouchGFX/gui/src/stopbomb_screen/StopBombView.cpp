#include <gui/stopbomb_screen/StopBombView.hpp>
#include <touchgfx/Color.hpp>
#include <cstdlib>
#include <cmath>
#ifndef SIMULATOR
#include "knob_interface.hpp"
#endif
#include <touchgfx/Utils.hpp>

StopBombView::StopBombView()
{

}

void StopBombView::setupScreen()
{
    StopBombViewBase::setupScreen();
}

void StopBombView::tearDownScreen()
{
    StopBombViewBase::tearDownScreen();
}

void StopBombView::handleTickEvent()
{
    tickCounter++;
    if(tickCounter % 180 == 0)
    {
        circle1.setVisible(true);
        shape1.setVisible(true);
        textArea1.setVisible(false);
        shape1.moveTo(120 - shape1.getWidth(), 120 - shape1.getHeight());
        setFrameColor(255, 255, 255); // white
        error_counter = 0;
        
    }
    if(tickCounter % 200 == 0)
    {
        shape1.clearMoveAnimationEndedAction();
        updatePosition2();
        tickCounter = 0;
    }
}

void StopBombView::decrementValue()
{
    angle += 0.25f; // Increment angle
    updatePosition();
}

void StopBombView::incrementValue()
{
    angle -= 0.25f; // Decrement angle
    updatePosition();
}

void StopBombView::knobPressed()
{
    if(error_counter > 2)
    {
        application().gotoGameOverScreenBlockTransition();
    }
    
    if( shape1.getX() >= (circle1.getX() - shape1.getWidth()/2) &&
        shape1.getX() <= (circle1.getX() + circle1.getWidth()/2) &&
        shape1.getY() >= (circle1.getY() - shape1.getHeight()/2) &&
        shape1.getY() <= (circle1.getY() + circle1.getHeight()/2) )
    {
        // Hit
        setFrameColor(0, 255, 0); // Green
    }
    else
    {
        // Miss
        setFrameColor(255, 0, 0); // Red
        error_counter++;
    }
    frame.invalidate();
}

void StopBombView::updatePosition()
{
    // Keep angle in [0, 2*pi] to avoid floating overflow
    if (angle > 2 * 3.14f)
    {
        angle -= 2 * 3.14f;
    }
    else if (angle < 0)
    {
        angle += 2 * 3.14f;
    }

    int x = centerX + static_cast<int>(radius * std::cos(angle));
    int y = centerY + static_cast<int>(radius * std::sin(angle));

    touchgfx_printf("\nNew circle position [x: %d y: %d]", x, y);

    circle1.setXY(x - circle1.getWidth() / 2, y - circle1.getHeight() / 2);

    invalidate();
}

void StopBombView::updatePosition2()
{
    // Keep angle in [0, 2*pi] to avoid floating overflow
    angle2 = rand() % 628; // 0 to 627
    
    
    angle2 /= 100.0f; // Convert to radians (0 to 6.27)

    int x = centerX2 + static_cast<int>(radius2 * std::cos(angle2));
    int y = centerY2 + static_cast<int>(radius2 * std::sin(angle2));

    touchgfx_printf("\nNew shape position [x: %d y: %d]", x, y);

    shape1.startMoveAnimation(x - shape1.getWidth() / 2, y - shape1.getHeight() / 2, 90, touchgfx::EasingEquations::linearEaseOut, touchgfx::EasingEquations::linearEaseIn);

    invalidate();
}

void StopBombView::setFrameColor(uint8_t r, uint8_t g, uint8_t b)
{
    framePainter.setColor(touchgfx::Color::getColorFromRGB(r, g, b));
    frame.invalidate();
#ifndef SIMULATOR
    knobSetAmbientLightRGB(r, g, b);
#endif    
}
