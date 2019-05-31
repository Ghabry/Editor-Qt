#ifndef DIALOGRUNGAME_H
#define DIALOGRUNGAME_H

#include <QDialog>
#include <QTableWidgetItem>
#include <rpg_testbattler.h>

namespace Ui {
class RunGameDialog;
}

class RunGameDialog : public QDialog
{
	Q_OBJECT

public:

	explicit RunGameDialog(QWidget *parent = nullptr);
	~RunGameDialog();

	void runHere(int map_id, int x, int y);

	void runBattle(int troop_id);

private slots:
	void on_comboMode_currentIndexChanged(int index);

	void on_tableInitialParty_itemChanged(QTableWidgetItem *item);

private:
	void UpdateModels();

	Ui::RunGameDialog *ui;
	std::vector<RPG::TestBattler> battletest_data;
};

#endif // DIALOGRUNGAME_H
