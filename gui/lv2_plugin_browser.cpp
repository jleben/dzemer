#include "lv2_plugin_browser.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "../lv2-util/World.hpp"

namespace dzemer {

LV2_Plugin_Browser::LV2_Plugin_Browser(LV2::World & lv2_world, QWidget * parent):
    QWidget(parent),
    m_lv2_world(lv2_world)
{
    m_plugin_list = new QTreeWidget;

    m_plugin_list->setHeaderLabels(QStringList() << "Name" << "URI");

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_plugin_list);

    update_plugin_info();
}

void LV2_Plugin_Browser::update_plugin_info()
{
    m_plugin_list->clear();

    m_lv2_world.loadAll();

    auto plugins = m_lv2_world.allPlugins();

    for (auto & plugin : plugins)
    {
        auto item = new QTreeWidgetItem;
        item->setText(0, QString::fromStdString(plugin.name()));

        m_plugin_list->addTopLevelItem(item);
    }
}

}
