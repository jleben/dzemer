#pragma once

#include <QTreeWidget>
#include "../lv2-util/World.hpp"

namespace dzemer {

class LV2_Plugin_Browser : public QWidget
{
public:
    LV2_Plugin_Browser(LV2::World &, QWidget * parent = nullptr);

    QSize sizeHint() const override { return QSize(400,400); }

    LV2::Plugin selectedPlugin() const;

private:
    void update_plugin_info();

    QTreeWidget * m_plugin_list;

    LV2::World & m_lv2_world;
};

}
