#ifndef STOPBOMBVIEW_HPP
#define STOPBOMBVIEW_HPP

#include <gui_generated/stopbomb_screen/StopBombViewBase.hpp>
#include <gui/stopbomb_screen/StopBombPresenter.hpp>

class StopBombView : public StopBombViewBase
{
public:
    StopBombView();
    virtual ~StopBombView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    virtual void decrementValue();
    virtual void incrementValue();
    virtual void knobPressed();
protected:
    uint32_t tickCounter = 0;
    uint8_t x_value = 22;
    uint8_t error_counter = 0;
private: 
    void updatePosition();
    
    float angle = 0;     // Current angle in radians
    int centerX =  120;     // Circle center X coordinate
    int centerY = 120;     // Circle center Y coordinate
    int radius = 95;      

    void updatePosition2();
    
    float angle2 = 0;     // Current angle in radians
    int centerX2 =  120;     // Circle center X coordinate
    int centerY2 = 120;     // Circle center Y coordinate
    int radius2 = 100;      

    void setFrameColor(uint8_t r, uint8_t g, uint8_t b);
};

#endif // STOPBOMBVIEW_HPP
