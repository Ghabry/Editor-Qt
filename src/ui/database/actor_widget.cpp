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

#include "actor_widget.h"
#include "ui_actor_widget.h"
#include <QTimer>
#include <QGraphicsOpacityEffect>
#include <QSortFilterProxyModel>

#include "common/dbstring.h"
#include "common/lcf_widget_binding.h"
#include "model/actor.h"

ActorWidget::ActorWidget(lcf::rpg::Database &database, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ActorWidget),
	m_data(database)
{
	const auto kMaxLevel = database.system.ldb_id == 2003 ? 99 : 50;
	const auto kMaxHp = database.system.ldb_id == 2003 ? 9999 : 999;
	ui->setupUi(this);

	m_currentActor = nullptr;

	ui->spinMaxLv->setMaximum(kMaxLevel);

	ui->graphicsBattleset->setScene(new QGraphicsScene(this));
	m_charaItem = new CharSetItem();
	m_charaItem->setSpin(true);
	m_charaItem->setWalk(true);
	m_charaItem->setScale(2.0);
	m_charaItem->setGraphicsEffect(new QGraphicsOpacityEffect(this));

	m_faceItem = new FaceSetItem();
	m_faceItem->setScale(2.0);

	m_battlerItem = new BattleAnimationItem();

	for (int i = 0; i < kMaxLevel; i++)
		m_dummyCurve.push_back(0);
	m_hpItem = new CurveItem(Qt::red, m_dummyCurve);
	m_hpItem->setMaxValue(kMaxHp);
	m_mpItem = new CurveItem(Qt::magenta, m_dummyCurve);
	m_attItem = new CurveItem(Qt::yellow, m_dummyCurve);
	m_defItem = new CurveItem(Qt::green, m_dummyCurve);
	m_intItem = new CurveItem(Qt::darkBlue, m_dummyCurve);
	m_agyItem = new CurveItem(Qt::blue, m_dummyCurve);

	ui->graphicsCharset->setScene(new QGraphicsScene(this));
	ui->graphicsCharset->scene()->addItem(m_charaItem);
	ui->graphicsCharset->scene()->setSceneRect(0,0,48,64);

	ui->graphicsFaceset->setScene(new QGraphicsScene(this));
	ui->graphicsFaceset->scene()->addItem(m_faceItem);
	ui->graphicsFaceset->scene()->setSceneRect(0,0,96,96);

	ui->graphicsBattleset->setScene(new QGraphicsScene(this));
	ui->graphicsBattleset->scene()->addItem(m_battlerItem);
	ui->graphicsBattleset->scene()->setSceneRect(0,0,48,48);

	ui->graphicsHp->setScene(new QGraphicsScene(this));
	ui->graphicsHp->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsHp->size()));
	ui->graphicsHp->scene()->addItem(m_hpItem);

	ui->graphicsMp->setScene(new QGraphicsScene(this));
	ui->graphicsMp->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsMp->size()));
	ui->graphicsMp->scene()->addItem(m_mpItem);

	ui->graphicsAtt->setScene(new QGraphicsScene(this));
	ui->graphicsAtt->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsAtt->size()));
	ui->graphicsAtt->scene()->addItem(m_attItem);

	ui->graphicsDef->setScene(new QGraphicsScene(this));
	ui->graphicsDef->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsDef->size()));
	ui->graphicsDef->scene()->addItem(m_defItem);

	ui->graphicsInt->setScene(new QGraphicsScene(this));
	ui->graphicsInt->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsInt->size()));
	ui->graphicsInt->scene()->addItem(m_intItem);

	ui->graphicsAgy->setScene(new QGraphicsScene(this));
	ui->graphicsAgy->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsAgy->size()));
	ui->graphicsAgy->scene()->addItem(m_agyItem);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), ui->graphicsCharset->scene(), SLOT(advance()));
	connect(timer, SIGNAL(timeout()), ui->graphicsBattleset->scene(), SLOT(advance()));
	timer->start(200);
	UpdateModels();

	if (database.system.ldb_id != 2003) {
		for (int i = 0; i < ui->gridBattleSet->count(); ++i) {
			ui->gridBattleSet->itemAt(i)->widget()->hide();
		}
		ui->groupBoxClass->hide();
	}

	for (auto& uis : { ui->lineName, ui->lineTitle }) {
		LcfWidgetBinding::connect(this, uis);
	}

	for (auto& uis : {
			ui->checkAI,
			ui->checkDualWeapon,
			ui->checkFixedEquip,
			ui->checkStrongDefense,
			ui->checkTranslucent }) {
		LcfWidgetBinding::connect(this, uis);
	}

	for (auto& uis : {
			ui->spinMinLv,
			ui->spinMaxLv,
			ui->spinCritChance }) {
		LcfWidgetBinding::connect<int32_t>(this, uis);
	}

	for (auto& uis : {
			ui->comboInitialWeapon,
			ui->comboInitialShield,
			ui->comboInitialArmor,
			ui->comboInitialHelmet,
			ui->comboInitialMisc }) {
		LcfWidgetBinding::connect<int16_t>(this, uis->comboBox(), true);
		uis->makeModel(database, database.items);
	}

	LcfWidgetBinding::connect(this, ui->groupCritChance);

	LcfWidgetBinding::connect<int32_t>(this, ui->comboUnarmedAnimation->comboBox());
	ui->comboUnarmedAnimation->makeModel(database, database.animations);
}

