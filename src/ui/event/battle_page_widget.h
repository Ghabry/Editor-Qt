/*
 * This file is part of EasyRPG Editor.
 *
 * EasyRPG Editor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Editor. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QWidget>
#include <lcf/rpg/trooppage.h>

namespace Ui {
class BattlePageWidget;
}

class BattlePageWidget : public QWidget
{
	Q_OBJECT

public:
	explicit BattlePageWidget(QWidget *parent = nullptr);
	~BattlePageWidget() override;

	lcf::rpg::TroopPage* eventPage() const;
	void setEventPage(lcf::rpg::TroopPage *eventPage);

private:
	Ui::BattlePageWidget* ui;
	lcf::rpg::TroopPage*m_eventPage = nullptr;
};
