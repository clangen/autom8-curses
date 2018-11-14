#include "MainLayout.h"

#include <cursespp/Colors.h>

using namespace autom8;
using namespace autom8::app;
using namespace cursespp;
using namespace f8n::runtime;

static const int UPDATE_STATUS_MESSAGE = 1024;

MainLayout::MainLayout(client_ptr client)
: LayoutBase()
, client(client) {
    this->deviceListAdapter = std::make_shared<DeviceListAdapter>(
        client, Window::MessageQueue(), [this]() {
            this->deviceList->OnAdapterChanged();
        });

    this->clientStatus = std::make_shared<TextLabel>();
    this->clientStatus->SetText("client: disconnected", text::AlignCenter);
    this->clientStatus->SetContentColor(Color::Banner);
    this->AddWindow(clientStatus);

    this->serverStatus = std::make_shared<TextLabel>();
    this->serverStatus->SetText("server: stopped", text::AlignCenter);
    this->serverStatus->SetContentColor(Color::Banner);
    this->AddWindow(serverStatus);

    this->deviceList = std::make_shared<ListWindow>(this->deviceListAdapter);
    this->deviceList->SetFrameTitle("devices");
    this->AddWindow(this->deviceList);
    this->deviceList->SetFocusOrder(0);

    client->state_changed.connect(this, &MainLayout::OnClientStateChanged);
    autom8::server::started.connect(this, &MainLayout::OnServerStateChanged);
    autom8::server::stopped.connect(this, &MainLayout::OnServerStateChanged);

    this->Update();
}

void MainLayout::OnLayout() {
    int cx = this->GetContentWidth();
    int cy = this->GetContentHeight();
    this->clientStatus->MoveAndResize(0, 0, (cx / 2) - 1, 1);
    this->serverStatus->MoveAndResize(cx / 2, 0, cx - (cx / 2), 1);
    this->deviceList->MoveAndResize(0, 1, cx, cy - 1);
}

void MainLayout::ProcessMessage(IMessage& message){
    if (message.Type() == UPDATE_STATUS_MESSAGE) {
        this->Update();
    }
}

void MainLayout::Update() {
    using S = autom8::client::connection_state;

    auto str = "disconnected";
    auto color = Color::Banner;
    switch (client->state()) {
        case S::state_connecting:
            str = "connecting";
            break;
        case S::state_connected:
            str = "connected";
            color = Color::Footer;
            break;
        case S::state_disconnecting:
            str = "disconnecting";
            break;
        default:
            break;
    }

    this->clientStatus->SetText(std::string("client: ") + str, cursespp::text::AlignCenter);
    this->clientStatus->SetContentColor(Color(color));

    str = "stopped";
    color = Color::Banner;
    if (autom8::server::is_running()) {
        str = "running";
        color = Color::Footer;
    }

    this->serverStatus->SetText(std::string("server: ") + str, cursespp::text::AlignCenter);
    this->serverStatus->SetContentColor(Color(color));
}

void MainLayout::OnServerStateChanged() {
    this->Post(UPDATE_STATUS_MESSAGE);
}

void MainLayout::OnClientStateChanged(autom8::client::connection_state state, autom8::client::reason reason) {
    this->Post(UPDATE_STATUS_MESSAGE);
}