/*
 * Copyright (c) 2024 Joachim Kohlhammer (joachim.kohlhammer@gmx.de)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "ActionButtonBox.h"

#include "Colors.h"


ActionButtonBox::ActionButtonBox
(ActionButtonBox::StandardButtonGroups standardButtons, QWidget *parent)
: QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setSpacing(2 * dpiXFactor);
    layout->setContentsMargins(0, 0, 0, 0);
    if (standardButtons.testFlag(StandardButtonGroup::UpDownGroup)) {
#ifdef Q_OS_MAC
        up = new QPushButton(tr("Up"));
        down = new QPushButton(tr("Down"));
#else
        up = new QToolButton(this);
        up->setArrowType(Qt::UpArrow);
        up->setFixedSize(20 * dpiXFactor, 20 * dpiYFactor);
        down = new QToolButton(this);
        down->setArrowType(Qt::DownArrow);
        down->setFixedSize(20 * dpiXFactor, 20 * dpiYFactor);
#endif
        layout->addWidget(up, 0, Qt::AlignHCenter | Qt::AlignTop);
        layout->addWidget(down, 0, Qt::AlignHCenter | Qt::AlignTop);

        layout->addItem(new QSpacerItem(5 * dpiXFactor, 0));
        leftOffset = 3;

        connect(up, &QAbstractButton::clicked, this, &ActionButtonBox::upRequested);
        connect(down, &QAbstractButton::clicked, this, &ActionButtonBox::downRequested);
    }
    layout->addStretch();
    if (standardButtons.testFlag(StandardButtonGroup::EditGroup)) {
        edit = new QPushButton(tr("Edit"));
        int spacerWidth = 0;
        if (standardButtons.testFlag(StandardButtonGroup::AddDeleteGroup)) {
            spacerWidth = 5 * dpiXFactor;
        }
        layout->addItem(new QSpacerItem(spacerWidth, edit->sizeHint().height()));
        layout->addWidget(edit);

        connect(edit, &QPushButton::clicked, this, &ActionButtonBox::editRequested);
        if (standardButtons.testFlag(StandardButtonGroup::AddDeleteGroup)) {
            rightOffset += 2;
        } else {
            layout->addStretch();
        }
    }
    if (standardButtons.testFlag(StandardButtonGroup::AddDeleteGroup)) {
        add = new QPushButton(tr("+"));
        del = new QPushButton(tr("-"));
#ifdef Q_OS_MAC
        add->setText(tr("Add"));
        del->setText(tr("Delete"));
#else
        add->setFixedSize(20 * dpiXFactor, 20 * dpiYFactor);
        del->setFixedSize(20 * dpiXFactor, 20 * dpiYFactor);
#endif
        layout->addItem(new QSpacerItem(5 * dpiXFactor, 0));
        layout->addWidget(add, 0, Qt::AlignHCenter | Qt::AlignTop);
        layout->addWidget(del, 0, Qt::AlignHCenter | Qt::AlignTop);
        rightOffset += 3;

        connect(add, &QPushButton::clicked, this, &ActionButtonBox::addRequested);
        connect(del, &QPushButton::clicked, this, &ActionButtonBox::deleteRequested);
    }
}


QAbstractButton*
ActionButtonBox::which
(ActionButtonBox::StandardButton which) const
{
    switch (which) {
        case Up:
            return up;
        case Down:
            return down;
        case Add:
            return add;
        case Delete:
            return del;
        case Edit:
            return edit;
        default:
            return nullptr;
    }
}


void
ActionButtonBox::setButtonHidden
(ActionButtonBox::StandardButton standardButton, bool hidden)
{
    QAbstractButton *button = which(standardButton);
    if (button != nullptr) {
        button->setHidden(hidden);
    }
}


void
ActionButtonBox::setButtonEnabled
(ActionButtonBox::StandardButton standardButton, bool enabled)
{
    QAbstractButton *button = which(standardButton);
    if (button != nullptr) {
        button->setEnabled(enabled);
    }
}


QPushButton*
ActionButtonBox::addButton
(const QString &text, ActionButtonBox::ActionPosition pos)
{
    QPushButton *ret = new QPushButton(text);
    if (pos == ActionButtonBox::Left) {
        layout->insertWidget(leftOffset++, ret);
        layout->insertItem(leftOffset++, new QSpacerItem(0, ret->sizeHint().height()));
    } else {
        layout->insertWidget(layout->count() - rightOffset++, ret);
        layout->insertItem(layout->count() - rightOffset++, new QSpacerItem(0, ret->sizeHint().height()));
    }
    return ret;
}


void
ActionButtonBox::addWidget
(QWidget *widget)
{
    layout->insertWidget(leftOffset++, widget);
    layout->insertItem(leftOffset++, new QSpacerItem(0, widget->sizeHint().height()));
}


void
ActionButtonBox::addStretch
(ActionPosition pos)
{
    if (pos == ActionButtonBox::Left) {
        layout->insertStretch(leftOffset++);
    } else {
        layout->insertStretch(layout->count() - rightOffset++);
    }
}


void
ActionButtonBox::defaultConnect
(StandardButtonGroup group, QTreeWidget *tree)
{
    defaultConnectInit(group);
    connect(tree, &QTreeWidget::currentItemChanged, [=]() {
            updateButtonState(group, tree);
        });
    // Also listen for rowsRemoved as currentItemChanged fires to early (count of tree not yet reduced)
    connect(tree->model(), &QAbstractItemModel::rowsRemoved, [=]() {
            updateButtonState(group, tree);
        });
}


void
ActionButtonBox::defaultConnect
(StandardButtonGroup group, QListWidget *list)
{
    defaultConnectInit(group);
    connect(list, &QListWidget::currentItemChanged, [=]()
        {
            updateButtonState(group, list);
        }
    );
    // Also listen for rowsRemoved as currentItemChanged fires to early (count of list not yet reduced)
    connect(list->model(), &QAbstractItemModel::rowsRemoved, [=]()
        {
            updateButtonState(group, list);
        }
    );
}


void
ActionButtonBox::updateButtonState
(StandardButtonGroup group, QTreeWidget *tree)
{
    QTreeWidgetItem *item = tree->currentItem();
    switch (group) {
    case UpDownGroup:
        if (up != nullptr && down != nullptr) {
            up->setEnabled(item != nullptr && tree->currentIndex().row() > 0);
            down->setEnabled(item != nullptr && tree->currentIndex().row() < tree->invisibleRootItem()->childCount() - 1);
        }
        break;
    case AddDeleteGroup:
        if (add != nullptr && del != nullptr) {
            add->setEnabled(true);
            del->setEnabled(item != nullptr);
        }
        break;
    case EditGroup:
        if (edit != nullptr) {
            edit->setEnabled(item != nullptr);
        }
        break;
    default:
        break;
    }
}


void
ActionButtonBox::updateButtonState
(StandardButtonGroup group, QListWidget *list)
{
    QListWidgetItem *item = list->currentItem();
    switch (group) {
    case UpDownGroup:
        if (up != nullptr && down != nullptr) {
            up->setEnabled(item != nullptr && list->row(item) > 0);
            down->setEnabled(item != nullptr && list->row(item) < list->count() - 1);
        }
        break;
    case AddDeleteGroup:
        if (add != nullptr && del != nullptr) {
            add->setEnabled(true);
            del->setEnabled(item != nullptr);
        }
        break;
    case EditGroup:
        if (edit != nullptr) {
            edit->setEnabled(item != nullptr);
        }
        break;
    default:
        break;
    }
}


void
ActionButtonBox::defaultConnectInit
(StandardButtonGroup group)
{
    switch (group) {
    case UpDownGroup:
        up->setEnabled(false);
        down->setEnabled(false);
        break;
    case AddDeleteGroup:
        add->setEnabled(true);
        del->setEnabled(false);
        break;
    case EditGroup:
        edit->setEnabled(false);
        break;
    default:
        break;
    }
}