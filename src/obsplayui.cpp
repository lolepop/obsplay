#include "obsplayui.h"
#include "./ui_obsplayui.h"
#include "native.h"

ObsPlayUI::ObsPlayUI(const std::shared_ptr<Settings>& settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ObsPlayUI)
{
    this->settings = settings;

    ui->setupUi(this);

    connect(ui->whitelistBtn, &QPushButton::clicked, this, &ObsPlayUI::addWhitelist);
    connect(ui->removeBtn, &QPushButton::clicked, this, &ObsPlayUI::removeWhitelist);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &ObsPlayUI::close);
    connect(ui->saveBtn, &QPushButton::clicked, this, &ObsPlayUI::saveSettings);

    QStringList list;
    // list << "testing" << "testing 123";

    whitelistModel = new QStringListModel(this);
    whitelistModel->setStringList(list);
    ui->whitelist->setModel(whitelistModel);
    ui->whitelist->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);

    loadSettings();
    populateWindowBox();
}

void ObsPlayUI::addWhitelistItem(QString item)
{
    // ignore duplicates
    auto entries = whitelistModel->stringList();
    for (auto i : entries)
    {
        if (item == i)
            return;
    }

    int row = whitelistModel->rowCount();
    whitelistModel->insertRow(row);
    auto i = whitelistModel->index(row);
    whitelistModel->setData(i, item);
    ui->whitelist->setCurrentIndex(i);
}

void ObsPlayUI::addWhitelist()
{
    auto selectedWindow = ui->windowBox->currentText();
    addWhitelistItem(selectedWindow);
}

void ObsPlayUI::removeWhitelist()
{
    auto selectedIndex = ui->whitelist->currentIndex();
    whitelistModel->removeRow(selectedIndex.row());
}

void ObsPlayUI::saveSettings()
{
    auto s = settings->getSettings();

    obs_data_set_bool(s, "enabled", ui->enableCheckBox->isChecked());
    obs_data_set_int(s, "timeout", ui->timeoutBox->value());
    obs_data_set_int(s, "interval", ui->intervalBox->value());

    auto whitelist = obs_data_array_create();
    auto entries = whitelistModel->stringList();
    for (auto i : entries)
    {
        auto e = obs_data_create();
        obs_data_set_string(e, "process", i.toStdString().c_str());
        obs_data_array_push_back(whitelist, e);
        obs_data_release(e);
    }
    
    obs_data_set_array(s, "whitelist", whitelist);
    obs_data_array_release(whitelist);
    
    // Settings::saveSettings(settings);
    settings->saveSettings();
    close();
}

void ObsPlayUI::loadSettings()
{
    auto s = settings->getSettings();
    
    ui->enableCheckBox->setChecked(obs_data_get_bool(s, "enabled"));
    ui->timeoutBox->setValue(obs_data_get_int(s, "timeout"));
    ui->intervalBox->setValue(obs_data_get_int(s, "interval"));
    
    auto whitelist = Settings::getArray(s, "whitelist");
    for (size_t i = 0; i < obs_data_array_count(whitelist); i++)
    {
        obs_data_t* item = obs_data_array_item(whitelist, i);
        auto p = obs_data_get_string(item, "process");
        addWhitelistItem(QString(p));
		obs_data_release(item);
    }
}

void ObsPlayUI::populateWindowBox()
{
    auto windows = Native::getAllWindows();
    std::vector winVec(windows.begin(), windows.end());
    std::sort(winVec.begin(), winVec.end());

    ui->windowBox->clear();

    for (auto i = winVec.begin(); i != winVec.end(); i++)
    {
        if (i == winVec.begin())
            ui->windowBox->setItemData(0, i->c_str());
        else
            ui->windowBox->addItem(i->c_str());
    }
}

ObsPlayUI::~ObsPlayUI()
{
    delete ui;
}

