#include "installerwindow.h"

#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextStream>
#include <QPushButton>
#include <QRegularExpression>
#include <QProcess>
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
    QProcess lsblk;
    lsblk.start("lsblk", {"-nrpo", "NAME,TYPE,SIZE,MOUNTPOINT"});
    if (!lsblk.waitForFinished(3000)) {
        driveList_->addItem("Failed to query block devices (lsblk timeout).");
        return;
    }

    const QString output = QString::fromUtf8(lsblk.readAllStandardOutput());
    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    const QRegularExpression sdDiskOrPartitionPattern("^/dev/sd[a-z]+(?:\\d+)?$");

    for (const QString& rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty()) {
            continue;
        }

        const QStringList fields = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (fields.size() < 3) {
            continue;
        }

        const QString device = fields.at(0);
        const QString type = fields.at(1);
        const QString size = fields.at(2);
        const QString mountPoint = fields.size() > 3 ? fields.at(3) : "<not mounted>";

        if ((type != "disk" && type != "part")
            || !sdDiskOrPartitionPattern.match(device).hasMatch()) {
            continue;
        }

        const QString label = QString("%1  |  mount: %2  |  size: %3")
                                  .arg(device, mountPoint, size);

        auto* item = new QListWidgetItem(driveList_);
        driveList_->addItem(item);
        auto* checkbox = new QCheckBox(label, driveList_);
        checkbox->setProperty("devicePath", device);
        connect(checkbox, &QCheckBox::toggled, this, [this](bool) {
            writeSelectedDevicesToFile();
        });
        driveList_->setItemWidget(item, checkbox);
        item->setSizeHint(checkbox->sizeHint());
    }

    if (driveList_->count() == 0) {
        driveList_->addItem("No /dev/sdX or /dev/sdXY devices were detected.");
    }

    writeSelectedDevicesToFile();
}

void InstallerWindow::updateNavButtons() {
    const int index = pages_->currentIndex();
    const int last = pages_->count() - 1;

    backButton_->setEnabled(index > 0);
    nextButton_->setText(index == last ? "Done" : "Next");
}

void InstallerWindow::writeSelectedDevicesToFile() {
    QFile file("/installer.sh");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    for (int i = 0; i < driveList_->count(); ++i) {
        QListWidgetItem* item = driveList_->item(i);
        auto* checkbox = qobject_cast<QCheckBox*>(driveList_->itemWidget(item));
        if (!checkbox || !checkbox->isChecked()) {
            continue;
        }

        const QString device = checkbox->property("devicePath").toString().trimmed();
        if (!device.isEmpty()) {
            out << device << '\n';
        }
    }
}
