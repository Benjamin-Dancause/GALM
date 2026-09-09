#include "launcher.h"
#include <QRegularExpression>

namespace {
QString escapeValue(const QString &value) {
    QString result;
    for (QChar c : value) {
        switch (c.unicode()) {
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        case ' ': result += "\\s"; break;
        default: result += c;
        }
    }
    return result;
}

QString escapeExecutable(const QString &value) {
    QString quoted = "\"";
    for (QChar c : value) {
        if (c == '%') {
            quoted += "%%";
        } else {
            if (c == '"' || c == '`' || c == '$' || c == '\\')
                quoted += '\\';
            quoted += c;
        }
    }
    quoted += '"';
    // Desktop-entry string decoding happens before Exec command-line decoding.
    return escapeValue(quoted);
}
}

QString renderLauncher(const QString &source, const QString &name,
                       const QString &executable, const QString &icon) {
    QString templateText = source;
    if (icon.isEmpty()) {
        templateText.remove(QRegularExpression("^Icon=_iconPath_(?:\\r?\\n|$)",
                                              QRegularExpression::MultilineOption));
    }
    const QRegularExpression placeholders("_name_|_execPath_|_iconPath_");
    auto matches = placeholders.globalMatch(templateText);
    QString result;
    qsizetype offset = 0;
    while (matches.hasNext()) {
        const auto match = matches.next();
        result += templateText.mid(offset, match.capturedStart() - offset);
        const QString token = match.captured();
        result += token == "_name_" ? escapeValue(name)
            : token == "_execPath_" ? escapeExecutable(executable) : escapeValue(icon);
        offset = match.capturedEnd();
    }
    return result + templateText.mid(offset);
}
