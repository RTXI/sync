
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QTimer>

#include "widget.hpp"

#include <rtxi/fifo.hpp>
#include <rtxi/rtos.hpp>

Q_DECLARE_METATYPE(Widgets::Component*)

sync::Plugin::Plugin(Event::Manager* ev_manager)
    : Widgets::Plugin(ev_manager, std::string(sync::MODULE_NAME))
{
  sync_device = std::make_unique<sync::Device>();
  int err = RT::OS::getFifo(this->sync_pipe, 10 * sizeof(sync::response));
  if (err < 0) {
    ERROR_MSG("sync::Plugin::Constructor : Unable to create rtxi fifo");
    return;
  }
  sync_device->attachFifo(sync_pipe.get());
  Event::Object insert_device_event(Event::Type::RT_DEVICE_INSERT_EVENT);
  insert_device_event.setParam(
      "device", std::any(static_cast<RT::Device*>(this->sync_device.get())));
  getEventManager()->postEvent(&insert_device_event);
}

RT::OS::Fifo* sync::Plugin::getFifo()
{
  return this->sync_pipe.get();
}

bool sync::Plugin::isDeviceActive()
{
  return sync_device->isRunning();
}

sync::Plugin::~Plugin()
{
  Event::Object unplug_block_event(Event::Type::RT_DEVICE_REMOVE_EVENT);
  unplug_block_event.setParam(
      "device", std::any(static_cast<RT::Device*>(this->sync_device.get())));
  this->getEventManager()->postEvent(&unplug_block_event);
  sync_pipe->close();
}

void sync::Plugin::receiveEvent(Event::Object* event)
{
  auto* sync_panel = dynamic_cast<sync::Panel*>(this->getPanel());
  RT::Thread* removed_thread = nullptr;
  switch (event->getType()) {
    case Event::Type::RT_THREAD_INSERT_EVENT:
      sync_panel->signalSyncPluginList();
      break;
    case Event::Type::RT_THREAD_REMOVE_EVENT:
      removed_thread = std::any_cast<RT::Thread*>(event->getParam("thread"));
      sync_panel->removeSyncBlock(
          dynamic_cast<Widgets::Component*>(removed_thread));
      sync_panel->signalSyncPluginList();
      break;
    default:
      break;
  }
}

