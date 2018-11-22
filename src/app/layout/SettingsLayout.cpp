#include "SettingsLayout.h"
#include <f8n/debug/debug.h>
#include <f8n/i18n/Locale.h>
#include <f8n/utf/str.h>
#include <cursespp/App.h>
#include <cursespp/Screen.h>
#include <cursespp/PluginOverlay.h>
#include <autom8/device/device_system.hpp>
#include <app/util/Message.h>
#include <app/util/Settings.h>
#include <app/overlay/DeviceEditOverlay.h>

using namespace cursespp;
using namespace autom8::app;
using namespace f8n;

static bool isDeleteKey(const std::string& kn) {
#ifdef __APPLE__
    return kn == "KEY_BACKSPACE";
#else
    return kn == "KEY_DC";
#endif
}

SettingsLayout::SettingsLayout(autom8::client_ptr client)
: LayoutBase()
, client(client) {
    int order = 0;

    this->aboutConfig = std::make_shared<TextLabel>();
    this->aboutConfig->SetText(_TSTR("settings_about_config"));
    this->aboutConfig->SetFocusOrder(order++);
    this->aboutConfig->Activated.connect(this, &SettingsLayout::OnAboutConfigActivated);
    this->AddWindow(aboutConfig);

    this->deviceController = std::make_shared<TextLabel>();
    this->deviceController->SetFocusOrder(order++);
    this->deviceController->Activated.connect(this, &SettingsLayout::OnDeviceControllerActivated);
    this->AddWindow(deviceController);

    this->addDevice = std::make_shared<TextLabel>();
    this->addDevice->SetFocusOrder(order++);
    this->addDevice->Activated.connect(this, &SettingsLayout::OnAddDeviceActivated);
    this->AddWindow(addDevice);

    this->deviceModelAdapter = std::make_shared<DeviceModelAdapter>(device_system::instance());
    this->deviceModelList = std::make_shared<ListWindow>(this->deviceModelAdapter);
    this->deviceModelList->EntryActivated.connect(this, &SettingsLayout::OnDeviceRowActivated);
    this->deviceModelList->SetFocusOrder(order++);
    this->AddWindow(this->deviceModelList);

    this->Reload();
}

void SettingsLayout::OnLayout() {
    auto cx = this->GetContentWidth() - 1;
    auto cy = this->GetContentHeight();
    int x = 1;
    int y = 0;
    this->aboutConfig->MoveAndResize(x, y++, cx, 1);
    this->deviceController->MoveAndResize(x, y++, cx, 1);
    ++y;
    this->addDevice->MoveAndResize(x, y++, cx, 1);
    this->deviceModelList->MoveAndResize(0, y, cx + 1, cy - y);
}

void SettingsLayout::ProcessMessage(f8n::runtime::IMessage& message) {

}

void SettingsLayout::Reload() {
    auto controller = settings::Prefs()->GetString(settings::SYSTEM_SELECTED);

    this->deviceController->SetText(str::format(
        _TSTR("settings_device_controller"), controller.c_str()));

    this->addDevice->SetText(str::format(
        _TSTR("settings_add_device"), controller.c_str()));

    this->deviceModelList->SetFrameTitle(str::format(
        _TSTR("settings_device_system_list"), controller.c_str()));

    this->deviceModelAdapter->Requery(device_system::instance());
    this->deviceModelList->OnAdapterChanged();
}

void SettingsLayout::OnDeviceRowActivated(cursespp::ListWindow* window, size_t index) {
    DeviceEditOverlay::Edit(
        device_system::instance(),
        this->deviceModelAdapter->At(index),
        [this]() { this->Reload(); });
}

bool SettingsLayout::KeyPress(const std::string& kn) {
    if (kn == "d") {
        Broadcast(message::BROADCAST_SWITCH_TO_CLIENT_LAYOUT);
        return true;
    }
    else if (this->GetFocus() == this->deviceModelList && isDeleteKey(kn)) {
        auto index = this->deviceModelList->GetSelectedIndex();
        if (index != ListWindow::NO_SELECTION) {
            DeviceEditOverlay::Delete(
                device_system::instance(),
                this->deviceModelAdapter->At(index),
                [this]() { this->Reload(); });
        }
        return true;
    }

    return LayoutBase::KeyPress(kn);
}

void SettingsLayout::OnAboutConfigActivated(cursespp::TextLabel* label) {
    PluginOverlay::Show(
        "settings",
        settings::Prefs(),
        settings::Schema(),
        [this](bool changed) {
            if (changed) {
                debug::i("ClientLayout", "settings saved, reconnecting...");
                this->Reload();
                auto prefs = settings::Prefs();
                prefs->Save();
                this->client->disconnect(true);
                this->client->connect(
                    prefs->GetString(settings::CLIENT_HOSTNAME),
                    prefs->GetInt(settings::CLIENT_PORT),
                    prefs->GetString(settings::CLIENT_PASSWORD));
            }
        });
}

void SettingsLayout::OnDeviceControllerActivated(cursespp::TextLabel* label) {

}

void SettingsLayout::OnAddDeviceActivated(cursespp::TextLabel* label) {
    DeviceEditOverlay::Create(
        device_system::instance(),
        [this]() { this->Reload(); });
}

void SettingsLayout::SetShortcutsWindow(cursespp::ShortcutsWindow* shortcuts) {
    debug::info("SettingsLayout", "SetShortcutsWindow()");

    if (shortcuts) {
        shortcuts->AddShortcut("d", _TSTR("shortcuts_devices"));
        shortcuts->AddShortcut("s", _TSTR("shortcuts_settings"));
        shortcuts->AddShortcut("^D", _TSTR("shortcuts_quit"));

        shortcuts->SetChangedCallback([this](std::string key) {
            if (key == "^D") {
                App::Instance().Quit();
            }
            else {
                this->KeyPress(key);
            }
        });

        shortcuts->SetActive("s");
    }
}