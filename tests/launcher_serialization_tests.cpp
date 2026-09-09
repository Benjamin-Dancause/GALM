#include "launcher.h"
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <iostream>

int main() {
    QFile source(":/Template.desktop");
    if (!source.open(QIODevice::ReadOnly)) return 1;
    const QString pattern = QString::fromUtf8(source.readAll());
    struct Case { QString name, executable, icon, expected; };
    const Case cases[] = {
        {"App", "/tmp/my app", "/tmp/icon.png",
         "Name=App\nExec=\"/tmp/my\\sapp\"\nIcon=/tmp/icon.png\n"},
        {"_execPath_", "/tmp/_iconPath_", "/tmp/_name_",
         "Name=_execPath_\nExec=\"/tmp/_iconPath_\"\nIcon=/tmp/_name_\n"},
        {" App\\name ", "/tmp/a\"b$c`d\\e%f", "/tmp/icon\nnext\tline\r.png",
         "Name=\\sApp\\\\name\\s\nExec=\"/tmp/a\\\\\"b\\\\$c\\\\`d\\\\\\\\e%%f\"\nIcon=/tmp/icon\\nnext\\tline\\r.png\n"},
        {QString::fromUtf8("日本語"), "/tmp/a'b;(c)&d", "/tmp/a b.png",
         QString::fromUtf8("Name=日本語\nExec=\"/tmp/a'b;(c)&d\"\nIcon=/tmp/a\\sb.png\n")}
    };
    QTemporaryDir directory;
    if (!directory.isValid()) return 1;
    const QString validator = QStandardPaths::findExecutable("desktop-file-validate");
    for (const auto &test : cases) {
        const QString rendered = renderLauncher(pattern, test.name, test.executable, test.icon);
        const QString expected = "[Desktop Entry]\n" + test.expected + "Terminal=false\nType=Application\n";
        if (rendered != expected) {
            std::cerr << "Unexpected serialization:\n" << rendered.toStdString();
            return 1;
        }
        if (!validator.isEmpty()) {
            QFile output(directory.filePath("test.desktop"));
            if (!output.open(QIODevice::WriteOnly)) return 1;
            const auto bytes = rendered.toUtf8();
            if (output.write(bytes) != bytes.size()) return 1;
            output.close();
            if (QProcess::execute(validator, {output.fileName()}) != 0) return 1;
        }
    }
    return 0;
}
