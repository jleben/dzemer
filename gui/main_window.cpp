#include "main_window.hpp"
#include "lv2_plugin_browser.hpp"

#include <QDebug>

namespace dzemer {

MainWindow::MainWindow(Engine * engine, LV2::World & lv2_world):
    m_engine(engine)
{
    m_lv2_plugin_browser = new LV2_Plugin_Browser(lv2_world);

    m_toolbar = new QToolBar;

    {
        auto action = m_toolbar->addAction("Add Instrument");
        connect(action, &QAction::triggered,
                this, &MainWindow::addSelectedInstrument);
    }

    setCentralWidget(m_lv2_plugin_browser);

    addToolBar(Qt::TopToolBarArea, m_toolbar);
}

void MainWindow::addSelectedInstrument()
{
    auto plugin = m_lv2_plugin_browser->selectedPlugin();
    if (!plugin)
        return;

    qDebug() << "Adding instrument: " << QString::fromStdString(plugin.name());

    m_engine->addSynth(plugin);
}

}