sync::Panel::Panel(QMainWindow* main_window, Event::Manager* ev_manager)
    : Widgets::Panel(std::string(sync::MODULE_NAME), main_window, ev_manager)
    , timerWheel(new QSpinBox(this))
    , pluginList(new QComboBox())
    , synchronizedPluginsList(new QListWidget())
    , record_timer(new QTimer(this))
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

  // Initialize plugin group
  auto* pluginGroupBox = new QGroupBox("Plugins");
  auto* pluginGroupLayout = new QVBoxLayout();

  pluginGroupBox->setLayout(pluginGroupLayout);
  pluginGroupLayout->addWidget(pluginList);
  QObject::connect(pluginList,
                   QOverload<int>::of(&QComboBox::activated),
                   this,
                   &sync::Panel::highlightSyncItem);
  QObject::connect(this,
                   &sync::Panel::signalSyncPluginList,
                   this,
                   &sync::Panel::updatePluginList);

  addPluginButton = new QPushButton("Sync");
  addPluginButton->setCheckable(true);
  addPluginButton->setEnabled(true);
  QObject::connect(pluginList,
                   QOverload<int>::of(&QComboBox::activated),
                   this,
                   QOverload<>::of(&sync::Panel::updateSyncButton));

  pluginGroupLayout->addWidget(addPluginButton);
  pluginGroupLayout->addWidget(synchronizedPluginsList);
  customlayout->addWidget(pluginGroupBox);
  QObject::connect(addPluginButton,
                   &QPushButton::clicked,
                   this,
                   &sync::Panel::updateSyncPluginList);
  QObject::connect(synchronizedPluginsList,
                   &QListWidget::currentItemChanged,
                   this,
                   QOverload<QListWidgetItem*, QListWidgetItem*>::of(
                       &sync::Panel::updateSyncButton));
  QObject::connect(synchronizedPluginsList,
                   &QListWidget::itemClicked,
                   this,
                   &sync::Panel::reverseHighlightSyncItem);

  dataCheckBox = new QCheckBox("&Sync Data Recorder");
  dataCheckBox->setEnabled(true);
  dataCheckBox->setChecked(false);
  QObject::connect(
      dataCheckBox, &QCheckBox::toggled, this, &sync::Panel::toggleRecord);
  timerCheckBox = new QCheckBox("&Sync Timer (s)");
  auto* timerBox = new QGroupBox("Timer Settings");
  auto* timerLayout = new QGridLayout();
  timerCheckBox->setWhatsThis(
      "Timer for turning sync off after a user-specified time period");
  timerCheckBox->setEnabled(true);
  timerCheckBox->setChecked(false);
  QObject::connect(
      timerCheckBox, &QCheckBox::toggled, this, &sync::Panel::toggleTimer);

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
  pauseCustomButton->setChecked(true);
  QObject::connect(pauseCustomButton,
                   &QPushButton::clicked,
                   this,
                   &sync::Panel::pauseToggle);
  buttonLayout->addWidget(pauseCustomButton);

  modifyCustomButton = new QPushButton("Modify", this);
  QObject::connect(
      modifyCustomButton, &QPushButton::clicked, this, &sync::Panel::modify);
  buttonLayout->addWidget(modifyCustomButton);

  unloadCustomButton = new QPushButton("Unload", this);
  QObject::connect(unloadCustomButton,
                   &QPushButton::clicked,
                   parentWidget(),
                   &QWidget::close);
  buttonLayout->addWidget(unloadCustomButton);

  buttonGroup->setLayout(buttonLayout);
  customlayout->addWidget(buttonGroup);

  pauseCustomButton->setDown(true);
  record_timer->start(1000);
  QObject::connect(
      record_timer, &QTimer::timeout, this, &sync::Panel::updateRecordTime);
  QObject::connect(
      record_timer, &QTimer::timeout, this, &sync::Panel::update_pause_slot);
  QTimer::singleShot(0, this, SLOT(resizeMe()));
  // Initialize plugin list
  updatePluginList();
}

void sync::Panel::toggleRecord(bool recording)
{
  syncRecorder = recording;
}

void sync::Panel::toggleTimer(bool timing)
{
  timerWheel->setEnabled(timing);
}

void sync::Panel::update_pause_slot()
{
  auto* hplugin = dynamic_cast<sync::Plugin*>(getHostPlugin());
  if (hplugin == nullptr) {
    return;
  }
  const bool paused = !hplugin->isDeviceActive();
  this->pauseCustomButton->setDown(paused);
  this->pauseCustomButton->setChecked(paused);
}

void sync::Panel::modify()
{
  // modify button will automatically update parameters and pause the sync
  // device
  time = 0;
  ready = true;
  syncState->setText("Ready");
  pauseToggle();
}

void sync::Panel::pauseToggle()
{
  auto* hplugin = dynamic_cast<sync::Plugin*>(getHostPlugin());
  if (hplugin == nullptr) {
    pauseCustomButton->setDown(true);
    pauseCustomButton->setChecked(true);
    ready = false;
    time = 0;
    syncState->setText(QString("Try Again"));
    return;
  }
  RT::OS::Fifo* ui_fifo = hplugin->getFifo();
  if (ui_fifo == nullptr) {
    pauseCustomButton->setDown(true);
    pauseCustomButton->setChecked(true);
    ready = false;
    time = 0;
    syncState->setText(QString("Try Again"));
    return;
  }
  sync::message message;
  sync::message* message_ptr = &message;
  sync::response response;
  std::vector<Widgets::Component*> block_list_copy(synchronizedBlocks);
  message.start = !pauseCustomButton->isChecked();
  message.timing = timerCheckBox->isChecked() ? timerWheel->text().toInt() : -1;
  message.block_list = &block_list_copy;
  ui_fifo->write(&message_ptr, sizeof(sync::message*));
  ui_fifo->poll();
  ui_fifo->read(&response, sizeof(sync::response));
  if (response.running != message.start) {
    ERROR_MSG(
        "sync::Panel::Pause : Unable to synchronize all plugins. Sync plugin "
        "paused");
    ready = false;
    pauseCustomButton->setDown(true);
    pauseCustomButton->setChecked(true);
    syncState->setText("Error!");
    return;
  }
  ready = true;
  time = 0;
  syncState->setText("Ready");
  if (syncRecorder) {
    Event::Type event_type = !pauseCustomButton->isChecked()
        ? Event::Type::START_RECORDING_EVENT
        : Event::Type::STOP_RECORDING_EVENT;
    Event::Object event(event_type);
    getRTXIEventManager()->postEvent(&event);
  }
  pauseCustomButton->setDown(!ready);
  pauseCustomButton->setChecked(!ready);
}

