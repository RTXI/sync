
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

typedef struct message {
  bool record; // whether to start recording or not in the real-time thread
  int timing; // -1 if no timing, >=0 is timing in seconds
  std::vector<Widgets::Component*>* block_list; // pre-allocated vector of components to sync
}message;

typedef struct response {
  bool running=false; // the ui side will be waiting for this to check whether sync device is operational
}response;

class Panel : public Widgets::Panel
{
  Q_OBJECT
public:
  Panel(QMainWindow* main_window, Event::Manager* ev_manager);

  void removeSyncBlock(Widgets::Component* component);
signals:
  void signalSyncPluginList();

private slots:
  void toggleRecord(bool recording);
  void toggleTimer(bool timing);
  void modify() override;
  void pauseToggle(bool paused);
  void highlightSyncItem();
  void reverseHighlightSyncItem();
  void updatePluginList();
  void updateSyncPluginList();
  void updateSyncButton(QListWidgetItem* current, QListWidgetItem* previous);
  void updateSyncButton();
  void updateRecordTime();

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
  QLabel* syncState=nullptr;
  QLabel* timeElapsed=nullptr;
  QListWidget* synchronizedPluginsList=nullptr;
  QTimer* record_timer=nullptr;
  std::vector<Widgets::Component*> synchronizedBlocks;
  std::atomic<int> time=0;
  std::atomic<bool> ready=false;
  std::atomic<bool> syncRecorder=false;
};

class Device : public RT::Device 
{
public:
  explicit Device();
  void read() override;
  void write() override;
  void attachFifo(RT::OS::Fifo* fifo){ this->rt_fifo=fifo; }
  bool isRunning() const{ return startDataRecorder; }

private:
  bool startDataRecorder;
  int64_t startTimerValue;
  std::vector<Widgets::Component*> block_list;
  RT::OS::Fifo* rt_fifo=nullptr;
  int64_t dt;
};

class Plugin final : public Widgets::Plugin
{
public:
  explicit Plugin(Event::Manager* ev_manager);
  ~Plugin() final;
  RT::OS::Fifo* getFifo();
  void receiveEvent(Event::Object* event) override;
  bool isDeviceActive();
private:
  std::unique_ptr<Device> sync_device;
  std::unique_ptr<RT::OS::Fifo> sync_pipe;
};


}  // namespace sync
