
#include <QTimer>

#include "widget.hpp"

sync::Plugin::Plugin(Event::Manager* ev_manager)
    : Widgets::Plugin(ev_manager, std::string(sync::MODULE_NAME))
{
  this->sync_device = std::make_unique<sync::Device>();
  Event::Object insert_device_event(Event::Type::RT_DEVICE_INSERT_EVENT);
  insert_device_event.setParam(
      "device", std::any(static_cast<RT::Device*>(this->sync_device.get())));
  getEventManager()->postEvent(&insert_device_event);
}

sync::Plugin::~Plugin()
{
  Event::Object unplug_block_event(Event::Type::RT_DEVICE_REMOVE_EVENT);
  unplug_block_event.setParam(
      "device", std::any(static_cast<RT::Device*>(this->sync_device.get())));
  this->getEventManager()->postEvent(&unplug_block_event);
}

sync::Panel::Panel(QMainWindow* main_window, Event::Manager* ev_manager)
    : Widgets::Panel(std::string(sync::MODULE_NAME), main_window, ev_manager)
    , pluginList(new QComboBox())
    , directionList(new QComboBox())
    , portList(new QComboBox())
{
  setWhatsThis(
      "<p><b>Sync:</b><br>This module allows you to synchronize other modules "
      "that are derived from the DefaultGUIModel class. It does not work with "
      "other custom user modules. Type in a comma-separated list (with or "
      "without spaces) of numbers that are the instance IDs of the modules you "
      "want to synchronize. Instance IDs are located in the left-hand corner "
      "of the module's toolbar.</p>");
  auto* customlayout = new QVBoxLayout();
  this->setLayout(customlayout);

  // Initialize plugin list
  updatePluginList();

  // Initialize direction list
  directionList->addItem("OUTPUT", QVariant::fromValue(IO::OUTPUT));
  directionList->addItem("INPUT", QVariant::fromValue(IO::INPUT));

  // Initialize ports list for the currently selected block
  updatePortList();

  // Initialize plugin group
  auto* pluginGroupBox = new QGroupBox("Plugins");
  auto* pluginGroupLayout = new QVBoxLayout();
  auto* pluginInfoLayout = new QHBoxLayout();

  pluginGroupBox->setLayout(pluginGroupLayout);
  pluginInfoLayout->addWidget(pluginList);
  pluginInfoLayout->addWidget(directionList);
  pluginInfoLayout->addWidget(portList);
  pluginGroupLayout->addLayout(pluginInfoLayout);
  auto* pluginButtonsLayout = new QHBoxLayout();
  addPluginButton = new QPushButton("Sync");
  removePluginButton = new QPushButton("Remove");
  pluginButtonsLayout->addWidget(addPluginButton);
  pluginButtonsLayout->addWidget(removePluginButton);
  pluginGroupLayout->addLayout(pluginButtonsLayout);
  synchronizedPluginsList = new QListWidget();
  pluginGroupLayout->addWidget(synchronizedPluginsList);
  customlayout->addWidget(pluginGroupBox);
  updateSyncPluginList();

  dataCheckBox = new QCheckBox("&Sync Data Recorder");
  dataCheckBox->setEnabled(true);
  dataCheckBox->setChecked(false);
  QObject::connect(
      dataCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleRecord(bool)));
  timerCheckBox = new QCheckBox("&Sync Timer (s)");
  auto* timerBox = new QGroupBox("Timer Settings");
  auto* timerLayout = new QGridLayout();
  timerCheckBox->setWhatsThis(
      "Timer for turning sync off after a user-specified time period");
  timerCheckBox->setEnabled(true);
  timerCheckBox->setChecked(false);
  QObject::connect(
      timerCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleTimer(bool)));
  timerWheel = new QSpinBox(this);
  timerWheel->setEnabled(false);
  timerWheel->setValue(10);
  timerWheel->setFixedWidth(75);
  timerWheel->setMaximum(9999);
  timerWheel->setWhatsThis("Timer value in seconds");
  timerLayout->addWidget(dataCheckBox, 0, 0);
  timerLayout->addWidget(timerCheckBox, 1, 0);
  timerLayout->addWidget(timerWheel, 1, 1);
  timerBox->setLayout(timerLayout);
  customlayout->addWidget(timerBox);

  auto* infoGroup = new QGroupBox("Info");
  auto* infoLayout = new QHBoxLayout();
  syncState = new QLabel("Ready");
  timeElapsed = new QLabel("0");
  infoLayout->addWidget(new QLabel("Status :"));
  infoLayout->addWidget(syncState);
  infoLayout->addWidget(new QLabel("Time Elapsed :"));
  infoLayout->addWidget(timeElapsed);
  infoGroup->setLayout(infoLayout);
  customlayout->addWidget(infoGroup);

  // Create child widget
  auto* buttonGroup = new QGroupBox;
  auto* buttonLayout = new QHBoxLayout;

  // Create elements
  pauseCustomButton = new QPushButton("Pause", this);
  pauseCustomButton->setCheckable(true);
  QObject::connect(
      pauseCustomButton, SIGNAL(toggled(bool)), this, SLOT(syncToggle(bool)));
  buttonLayout->addWidget(pauseCustomButton);

  modifyCustomButton = new QPushButton("Modify", this);
  QObject::connect(
      modifyCustomButton, SIGNAL(clicked()), this, SLOT(updateSync()));
  buttonLayout->addWidget(modifyCustomButton);

  unloadCustomButton = new QPushButton("Unload", this);
  QObject::connect(
      unloadCustomButton, SIGNAL(clicked()), parentWidget(), SLOT(close()));
  buttonLayout->addWidget(unloadCustomButton);

  buttonGroup->setLayout(buttonLayout);
  setLayout(customlayout);

  QTimer::singleShot(0, this, SLOT(resizeMe()));
}

void sync::Panel::toggleRecord(bool) {}

void sync::Panel::toggleTimer(bool timerStatus)
{
  timerWheel->setEnabled(timerStatus);
}

void sync::Panel::syncToggle(bool) {}

void sync::Panel::updateSync() {}

void sync::Panel::updatePluginList() {}

void sync::Panel::updatePortList() {}

void sync::Panel::updateSyncPluginList() {}

sync::Device::Device()
    : RT::Device(std::string(sync::MODULE_NAME), {})
{
}

void sync::Device::read()
{
  // int64_t current_time = RT::OS::getTime();
  // if (current_time > (timerValue + dt * getValue<int64_t>(PARAMETER::TIME)))
  // {
  //   for (auto component : component_list) {
  //     component->setState(RT::State::PAUSE);
  //   }
  // }
}

void sync::Device::write() {}

std::unique_ptr<Widgets::Plugin> createRTXIPlugin(Event::Manager* ev_manager)
{
  return std::make_unique<sync::Plugin>(ev_manager);
}

Widgets::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager)
{
  return new sync::Panel(main_window, ev_manager);
}

std::unique_ptr<Widgets::Component> createRTXIComponent(
    Widgets::Plugin* /*host_plugin*/)
{
  return nullptr;
}

Widgets::FactoryMethods fact;

extern "C"
{
Widgets::FactoryMethods* getFactories()
{
  fact.createPanel = &createRTXIPanel;
  fact.createComponent = &createRTXIComponent;
  fact.createPlugin = &createRTXIPlugin;
  return &fact;
}
};

//////////// END //////////////////////
