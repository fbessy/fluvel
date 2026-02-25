#pragma once

#include <QIcon>
#include <QPushButton>
#include <QWidget>

namespace ofeli_app
{

class RightPanelToggleButton : public QPushButton
{
    Q_OBJECT

public:
    explicit RightPanelToggleButton(QWidget* parent = nullptr);

private slots:
    void updateAppearance(bool checked);

private:
    QIcon iconOn_;
    QIcon iconOff_;
};

} // namespace ofeli_app