void sync::Panel::highlightSyncItem()
{
  Widgets::Component* block = nullptr;
  auto block_variant = pluginList->currentData();
  if (block_variant.isValid()) {
    block = block_variant.value<Widgets::Component*>();
  } else {
    return;
  }
  Widgets::Component* current_block = nullptr;
  for (int row = 0; row < synchronizedPluginsList->count(); row++) {
    current_block = synchronizedPluginsList->item(row)
                        ->data(Qt::UserRole)
                        .value<Widgets::Component*>();
    if (current_block == block) {
      synchronizedPluginsList->setCurrentRow(row);
      return;
    }
  }
  // If we don't find the block, unselect all rows
  synchronizedPluginsList->setCurrentRow(-1);
}

void sync::Panel::reverseHighlightSyncItem()
{
  auto block = synchronizedPluginsList->currentItem()->data(Qt::UserRole);
  pluginList->setCurrentIndex(pluginList->findData(block));
  addPluginButton->setChecked(block.isValid());
  addPluginButton->setDown(block.isValid());
}

void sync::Panel::updatePluginList()
{
  auto prev_selected_plugin = pluginList->currentData();
  pluginList->clear();
  Event::Object available_plugins_request(Event::Type::IO_BLOCK_QUERY_EVENT);
  getRTXIEventManager()->postEvent(&available_plugins_request);
  auto block_list = std::any_cast<std::vector<IO::Block*>>(
      available_plugins_request.getParam("blockList"));
  for (auto* block : block_list) {
    if (block->getName() == MODULE_NAME) {
      continue;
    }  // ignore ourselves
    if (block->getName().find("Probe") != std::string::npos) {
      continue;
    }  // ignore oscilloscope
    if (block->getName().find("Recorder") != std::string::npos) {
      continue;
    }  // ignore recorder
    if (!block->dependent()) {
      continue;
    }  // we only care about threads
    pluginList->addItem(
        QString(block->getName().c_str()) + " "
            + QString::number(block->getID()),
        QVariant::fromValue(dynamic_cast<Widgets::Component*>(block)));
  }
  pluginList->setCurrentIndex(pluginList->findData(prev_selected_plugin));
  // The user could have removed a plugin, which would be very bad and crash the
  // program... unless we stop the sync plugin to avoid nullptr access'
  pauseCustomButton->setDown(true);
  ready = false;
  pauseToggle();
}

