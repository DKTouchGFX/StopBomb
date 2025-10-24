#include <gui/stopbomb_screen/StopBombView.hpp>
#include <touchgfx/Color.hpp>
#include <cstdlib>
#include <cmath>
#ifndef SIMULATOR
#include "knob_interface.hpp"
#endif
// #include <touchgfx/Utils.hpp>

StopBombView::StopBombView()
{

}

void StopBombView::setupScreen()
{
    StopBombViewBase::setupScreen();
    error_counter = 0;
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
        resetBomb.setVisible(true);
        bomb.setVisible(true);
        txtInfo.setVisible(false);
        bomb.moveTo(120 - (bomb.getWidth() / 2), 120 - (bomb.getHeight() /2));
        setFrameColor(255, 255, 255); // white
    }
    if(tickCounter % 200 == 0)
    {
        bomb.clearMoveAnimationEndedAction();
        updatePositionBomb();
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
    if(error_counter >= 2)
    {
        application().gotoGameOverScreenNoTransition();
    }
    
    if( bomb.getX() >= (resetBomb.getX() - bomb.getWidth()/2) &&
        bomb.getX() <= (resetBomb.getX() + resetBomb.getWidth()/2) &&
        bomb.getY() >= (resetBomb.getY() - bomb.getHeight()/2) &&
        bomb.getY() <= (resetBomb.getY() + resetBomb.getHeight()/2) )
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

    resetBomb.startMoveAnimation(x - resetBomb.getWidth() / 2, y - resetBomb.getHeight() / 2, 0, touchgfx::EasingEquations::linearEaseOut, touchgfx::EasingEquations::linearEaseIn);

    resetBomb.invalidate();
}

void StopBombView::updatePositionBomb()
{
    angleBomb = rand() % 628; // 0 to 627   
    angleBomb /= 100.0f;      // Convert to radians (0 to 6.27)

    int x = centerXBomb + static_cast<int>(radiusBomb * std::cos(angleBomb));
    int y = centerYBomb + static_cast<int>(radiusBomb * std::sin(angleBomb));

    // touchgfx_printf("\nNew shape position [x: %d y: %d]", x, y);

    bomb.startMoveAnimation(x - bomb.getWidth() / 2, y - bomb.getHeight() / 2, 90, touchgfx::EasingEquations::linearEaseOut, touchgfx::EasingEquations::linearEaseIn);

    bomb.invalidate();
}

void StopBombView::setFrameColor(uint8_t r, uint8_t g, uint8_t b)
{
    framePainter.setColor(touchgfx::Color::getColorFromRGB(r, g, b));
    frame.invalidate();
#ifndef SIMULATOR
    knobSetAmbientLightRGB(r, g, b);
#endif    
}
