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
#include <fstream>
#include <regex>


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
        QLabel *label2 = new QLabel("Icon Path:", this);
        m_iconNamePath = new QLineEdit(this);
        m_iconNamePath->setPlaceholderText("Select an icon");
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
        connect(m_appName, &QLineEdit::editingFinished, this, &PathSelectorWidget::onTextUpdate);
        connect(m_browseButtonExec, &QPushButton::clicked, this, &PathSelectorWidget::onBrowseExec);
        connect(m_browseButtonIcon, &QPushButton::clicked, this, &PathSelectorWidget::onBrowseIcon);
        connect(m_createButton, &QPushButton::clicked, this, &PathSelectorWidget::onCreate);

        setLayout(mainLayout);
        setWindowTitle("App launcher creation wizard");
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
        std::string desktopFilePath = std::string(std::getenv("HOME")) + "/.local/share/applications/";
        if (m_systemwideCheckBox->isChecked()) {
            std::string desktopFilePath = "/usr/share/applications";
        }

        desktopFilePath+=m_appName->text().toStdString() +".desktop";


        std::ifstream ifs("Template.desktop");
        std::string content;
        content.assign( (std::istreambuf_iterator<char>(ifs) ),
                        (std::istreambuf_iterator<char>()    ) );

        content = std::regex_replace(content, std::regex("_name_"), m_appName->text().toStdString());
        content = std::regex_replace(content, std::regex("_execPath_"), m_execFilePath->text().toStdString());
        content = std::regex_replace(content, std::regex("_iconPath_"), m_iconNamePath->text().toStdString());

        std::ofstream desktopFile(desktopFilePath);
        desktopFile << content;

        desktopFile.close();


        QApplication::quit();
    }

private:
    QLineEdit *m_appName;
    QLineEdit *m_execFilePath;
    QLineEdit *m_iconNamePath;
    QPushButton *m_createButton;
    QCheckBox *m_systemwideCheckBox;

    void checkValidity() {
        bool isNameValid = !m_execFilePath->text().isEmpty();
        bool isExecPathValid = !m_execFilePath->text().isEmpty();
        bool isIconPathValid = !m_iconNamePath->text().isEmpty();
        m_createButton->setEnabled(isNameValid && isExecPathValid && isIconPathValid);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PathSelectorWidget widget;
    widget.show();
    return app.exec();
}

#include "main.moc"