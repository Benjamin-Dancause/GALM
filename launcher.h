#pragma once

#include <QString>

// Render only placeholders in the original template; inserted values are never rescanned.
QString renderLauncher(const QString &source, const QString &name,
                       const QString &executable, const QString &icon);
