#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>

class PluginInterface{
public:
    virtual ~PluginInterface() = default;
    virtual void implement() = 0;

};

#define PluginInterface_iid "org.qt-project.PluginInterface"

Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
