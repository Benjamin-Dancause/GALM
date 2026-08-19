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




class PathSelectorWidget : public QWidget {
    Q_OBJECT  // Required for signals/slots – works because we enable CMAKE_AUTOMOC

public:
    explicit PathSelectorWidget(QWidget *parent = nullptr) : QWidget(parent) {
        // ---- Main vertical layout ----
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // ---- Row 0 App name: Label + Name field ----
        QHBoxLayout *row0 = new QHBoxLayout();
        QLabel *label0 = new QLabel("App name:", this);
        m_pathEdit0 = new QLineEdit(this);
        m_pathEdit0->setPlaceholderText("Write name of the app launcher");
        row0->addWidget(label0);
        row0->addWidget(m_pathEdit0);
        mainLayout->addLayout(row0);

        // ---- Row 1 (Exec file): Label + Path for Path 1 + Browse button ----
        QHBoxLayout *row1 = new QHBoxLayout();
        QLabel *label1 = new QLabel("Exec Path:", this);
        m_pathEdit1 = new QLineEdit(this);
        m_pathEdit1->setPlaceholderText("Select an executable file");
        QPushButton *m_browseButton1 = new QPushButton("...", this);
        m_browseButton1->setFixedWidth(35);
        row1->addWidget(label1);
        row1->addWidget(m_pathEdit1);
        row1->addWidget(m_browseButton1);
        mainLayout->addLayout(row1);


        // ---- Row 2: Label + Line Edit for Path 2 ----
        QHBoxLayout *row2 = new QHBoxLayout();
        QLabel *label2 = new QLabel("Icon Path:", this);
        m_pathEdit2 = new QLineEdit(this);
        m_pathEdit2->setPlaceholderText("Select an icon");
        QPushButton *m_browseButton2 = new QPushButton("...", this);
        m_browseButton2->setFixedWidth(35);
        row2->addWidget(label2);
        row2->addWidget(m_pathEdit2);
        row2->addWidget(m_browseButton2);
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
        connect(m_browseButton1, &QPushButton::clicked, this, &PathSelectorWidget::onBrowse1);
        connect(m_browseButton2, &QPushButton::clicked, this, &PathSelectorWidget::onBrowse2);
        connect(m_createButton, &QPushButton::clicked, this, &PathSelectorWidget::onCreate);

        setLayout(mainLayout);
        setWindowTitle("App launcher creation wizard");
        setFixedSize(600,175);

        std::cout << std::string(std::getenv("HOME")) + "/.local/share/applications/" << std::endl;
    }

private slots:
    void onBrowse1() {
        QString filePath = QFileDialog::getOpenFileName(this, "Select the exec file");
        if (std::filesystem::is_regular_file(filePath.toStdString())) {
            m_pathEdit1->setText(filePath);
        }
    }

    void onBrowse2() {
        QString filePath = QFileDialog::getOpenFileName(this, "Select the icon file");
        if (std::filesystem::is_regular_file(filePath.toStdString())) {
            m_pathEdit2->setText(filePath);
        }
    }

    void onCreateHover() {

    }

    void onCreate() {
        std::string desktopFilePath = std::string(std::getenv("HOME")) + "/.local/share/applications/";
        if (m_systemwideCheckBox->isChecked()) {
            std::string desktopFilePath = "/usr/share/applications";
        }


        QApplication::quit();
    }

private:
    QLineEdit *m_pathEdit0;
    QLineEdit *m_pathEdit1;
    QLineEdit *m_pathEdit2;
    QPushButton *m_createButton;
    QCheckBox *m_systemwideCheckBox;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PathSelectorWidget widget;
    widget.show();
    return app.exec();
}


// Important: Because we use Q_OBJECT, you MUST have CMAKE_AUTOMOC ON in your CMakeLists.txt
// (which we already set in the previous guide).
#include "main.moc"