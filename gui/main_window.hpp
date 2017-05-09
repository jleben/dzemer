#pragma once

#include "../engine/engine.hpp"

#include <QMainWindow>
#include <QToolBar>

namespace dzemer {

class LV2_Plugin_Browser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Engine *, LV2::World &);

private:
    void addSelectedInstrument();

    Engine * m_engine;
    LV2_Plugin_Browser * m_lv2_plugin_browser;
    QToolBar * m_toolbar;
};

}
