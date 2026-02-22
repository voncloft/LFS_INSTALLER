#pragma once

#include <QMainWindow>

class QCheckBox;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QWidget;

class InstallerWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit InstallerWindow(QWidget* parent = nullptr);

private slots:
    void goBack();
    void goNext();
    void refreshDriveList();

private:
    QWidget* buildWelcomePage();
    QWidget* buildDrivePage();
    void updateNavButtons();

    QStackedWidget* pages_ = nullptr;
    QPushButton* backButton_ = nullptr;
    QPushButton* nextButton_ = nullptr;
    QListWidget* driveList_ = nullptr;
};
