#ifndef OBSPLAYUI_H
#define OBSPLAYUI_H

#include <QDialog>
#include <QPushButton>
#include <QListView>
#include <QStringListModel>
#include <QMessageBox>
#include <memory>
#include "settings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ObsPlayUI; }
QT_END_NAMESPACE

class ObsPlayUI : public QDialog
{
    Q_OBJECT

public:
    ObsPlayUI(const std::shared_ptr<Settings>& settings, QWidget *parent = nullptr);
    ~ObsPlayUI();

public slots:
    void addWhitelist();
    void removeWhitelist();
    void saveSettings();

private:
    Ui::ObsPlayUI *ui;
    // OBSData settings;
    std::shared_ptr<Settings> settings;

    QStringListModel* whitelistModel;

    void addWhitelistItem(QString item);
    void refreshWindowList();
    void loadSettings();
    void populateWindowBox();
};
#endif // OBSPLAYUI_H
