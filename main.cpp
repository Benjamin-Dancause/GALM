#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QCheckBox>
#include <string>
#include <iostream>
#include <filesystem>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QMessageBox>
#include "launcher.h"


class PathSelectorWidget : public QWidget {
    Q_OBJECT  // Required for signals/slots – works because we enable CMAKE_AUTOMOC

public:
    explicit PathSelectorWidget(QWidget *parent = nullptr) : QWidget(parent) {
        // ---- Main vertical layout ----
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // ---- Row 0 App name: Label + Name field ----
        QHBoxLayout *row0 = new QHBoxLayout();
        QLabel *label0 = new QLabel("App name:", this);
        m_appName = new QLineEdit(this);
        m_appName->setPlaceholderText("Write name of the app launcher");
        row0->addWidget(label0);
        row0->addWidget(m_appName);
        mainLayout->addLayout(row0);

        // ---- Row 1 (Exec file): Label + Path for Path 1 + Browse button ----
        QHBoxLayout *row1 = new QHBoxLayout();
        QLabel *label1 = new QLabel("Exec Path:", this);
        m_execFilePath = new QLineEdit(this);
        m_execFilePath->setPlaceholderText("Select an executable file");
        m_execFilePath->setReadOnly(true);
        QPushButton *m_browseButtonExec = new QPushButton("...", this);
        m_browseButtonExec->setFixedWidth(35);
        row1->addWidget(label1);
        row1->addWidget(m_execFilePath);
        row1->addWidget(m_browseButtonExec);
        mainLayout->addLayout(row1);


        // ---- Row 2: Label + Line Edit for Path 2 ----
        QHBoxLayout *row2 = new QHBoxLayout();
        QLabel *label2 = new QLabel("Icon (optional):", this);
        m_iconNamePath = new QLineEdit(this);
        m_iconNamePath->setPlaceholderText("Select an icon (optional)");
        m_iconNamePath->setReadOnly(true);
        QPushButton *m_browseButtonIcon = new QPushButton("...", this);
        m_browseButtonIcon->setFixedWidth(35);
        row2->addWidget(label2);
        row2->addWidget(m_iconNamePath);
        row2->addWidget(m_browseButtonIcon);
        mainLayout->addLayout(row2);

        // ---- Row 3: The two buttons + checkbox (placed side-by-side below the fields) ----
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        m_createButton = new QPushButton("Create app launcher", this);
        m_createButton->setEnabled(false);
        m_systemwideCheckBox = new QCheckBox("Make systemwide launcher",this);
        m_systemwideCheckBox->setStyleSheet("QCheckBox::indicator {width: 24px;height: 24px;}");
        buttonLayout->addWidget(m_createButton);
        buttonLayout->addWidget(m_systemwideCheckBox);
        mainLayout->addLayout(buttonLayout);

        // ---- Connect buttons to their slots ----
        connect(m_appName, &QLineEdit::textChanged, this, &PathSelectorWidget::onTextUpdate);
        connect(m_browseButtonExec, &QPushButton::clicked, this, &PathSelectorWidget::onBrowseExec);
        connect(m_browseButtonIcon, &QPushButton::clicked, this, &PathSelectorWidget::onBrowseIcon);
        connect(m_createButton, &QPushButton::clicked, this, &PathSelectorWidget::onCreate);

        setLayout(mainLayout);
        setWindowTitle("GALM — GNOME App Launcher Maker");
        setFixedSize(600,175);

        std::cout << std::string(std::getenv("HOME")) + "/.local/share/applications/" << std::endl;
    }

private slots:

    void onTextUpdate() {
        checkValidity();
    }

    void onBrowseExec() {
        QString filePath = QFileDialog::getOpenFileName(this, "Select the exec file");
        if (std::filesystem::is_regular_file(filePath.toStdString())) {
            m_execFilePath->setText(filePath);
        }
        checkValidity();
    }

    void onBrowseIcon() {
        QString filePath = QFileDialog::getOpenFileName(this, "Select the icon file");
        if (std::filesystem::is_regular_file(filePath.toStdString())) {
            m_iconNamePath->setText(filePath);
        }
        checkValidity();
    }

    void onCreate() {
        checkValidity();
        if (!m_createButton->isEnabled()) {
            return;
        }

        const auto showError = [this](const QString &message) {
            QMessageBox::critical(this, "Could not create launcher", message);
        };
        if (m_execFilePath->text().contains('=')) {
            showError("Executable paths cannot contain '=' in a desktop entry. Rename the executable or its containing directory.");
            return;
        }
        QFile templateFile(":/Template.desktop");
        if (!templateFile.open(QIODevice::ReadOnly)) {
            showError("Could not read the bundled launcher template: " + templateFile.errorString());
            return;
        }
        QString content = QString::fromUtf8(templateFile.readAll());
        if (templateFile.error() != QFileDevice::NoError || content.trimmed().isEmpty()) {
            showError("The bundled launcher template could not be read or is empty.");
            return;
        }
        content = renderLauncher(content, m_appName->text(),
                                 m_execFilePath->text(), m_iconNamePath->text());

        const QString directory = m_systemwideCheckBox->isChecked()
            ? "/usr/share/applications"
            : QDir::homePath() + "/.local/share/applications";
        if (!QDir().mkpath(directory)) {
            showError("Could not create the applications directory: " + directory);
            return;
        }
        const QString desktopFilePath = QDir(directory).filePath(m_appName->text() + ".desktop");
        QSaveFile desktopFile(desktopFilePath);
        if (!desktopFile.open(QIODevice::WriteOnly)) {
            showError("Could not open " + desktopFilePath + ": " + desktopFile.errorString());
            return;
        }
        const QByteArray data = content.toUtf8();
        if (desktopFile.write(data) != data.size()) {
            showError("Could not write " + desktopFilePath + ": " + desktopFile.errorString());
            return;
        }
        if (!desktopFile.commit()) {
            showError("Could not save " + desktopFilePath + ": " + desktopFile.errorString());
            return;
        }

        QApplication::quit();
    }

private:
    QLineEdit *m_appName;
    QLineEdit *m_execFilePath;
    QLineEdit *m_iconNamePath;
    QPushButton *m_createButton;
    QCheckBox *m_systemwideCheckBox;

    void checkValidity() {
        const QString name = m_appName->text();
        bool isNameValid = !name.trimmed().isEmpty() && !name.contains('/')
            && !name.contains('\n') && !name.contains('\r') && !name.contains(QChar::Null);
        bool isExecPathValid = !m_execFilePath->text().isEmpty();
        m_createButton->setEnabled(isNameValid && isExecPathValid);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PathSelectorWidget widget;
    widget.show();
    return app.exec();
}

#include "main.moc"