void ActorWidget::setData(lcf::rpg::Actor* actor) {
	on_currentActorChanged(actor);
}

ActorWidget::~ActorWidget()
{
	delete ui;
}

void ActorWidget::UpdateModels()
{
	/* Clear */
	ui->comboBattleset->clear();
	ui->listAttributeRanks->clear();
	ui->listStatusRanks->clear();
	/* Fill */
	ui->comboBattleset->addItem(tr("<none>"));
	for (unsigned int i = 0; i < m_data.battleranimations.size(); i++)
		ui->comboBattleset->addItem(m_data.battleranimations[i].name.c_str());
	ui->comboProfession->addItem(tr("<none>"));
	for (unsigned int i = 0; i < m_data.classes.size(); i++)
		ui->comboProfession->addItem(m_data.classes[i].name.c_str());
	for (unsigned int i = 0; i < m_data.attributes.size(); i++)
		ui->listAttributeRanks->addItem(m_data.attributes[i].name.c_str());
	for (unsigned int i = 0; i < m_data.states.size(); i++)
		ui->listStatusRanks->addItem(m_data.states[i].name.c_str());

	on_currentActorChanged(m_currentActor);
}

void ActorWidget::on_comboBattleset_currentIndexChanged(int index)
{
	if (!m_currentActor)
		return;

	m_currentActor->battler_animation = index;

	if (index <= 0 || index >= static_cast<int>(m_data.battleranimations.size()))
		m_battlerItem->setBasePix(BattleAnimationItem::Battler,"");
	else
		m_battlerItem->setDemoAnimation(m_data.battleranimations[static_cast<size_t>(index) - 1]);
}

void ActorWidget::on_pushSetCharset_clicked()
{
	if (!m_currentActor)
		return;

	CharSetPickerDialog dlg(this, false);
	dlg.setName(ToQString(m_currentActor->character_name));
	dlg.setIndex(m_currentActor->character_index);
	dlg.exec();
	if (dlg.result() == QDialogButtonBox::Ok)
	{
		m_currentActor->character_name = ToDBString(dlg.name());
		m_currentActor->character_index = dlg.index();

		m_charaItem->setVisible(true);
		m_charaItem->setBasePix(ToQString(m_currentActor->character_name));
		m_charaItem->setIndex(m_currentActor->character_index);
	}
}

void ActorWidget::on_pushSetFace_clicked()
{
	faceset_picker_dialog dlg(this, true);
	dlg.setName(ToQString(m_currentActor->face_name));
	dlg.exec();
	if (dlg.result() == QDialogButtonBox::Ok)
	{
		m_currentActor->face_name = ToDBString(dlg.name());
		m_currentActor->face_index = dlg.index();

		m_faceItem->setVisible(true);
		m_faceItem->setBasePix(ToQString(m_currentActor->face_name));
		m_faceItem->setIndex(m_currentActor->face_index);

	}
}

