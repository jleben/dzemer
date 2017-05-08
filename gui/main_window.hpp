#pragma once

#include "../engine/engine.hpp"

#include <QMainWindow>

namespace dzemer {

class LV2_Plugin_Browser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Engine *, LV2::World &);

private:
    Engine * m_engine;
    LV2_Plugin_Browser * m_lv2_plugin_browser;
};

}