void sync::Panel::updateSyncPluginList()
{
  auto block_variant = pluginList->currentData();
  // Don't bother to run if there is no valid plugin selected
  if (!block_variant.isValid()) {
    return;
  }
  auto* block = block_variant.value<Widgets::Component*>();
  QListWidgetItem* item = nullptr;
  const QString name = QString(block->getName().c_str()) + QString(" ")
      + QString::number(block->getID());
  QString temp_name;
  if (addPluginButton->isChecked()) {
    synchronizedBlocks.push_back(block);
    item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, QVariant::fromValue(block));
    synchronizedPluginsList->addItem(item);
  } else {
    synchronizedPluginsList->clear();
    auto iter =
        std::find(synchronizedBlocks.begin(), synchronizedBlocks.end(), block);
    if (iter != synchronizedBlocks.end()) {
      synchronizedBlocks.erase(iter);
    }
    for (auto* block : synchronizedBlocks) {
      temp_name = QString(block->getName().c_str()) + QString(" ")
          + QString::number(block->getID());
      item = new QListWidgetItem(temp_name);
      item->setData(Qt::UserRole, QVariant::fromValue(block));
      synchronizedPluginsList->addItem(item);
    }
  }
  // addPluginButton->setDown(addPluginButton->isChecked());
  ready = false;
  syncState->setText("Modify");
}

void sync::Panel::removeSyncBlock(Widgets::Component* component)
{
  auto iter = std::find(
      synchronizedBlocks.begin(), synchronizedBlocks.end(), component);
  if (iter != synchronizedBlocks.end()) {
    synchronizedBlocks.erase(iter);
  }
  // We should update the ui list as well
  QString temp_name;
  QListWidgetItem* item = nullptr;
  synchronizedPluginsList->clear();
  for (auto* block : synchronizedBlocks) {
    temp_name = QString(block->getName().c_str()) + QString(" ")
        + QString::number(block->getID());
    item = new QListWidgetItem(temp_name);
    item->setData(Qt::UserRole, QVariant::fromValue(block));
    synchronizedPluginsList->addItem(item);
  }
}

void sync::Panel::updateSyncButton(QListWidgetItem* current,
                                   QListWidgetItem* /*previous*/)
{
  if (current != nullptr) {
    addPluginButton->setChecked(true);
    addPluginButton->setDown(true);
  }
}

void sync::Panel::updateSyncButton()
{
  const auto block_variant = pluginList->currentData();
  if (!block_variant.isValid()) {
    addPluginButton->setChecked(false);
    addPluginButton->setDown(false);
    return;
  }
  auto iter = std::find(synchronizedBlocks.begin(),
                        synchronizedBlocks.end(),
                        block_variant.value<Widgets::Component*>());
  addPluginButton->setChecked(iter != synchronizedBlocks.end());
}

void sync::Panel::updateRecordTime()
{
  if (ready.load() && !pauseCustomButton->isChecked()) {
    timeElapsed->setText(QString::number(time.load()) + QString(" sec"));
    time++;
  }
}

sync::Device::Device()
    : RT::Device(std::string(sync::MODULE_NAME), {})
    , start(false)
    , startTimerValue(0)
    , dt(0)
{
  this->setActive(true);
}

void sync::Device::read()
{
  // device doesn't do anything if there is no fifo attached
  if (this->rt_fifo == nullptr) {
    return;
  }

  sync::message* message_ptr = nullptr;
  sync::response response;
  // grab message. We only grab the first one
  if (rt_fifo->readRT(&message_ptr, sizeof(sync::message*)) > 0) {
    std::swap(block_list, *(message_ptr->block_list));
    const RT::State::state_t state =
        message_ptr->start ? RT::State::EXEC : RT::State::PAUSE;
    for (auto* block : block_list) {
      block->setState(state);
    }
    start = message_ptr->start;
    dt = message_ptr->timing >= 0
        ? RT::OS::SECONDS_TO_NANOSECONDS * message_ptr->timing
        : -1;
    startTimerValue = RT::OS::getTime();
    response.running = start;
    rt_fifo->writeRT(&response, sizeof(sync::response));
  }
}

void sync::Device::write()
{
  if (!start) {
    for (auto* block : block_list) {
      block->setState(RT::State::PAUSE);
    }
    return;
  }

  if (dt < 0) {
    return;
  }
  int64_t current_time = RT::OS::getTime();
  if (current_time > (startTimerValue + dt)) {
    start = false;
    for (auto* block : block_list) {
      block->setState(RT::State::PAUSE);
    }
  } else {
    for (auto* block : block_list) {
      block->setState(RT::State::EXEC);
    }
  }
}

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
