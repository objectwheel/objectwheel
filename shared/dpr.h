#ifndef DPR_H
#define DPR_H

#include <QScreen>
#include <QGuiApplication>

#define DPR QGuiApplication::primaryScreen()->devicePixelRatio()

#endif // DPR_H
