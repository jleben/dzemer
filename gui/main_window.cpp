#include "main_window.hpp"
#include "lv2_plugin_browser.hpp"

namespace dzemer {

MainWindow::MainWindow(Engine * engine, LV2::World & lv2_world):
    m_engine(engine)
{
    m_lv2_plugin_browser = new LV2_Plugin_Browser(lv2_world);

    setCentralWidget(m_lv2_plugin_browser);
}


}
