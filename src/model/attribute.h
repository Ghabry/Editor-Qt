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

#include <lcf/rpg/database.h>
#include <lcf/rpg/attribute.h>
#include "rpg_base.h"

/**
 * A thin wrapper around lcf::rpg::Attribute
 */
class Attribute : public RpgBase
{
public:
	Attribute(lcf::rpg::Attribute& data, lcf::rpg::Database& database);

	lcf::rpg::Attribute& data();

	QPixmap preview() override;

	QDialog* edit(QWidget* parent = nullptr) override;

private:
	lcf::rpg::Attribute& m_data;
	lcf::rpg::Database& database;
};

