
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QListWidget>
#include <rtxi/widgets.hpp>

// This is an generated header file. You may change the namespace, but
// make sure to do the same in implementation (.cpp) file
namespace sync
{

constexpr std::string_view MODULE_NAME = "sync";

inline std::vector<Widgets::Variable::Info> get_default_vars()
{
  return {};
}

inline std::vector<IO::channel_t> get_default_channels()
{
  return {};
}

class Panel : public Widgets::Panel
{
  Q_OBJECT
public:
  Panel(QMainWindow* main_window, Event::Manager* ev_manager);

private slots:
  void toggleRecord(bool);
  void toggleTimer(bool);
  void syncToggle(bool);
  void updateSync();
  void updatePluginList();
  void updatePortList();
  void updateSyncPluginList();

private:
  QCheckBox* dataCheckBox=nullptr;
  QCheckBox* timerCheckBox=nullptr;
  QSpinBox* timerWheel=nullptr;
  QPushButton* addPluginButton=nullptr;
  QPushButton* removePluginButton=nullptr;
  QPushButton* pauseCustomButton=nullptr;
  QPushButton* modifyCustomButton=nullptr;
  QPushButton* unloadCustomButton=nullptr;
  QComboBox* pluginList=nullptr;
  QComboBox* directionList=nullptr;
  QComboBox* portList=nullptr;
  QLabel* syncState=nullptr;
  QLabel* timeElapsed=nullptr;
  QListWidget* synchronizedPluginsList=nullptr;
  RT::OS::Fifo* ui_fifo;
};

class Device : public RT::Device 
{
public:
  explicit Device();
  void read() override;
  void write() override;

private:
  bool startDataRecorder;
  int64_t timerValue;
  std::vector<Widgets::Component*> component_list;
  RT::OS::Fifo* rt_fifo=nullptr;
  int64_t dt;
};

class Plugin final : public Widgets::Plugin
{
public:
  explicit Plugin(Event::Manager* ev_manager);
  ~Plugin() final;
  std::unique_ptr<RT::OS::Fifo>& getFifo();
private:
  std::unique_ptr<Device> sync_device;
  std::unique_ptr<RT::OS::Fifo> sync_pipe;
};


}  // namespace sync
