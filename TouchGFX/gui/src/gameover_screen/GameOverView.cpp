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
        if(Color::getRed(box1.getColor()) == 255)
        {
            box1.setColor(touchgfx::Color::getColorFromRGB(0, 0, 0));
        }
        else
        {
            box1.setColor(touchgfx::Color::getColorFromRGB(255, 0, 0));
        }
        tickCounter = 0;
        box1.invalidate();
    }
}

void GameOverView::knobPressed()
{
    application().gotoStopBombScreenNoTransition();
}