void ActorWidget::ResetExpText(lcf::rpg::Actor* actor) {
	int base = 0;
	int inflation = 0;
	int correction = 0;
	if (actor != nullptr) {
		base = actor->exp_base;
		inflation = actor->exp_inflation;
		correction = actor->exp_correction;
	}
	char buf[1024] = {};
	snprintf(buf, sizeof(buf), "Initial = %d; Increment = %d; Correction = %d",
			 base, inflation, correction);
	ui->labelExpCurve->setText(buf);
}

void ActorWidget::on_currentActorChanged(lcf::rpg::Actor *actor)
{
	if (!actor) {
		actor = &dummy;
	}
	m_currentActor = actor;

	ResetExpText(actor);

	LcfWidgetBinding::bind(ui->lineName, actor->name);
	LcfWidgetBinding::bind(ui->lineTitle, actor->title);
	LcfWidgetBinding::bind(ui->checkAI, actor->auto_battle);
	LcfWidgetBinding::bind(ui->checkDualWeapon, actor->two_weapon);
	LcfWidgetBinding::bind(ui->checkFixedEquip, actor->lock_equipment);
	LcfWidgetBinding::bind(ui->checkStrongDefense, actor->super_guard);
	LcfWidgetBinding::bind(ui->checkTranslucent, actor->transparent);
	LcfWidgetBinding::bind(ui->spinMinLv, actor->initial_level);
	LcfWidgetBinding::bind(ui->spinMaxLv, actor->final_level);
	LcfWidgetBinding::bind(ui->spinCritChance, actor->critical_hit_chance);
	LcfWidgetBinding::bind(ui->groupCritChance, actor->critical_hit);
	LcfWidgetBinding::bind(ui->comboUnarmedAnimation->comboBox(), actor->unarmed_animation);
	LcfWidgetBinding::bind(ui->comboInitialWeapon->comboBox(), actor->initial_equipment.weapon_id, true);
	LcfWidgetBinding::bind(ui->comboInitialShield->comboBox(), actor->initial_equipment.shield_id, true);
	LcfWidgetBinding::bind(ui->comboInitialHelmet->comboBox(), actor->initial_equipment.helmet_id, true);
	LcfWidgetBinding::bind(ui->comboInitialArmor->comboBox(), actor->initial_equipment.armor_id, true);
	LcfWidgetBinding::bind(ui->comboInitialMisc->comboBox(), actor->initial_equipment.accessory_id, true);

	std::array<std::tuple<QComboBox*, lcf::rpg::Item::Type>, 5> vals {{
		{ ui->comboInitialWeapon->comboBox(), lcf::rpg::Item::Type_weapon },
		{ ui->comboInitialShield->comboBox(), lcf::rpg::Item::Type_shield },
		{ ui->comboInitialArmor->comboBox(), lcf::rpg::Item::Type_armor },
		{ ui->comboInitialHelmet->comboBox(), lcf::rpg::Item::Type_helmet },
		{ ui->comboInitialMisc->comboBox(), lcf::rpg::Item::Type_accessory }
	}};

	for (auto& [uis, type] : vals)  {
		// Prevent event calling on filtered comboboxes before the filter is correctly configured
		SignalBlocker s(uis);

		auto filter = Actor(*m_currentActor, *core().project()).CreateEquipmentFilter(type);
		filter->setParent(this);
		filter->setSourceModel(uis->model());
		uis->setModel(filter);
		filter->invalidate();
	}

	ui->comboBattleset->setCurrentIndex(actor->battler_animation);

	ui->comboProfession->setCurrentIndex(actor->class_id);
	ui->tableSkills->setRowCount(0);
	for (int i = 0; i < static_cast<int>(actor->skills.size()); i++){
		ui->tableSkills->insertRow(i);
		ui->tableSkills->setItem(i,0,new QTableWidgetItem());
		ui->tableSkills->setItem(i,1,new QTableWidgetItem());
		ui->tableSkills->item(i, 0)->setText(QString::number(actor->skills[static_cast<size_t>(i)].level));
		// TODO: move getSkillName to Core
		QString name = QString("<%1?>").arg(actor->skills[static_cast<size_t>(i)].skill_id);
		if (actor->skills[static_cast<size_t>(i)].skill_id < static_cast<int>(m_data.skills.size()))
			name = m_data.skills[static_cast<size_t>(actor->skills[static_cast<size_t>(i)].skill_id)-1].name.c_str();
		// TODO/
		ui->tableSkills->item(i, 1)->setText(name);
	}
	ui->tableSkills->insertRow(static_cast<int>(actor->skills.size()));
	for (int i = 0; i < ui->listAttributeRanks->count(); i++)
	{
		if (i >= static_cast<int>(actor->attribute_ranks.size()))
			ui->listAttributeRanks->item(static_cast<int>(i))->setIcon(QIcon(":/embedded/share/old_rank2.png"));
		else
			ui->listAttributeRanks->item(static_cast<int>(i))->setIcon(QIcon(QString(":/embedded/share/old_rank%1.png").arg(static_cast<int>(actor->attribute_ranks[static_cast<size_t>(i)]))));
	}
	for (int i = 0; i < ui->listStatusRanks->count(); i++)
	{
		if (i >= static_cast<int>(actor->state_ranks.size()))
			ui->listStatusRanks->item(i)->setIcon(QIcon(":/embedded/share/old_rank2.png"));
		else
			ui->listStatusRanks->item(i)->setIcon(QIcon(QString(":/embedded/share/old_rank%1.png").arg(static_cast<int>(actor->state_ranks[static_cast<size_t>(i)]))));
	}
	m_charaItem->setVisible(true);
	m_charaItem->setBasePix(actor->character_name.c_str());
	m_charaItem->setIndex(actor->character_index);
	m_charaItem->graphicsEffect()->setEnabled(actor->transparent);

	m_faceItem->setVisible(true);
	m_faceItem->setBasePix(actor->face_name.c_str());
	m_faceItem->setIndex(actor->face_index);

	if (actor->battler_animation <= 0 ||
			actor->battler_animation >= static_cast<int>(m_data.battleranimations.size()))
		m_battlerItem->setBasePix(BattleAnimationItem::Battler,"");
	else
		m_battlerItem->setDemoAnimation(m_data.battleranimations[static_cast<size_t>(actor->battler_animation)-1]);

	m_hpItem->setData(actor->parameters.maxhp);
	m_mpItem->setData(actor->parameters.maxsp);
	m_attItem->setData(actor->parameters.attack);
	m_defItem->setData(actor->parameters.defense);
	m_intItem->setData(actor->parameters.spirit);
	m_agyItem->setData(actor->parameters.agility);

	this->setEnabled(actor != &dummy);
}

