#include "launcher.h"
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>

class LauncherSerializationTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void bundledTemplate_data();
    void bundledTemplate();
    void templateRendering_data();
    void templateRendering();
    void desktopValidation_data();
    void desktopValidation();

private:
    QString pattern;
};

void LauncherSerializationTests::initTestCase() {
    QFile source(":/Template.desktop");
    QVERIFY2(source.open(QIODevice::ReadOnly), qPrintable(source.errorString()));
    pattern = QString::fromUtf8(source.readAll());
    QCOMPARE(source.error(), QFileDevice::NoError);
    QVERIFY(!pattern.isEmpty());
}

void LauncherSerializationTests::bundledTemplate_data() {
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("executable");
    QTest::addColumn<QString>("icon");
    QTest::addColumn<QString>("expected");
    struct Case { const char *label; QString name, executable, icon, expected; };
    const Case cases[] = {
        {"spaces", "App", "/tmp/my app", "/tmp/icon.png",
         "Name=App\nExec=\"/tmp/my\\sapp\"\nIcon=/tmp/icon.png\n"},
        {"literal-placeholders", "_execPath_", "/tmp/_iconPath_", "/tmp/_name_",
         "Name=_execPath_\nExec=\"/tmp/_iconPath_\"\nIcon=/tmp/_name_\n"},
        {"escaping", " App\\name ", "/tmp/a\"b$c`d\\e%f", "/tmp/icon\nnext\tline\r.png",
         "Name=\\sApp\\\\name\\s\nExec=\"/tmp/a\\\\\"b\\\\$c\\\\`d\\\\\\\\e%%f\"\nIcon=/tmp/icon\\nnext\\tline\\r.png\n"},
        {"unicode-and-shell-punctuation", QString::fromUtf8("日本語"), "/tmp/a'b;(c)&d", "/tmp/a b.png",
         QString::fromUtf8("Name=日本語\nExec=\"/tmp/a'b;(c)&d\"\nIcon=/tmp/a\\sb.png\n")}
    };
    for (const auto &test : cases) {
        QTest::newRow(test.label) << test.name << test.executable << test.icon
            << QString("[Desktop Entry]\n" + test.expected + "Terminal=false\nType=Application\n");
    }
    QTest::newRow("no-icon") << QString("App") << QString("/tmp/app") << QString()
        << QString("[Desktop Entry]\nName=App\nExec=\"/tmp/app\"\nTerminal=false\nType=Application\n");
}

void LauncherSerializationTests::bundledTemplate() {
    QFETCH(QString, name);
    QFETCH(QString, executable);
    QFETCH(QString, icon);
    QFETCH(QString, expected);
    QCOMPARE(renderLauncher(pattern, name, executable, icon), expected);
}

void LauncherSerializationTests::templateRendering_data() {
    QTest::addColumn<QString>("source");
    QTest::addColumn<QString>("icon");
    QTest::addColumn<QString>("expected");
    QTest::newRow("empty-template") << QString() << QString() << QString();
    QTest::newRow("unchanged-text") << QString("# _unknown_\nType=Application\n")
        << QString() << QString("# _unknown_\nType=Application\n");
    QTest::newRow("repeated-adjacent-placeholders")
        << QString("_name__name_|_execPath_|_iconPath_|tail") << QString("_name_")
        << QString("_execPath__execPath_|\"/tmp/_iconPath_\"|_name_|tail");
    QTest::newRow("omit-icon-crlf") << QString("Name=_name_\r\nIcon=_iconPath_\r\nType=Application\r\n")
        << QString() << QString("Name=_execPath_\r\nType=Application\r\n");
    QTest::newRow("omit-icon-at-eof") << QString("Name=_name_\nIcon=_iconPath_")
        << QString() << QString("Name=_execPath_\n");
    QTest::newRow("omit-icon-only-line") << QString("Icon=_iconPath_\n")
        << QString() << QString();
    QTest::newRow("preserve-other-icon-text")
        << QString("# Icon=_iconPath_\nIcon=default\nComment=_iconPath_\n") << QString()
        << QString("# Icon=\nIcon=default\nComment=\n");
}

void LauncherSerializationTests::templateRendering() {
    QFETCH(QString, source);
    QFETCH(QString, icon);
    QFETCH(QString, expected);
    QCOMPARE(renderLauncher(source, "_execPath_", "/tmp/_iconPath_", icon), expected);
}

void LauncherSerializationTests::desktopValidation_data() {
    bundledTemplate_data();
}

void LauncherSerializationTests::desktopValidation() {
    const QString validator = QStandardPaths::findExecutable("desktop-file-validate");
    if (validator.isEmpty())
        QSKIP("desktop-file-validate is not installed; serialization checks still run.");

    QFETCH(QString, name);
    QFETCH(QString, executable);
    QFETCH(QString, icon);
    QTemporaryDir directory;
    QVERIFY2(directory.isValid(), qPrintable(directory.errorString()));
    QFile output(directory.filePath("test.desktop"));
    QVERIFY2(output.open(QIODevice::WriteOnly), qPrintable(output.errorString()));
    const auto bytes = renderLauncher(pattern, name, executable, icon).toUtf8();
    QCOMPARE(output.write(bytes), qint64(bytes.size()));
    QVERIFY2(output.flush(), qPrintable(output.errorString()));
    output.close();

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(validator, {output.fileName()});
    QVERIFY2(process.waitForStarted(5000), qPrintable(process.errorString()));
    const bool finished = process.waitForFinished(10000);
    if (!finished) {
        process.kill();
        process.waitForFinished(5000);
    }
    const QByteArray diagnostics = process.readAll();
    QVERIFY2(finished, "desktop-file-validate timed out");
    QVERIFY2(process.exitStatus() == QProcess::NormalExit, diagnostics.constData());
    QVERIFY2(process.exitCode() == 0, diagnostics.constData());
}

QTEST_GUILESS_MAIN(LauncherSerializationTests)
#include "launcher_serialization_tests.moc"
