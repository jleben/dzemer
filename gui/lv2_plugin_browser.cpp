#include "lv2_plugin_browser.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QHeaderView>

#include "../lv2-util/World.hpp"

namespace dzemer {

LV2_Plugin_Browser::LV2_Plugin_Browser(LV2::World & lv2_world, QWidget * parent):
    QWidget(parent),
    m_lv2_world(lv2_world)
{
    m_plugin_list = new QTreeWidget;

    m_plugin_list->setHeaderLabels(QStringList() << "Name");

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_plugin_list);

    update_plugin_info();
}

void LV2_Plugin_Browser::update_plugin_info()
{
    qDebug() << "Updating plugins info...";

    m_plugin_list->clear();

    m_lv2_world.loadAll();

    auto plugins = m_lv2_world.allPlugins();

    for (auto & plugin : plugins)
    {
        auto klass_uri = plugin.klass().uri();

        if (klass_uri != LV2_CORE__InstrumentPlugin)
            continue;

        auto item = new QTreeWidgetItem;
        item->setText(0, QString::fromStdString(plugin.name()));

        item->setData(0, Qt::UserRole, QString::fromStdString(plugin.uri()));

        m_plugin_list->addTopLevelItem(item);
    }

    qDebug() << "Updating plugins info done.";

    m_plugin_list->resizeColumnToContents(0);
}

LV2::Plugin LV2_Plugin_Browser::selectedPlugin() const
{
    auto selected_items = m_plugin_list->selectedItems();

    if (selected_items.empty())
        return LV2::Plugin();

    auto item = selected_items.front();

    auto uri = item->data(0, Qt::UserRole).toString();

    auto plugin = m_lv2_world.pluginForUri(uri.toStdString());

    if (!plugin)
        qCritical() << "Could not find plugin for selected URI.";

    return plugin;
}

}