void ActorWidget::on_pushApplyProfession_clicked()
{
	if (!m_currentActor)
		return;

	const lcf::rpg::Class &n_class = m_data.classes[static_cast<size_t>(ui->comboProfession->currentIndex())];
	/* Disconnect widgets */
	lcf::rpg::Actor *actor = m_currentActor;
	m_currentActor = nullptr;
	/* /Disconnect widgets */
	actor->class_id = ui->comboProfession->currentIndex();
	actor->attribute_ranks = n_class.attribute_ranks;
	actor->auto_battle = n_class.auto_battle;
	actor->battle_commands = n_class.battle_commands;
	actor->exp_base = n_class.exp_base;
	actor->exp_correction = n_class.exp_correction;
	actor->exp_inflation = n_class.exp_inflation;
	actor->lock_equipment = n_class.lock_equipment;
	actor->parameters = n_class.parameters;
	actor->skills = n_class.skills;
	actor->state_ranks = n_class.state_ranks;
	actor->super_guard = n_class.super_guard;
	actor->two_weapon = n_class.two_weapon;
	actor->battler_animation = n_class.battler_animation;
	/* /TODO */

	/* ConnectWidgets */
	on_currentActorChanged(actor);
}

void ActorWidget::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event)
	ui->graphicsHp->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsHp->size()));
	ui->graphicsMp->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsMp->size()));
	ui->graphicsAtt->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsAtt->size()));
	ui->graphicsDef->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsDef->size()));
	ui->graphicsInt->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsInt->size()));
	ui->graphicsAgy->scene()->setSceneRect(QRectF(QPointF(0,0),ui->graphicsAgy->size()));
}
