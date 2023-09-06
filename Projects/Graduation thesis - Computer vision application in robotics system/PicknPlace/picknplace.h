#ifndef PICKNPLACE_H
#define PICKNPLACE_H

#include <PluginInterface.h>
#include <main_dialog.h>

class PicknPlace : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid FILE "PicknPlace.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit PicknPlace(QObject *parent = nullptr);
    void implement();

private:
    main_dialog *main;

};

#endif // PICKNPLACE_H
