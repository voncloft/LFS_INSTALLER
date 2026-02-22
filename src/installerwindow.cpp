#include "installerwindow.h"

#include <QCheckBox>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QStackedWidget>
#include <QStorageInfo>
#include <QVBoxLayout>
#include <QWidget>

InstallerWindow::InstallerWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("LFS Installer Beta");
    resize(820, 500);

    auto* central = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(central);

    pages_ = new QStackedWidget(central);
    pages_->addWidget(buildWelcomePage());
    pages_->addWidget(buildDrivePage());
    rootLayout->addWidget(pages_);

    auto* navLayout = new QHBoxLayout();
    navLayout->addStretch();

    backButton_ = new QPushButton("Back", central);
    nextButton_ = new QPushButton("Next", central);
    navLayout->addWidget(backButton_);
    navLayout->addWidget(nextButton_);

    rootLayout->addLayout(navLayout);
    setCentralWidget(central);

    connect(backButton_, &QPushButton::clicked, this, &InstallerWindow::goBack);
    connect(nextButton_, &QPushButton::clicked, this, &InstallerWindow::goNext);

    refreshDriveList();
    updateNavButtons();
}

QWidget* InstallerWindow::buildWelcomePage() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(16);

    auto* title = new QLabel("Welcome to the LFS Installer (Beta)", page);
    title->setStyleSheet("font-size: 26px; font-weight: 700;");

    auto* intro = new QLabel(
        "This is a GUI prototype only.\n"
        "It does not execute formatting, partitioning, or installation logic.",
        page
    );
    intro->setWordWrap(true);

    auto* instructions = new QLabel(
        "Click Next to continue to drive selection.",
        page
    );
    instructions->setWordWrap(true);

    layout->addStretch();
    layout->addWidget(title);
    layout->addWidget(intro);
    layout->addWidget(instructions);
    layout->addStretch();

    return page;
}

QWidget* InstallerWindow::buildDrivePage() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(12);

    auto* title = new QLabel("Select Drive(s) to Format", page);
    title->setStyleSheet("font-size: 22px; font-weight: 700;");

    auto* note = new QLabel(
        "Prototype behavior: this screen only captures selection.\n"
        "No format action is available in this beta.",
        page
    );
    note->setWordWrap(true);

    driveList_ = new QListWidget(page);
    driveList_->setSelectionMode(QAbstractItemView::NoSelection);

    auto* refreshButton = new QPushButton("Refresh Drive List", page);
    connect(refreshButton, &QPushButton::clicked, this, &InstallerWindow::refreshDriveList);

    layout->addWidget(title);
    layout->addWidget(note);
    layout->addWidget(driveList_, 1);
    layout->addWidget(refreshButton);

    return page;
}

void InstallerWindow::goBack() {
    const int index = pages_->currentIndex();
    if (index > 0) {
        pages_->setCurrentIndex(index - 1);
        updateNavButtons();
    }
}

void InstallerWindow::goNext() {
    const int index = pages_->currentIndex();
    if (index < pages_->count() - 1) {
        pages_->setCurrentIndex(index + 1);
        updateNavButtons();
        return;
    }

    close();
}

void InstallerWindow::refreshDriveList() {
    driveList_->clear();

    const auto volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo& volume : volumes) {
        if (!volume.isValid() || !volume.isReady()) {
            continue;
        }

        const QString device = QString::fromUtf8(volume.device());
        if (device.isEmpty()) {
            continue;
        }

        QString mountPoint = volume.rootPath();
        if (mountPoint.isEmpty()) {
            mountPoint = "<no mount point>";
        }

        const qulonglong bytes = volume.bytesTotal();
        const double gib = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
        const QString label = QString("%1  |  mount: %2  |  size: %3 GiB")
                                  .arg(device, mountPoint, QString::number(gib, 'f', 1));

        auto* item = new QListWidgetItem(driveList_);
        driveList_->addItem(item);

        auto* checkbox = new QCheckBox(label, driveList_);
        driveList_->setItemWidget(item, checkbox);
        item->setSizeHint(checkbox->sizeHint());
    }

    if (driveList_->count() == 0) {
        driveList_->addItem("No mounted drives were detected.");
    }
}

void InstallerWindow::updateNavButtons() {
    const int index = pages_->currentIndex();
    const int last = pages_->count() - 1;

    backButton_->setEnabled(index > 0);
    nextButton_->setText(index == last ? "Done" : "Next");
}
