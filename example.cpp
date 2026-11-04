#include "TGE.hpp"

int main() {
    tge::Canvas canvas;
    tge::Canvas::initializeTerminal();

    while (true) {
        auto [width, height] = tge::Canvas::getScreenSize();
        canvas.resize(width, height);
        canvas.setBG(tge::Color::black);

        canvas.drawCircle(12, 12, 8, tge::Color::yellow);
        canvas.drawFilledCircle(34, 12, 8, tge::Color::green);
        canvas.drawFilledRect(50, 4, 16, 16, tge::Color::blue);

        canvas.print();
        tge::Canvas::sleep(16);
    }

}
