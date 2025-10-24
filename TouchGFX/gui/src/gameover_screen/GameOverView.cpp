#include <gui/gameover_screen/GameOverView.hpp>
#include <touchgfx/Color.hpp>

GameOverView::GameOverView()
{

}

void GameOverView::setupScreen()
{
    GameOverViewBase::setupScreen();
}

void GameOverView::tearDownScreen()
{
    GameOverViewBase::tearDownScreen();
}

void GameOverView::handleTickEvent()
{
    tickCounter++;
    if(tickCounter % 31 == 0)
    {
        if(Color::getRed(boxBackground.getColor()) == 255)
        {
            boxBackground.setColor(touchgfx::Color::getColorFromRGB(0, 0, 0));
        }
        else
        {
            boxBackground.setColor(touchgfx::Color::getColorFromRGB(255, 0, 0));
        }
        tickCounter = 0;
        boxBackground.invalidate();
    }
}
