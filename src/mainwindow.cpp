#include "dialognewproject.h"
#include "dialogopenproject.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QToolBar>
#include <QApplication>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include "EasyRPGCore.h"
#include "lmu_reader.h"
#include "lmt_reader.h"
#include "ldb_reader.h"
#include "inireader.h"
#include "rpg_map.h"
#include "data.h"

static void associateFileTypes(const QStringList &fileTypes)
{
    QString displayName = QGuiApplication::applicationDisplayName();
    QString filePath = QCoreApplication::applicationFilePath();
    QString fileName = QFileInfo(filePath).fileName();

    QSettings settings("HKEY_CURRENT_USER\\Software\\Classes\\Applications\\" + fileName, QSettings::NativeFormat);
    settings.setValue("FriendlyAppName", displayName);

    settings.beginGroup("SupportedTypes");
    foreach (const QString& fileType, fileTypes)
        settings.setValue(fileType, QString());
    settings.endGroup();

    settings.beginGroup("shell");
    settings.beginGroup("open");
    settings.setValue("FriendlyAppName", displayName);
    settings.beginGroup("Command");
    settings.setValue(".", QChar('"') + QDir::toNativeSeparators(filePath) + QString("\" \"%1\""));
}
//! [0]




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    const QString DEFAULT_DIR_KEY("default_dir");
    const QString CURRENT_PROJECT_KEY("current_project");
    EasyRPGCore::Init();
    ui->setupUi(this);
    // Hide map ids
    ui->treeMap->hideColumn(0);
    // Created hardcoded toolbar for palette window.
    ui->toolBar->setParent(ui->dockWidgetContents);
    //Create dialogs
    dlg_resource = new DialogResourceManager(this);
    dlg_resource->setModal(true);
    dlg_db = new DialogDataBase(this);
    dlg_db->setModal(true);
    update_actions();
    m_defDir = m_settings.value(DEFAULT_DIR_KEY, qApp->applicationDirPath()).toString();
    QString l_project = m_settings.value(CURRENT_PROJECT_KEY, QString()).toString();
    QFileInfo info(m_defDir+l_project+"/project.erp");
    if (l_project != QString() && info.exists())
        LoadProject(m_defDir+l_project+"/");
    m_paleteScene = new QGraphicsPaleteScene(ui->graphicsPalete);
    ui->graphicsPalete->setScene(m_paleteScene);
    /**  Test  **/
    EasyRPGCore::LoadChipset("C:/Program Files (x86)/ASCII/RPG2000/RTP/ChipSet/Basis.png");
    m_paleteScene->onChipsetChange();
    m_paleteScene->onLayerChange();
    QGraphicsScene *m_mapScene = new QGraphicsScene();
    ui->graphicsView->setScene(m_mapScene);
    m_mapWidget = new QGraphicsMapWidget();
    m_mapScene->addItem(m_mapWidget);
    /** /Test  **/
//    if (project())
//        m_paleteScene->onLayerChange();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete dlg_resource;
    delete dlg_db;
    delete m_paleteScene;
    delete m_mapWidget;
}

void MainWindow::LoadProject(QString p_path)
{
    const QString CURRENT_PROJECT_KEY("current_project");
    EasyRPGCore::setCurrentProjectPath(p_path);
    Data::Clear();
    if (!LDB_Reader::Load(QString(p_path+"rpg_rt.ldb").toStdString()))
    {
        QMessageBox::critical(this,
                              "Error loading project",
                              "Could not load database file: "+p_path+"RPG_RT.ldb");
        EasyRPGCore::setCurrentProjectPath("");
        return;
    }
    if (!LMT_Reader::Load(QString(p_path+"RPG_RT.lmt").toStdString()))
    {
        QMessageBox::critical(this,
                              "Error loading project",
                              "Could not load map tree file: "+p_path+"RPG_RT.lmt");
        EasyRPGCore::setCurrentProjectPath("");
        return;
    }

    INIReader reader(p_path.toStdString()+"RPG_RT.ini");
    QString title (reader.Get("RPG_RT","GameTitle", "Untitled").c_str());
    EasyRPGCore::setCurrentGameTitle(title);
    switch (reader.GetInteger("RPG_RT","MapEditMode", 0))
    {
    case 1:
        EasyRPGCore::setCurrentLayer(EasyRPGCore::UPPER);
        break;
    case 2:
        EasyRPGCore::setCurrentLayer(EasyRPGCore::EVENT);
        break;
    default:
        EasyRPGCore::setCurrentLayer(EasyRPGCore::LOWER);
        break;
    }
    //TODO:: create a new variable on the ini for a suitable zoom mode
    float scale = (float)reader.GetInteger("RPG_RT","MapEditZoom", 0)*0.5+0.5;
    m_mapWidget->setScale(scale);
    setWindowTitle("EasyRPG Editor - " + EasyRPGCore::currentGameTitle());
    m_settings.setValue(CURRENT_PROJECT_KEY, EasyRPGCore::currentGameTitle());
    update_actions();
}

void MainWindow::on_action_Quit_triggered()
{
    this->on_actionJukebox_triggered(true);
    Data::Clear();
    qApp->quit();
}

void MainWindow::on_actionPalete_triggered(bool checked)
{
    if (checked)
        ui->dockPalete->show();
    else
        ui->dockPalete->hide();
}

void MainWindow::on_actionMap_Tree_triggered(bool checked)
{
    if (checked)
        ui->dockMapTree->show();
    else
        ui->dockMapTree->hide();
}

void MainWindow::on_actionResource_Manager_triggered()
{
    dlg_resource->show();
}

void MainWindow::on_actionData_Base_triggered()
{
    dlg_db->show();
}

void MainWindow::update_actions()
{
    if (EasyRPGCore::currentProjectPath().isEmpty()){
        ui->actionCircle->setEnabled(false);
        ui->actionCreate_Game_Disk->setEnabled(false);
        ui->actionData_Base->setEnabled(false);
        ui->actionDraw->setEnabled(false);
        ui->actionFill->setEnabled(false);
        ui->actionRectangle->setEnabled(false);
        ui->actionResource_Manager->setEnabled(false);
        ui->actionRevert_all_Maps->setEnabled(false);
        ui->actionScale_1_1->setEnabled(false);
        ui->actionZoomIn->setEnabled(false);
        ui->actionZoomOut->setEnabled(false);
        ui->actionSearch->setEnabled(false);
        ui->actionSelect->setEnabled(false);
        ui->actionUndo->setEnabled(false);
        ui->actionZoom->setEnabled(false);
        ui->action_Close_Project->setEnabled(false);
        ui->action_Events->setEnabled(false);
        ui->action_Full_Screen->setEnabled(false);
        ui->action_Lower_Layer->setEnabled(false);
        ui->action_New_Project->setEnabled(true);
        ui->action_Open_Project->setEnabled(true);
        ui->action_Play_Test->setEnabled(false);
        ui->action_Save_all_Maps->setEnabled(false);
        ui->action_Script_Editor->setEnabled(false);
        ui->action_Title_Background->setEnabled(false);
        ui->action_Upper_Layer->setEnabled(false);
    } else {
        ui->actionCircle->setEnabled(true);
        ui->actionCreate_Game_Disk->setEnabled(true);
        ui->actionData_Base->setEnabled(true);
        ui->actionDraw->setEnabled(true);
        ui->actionFill->setEnabled(true);
        ui->actionRectangle->setEnabled(true);
        ui->actionResource_Manager->setEnabled(true);
        ui->actionRevert_all_Maps->setEnabled(true);
        ui->actionScale_1_1->setEnabled(true);
        ui->actionZoomIn->setEnabled(true);
        ui->actionZoomOut->setEnabled(true);
        ui->actionSearch->setEnabled(true);
        ui->actionSelect->setEnabled(true);
        ui->actionUndo->setEnabled(true);
        ui->actionZoom->setEnabled(true);
        ui->action_Close_Project->setEnabled(true);
        ui->action_Events->setEnabled(true);
        ui->action_Full_Screen->setEnabled(true);
        ui->action_Lower_Layer->setEnabled(true);
        ui->action_New_Project->setEnabled(false);
        ui->action_Open_Project->setEnabled(false);
        ui->action_Play_Test->setEnabled(true);
        ui->action_Save_all_Maps->setEnabled(true);
        ui->action_Script_Editor->setEnabled(true);
        ui->action_Title_Background->setEnabled(true);
        ui->action_Upper_Layer->setEnabled(true);
    }
}

void MainWindow::on_action_New_Project_triggered()
{
    const QString DEFAULT_DIR_KEY("default_dir");
    const QString CURRENT_PROJECT_KEY("current_project");
    DialogNewProject dlg(this);
    dlg.setDefDir(m_defDir);
    dlg.exec();
    if (dlg.result() == QDialog::Accepted){
        if (dlg.getProjectPath() == QString())
            return;
        QDir d_gamepath(dlg.getProjectPath());
            if (d_gamepath.exists())
            {
                int response = QMessageBox::warning(this,
                                "Game folder exist",
                                QString("The folder %1 where you want to place your new game already exist. Do you want to delete this folder and all it's content?").arg(dlg.getProjectPath()),
                                QMessageBox::Ok,
                                QMessageBox::Cancel);
                if (response == QMessageBox::Cancel)
                    return;
                removeDir(dlg.getProjectPath(),dlg.getProjectPath());
            }
            else
                d_gamepath.mkdir(".");
        EasyRPGCore::setCurrentProjectPath(dlg.getProjectPath());
        EasyRPGCore::setCurrentGameTitle(dlg.getGameTitle());
        EasyRPGCore::setTileSize(dlg.getTileSize());
        m_defDir = dlg.getDefDir();
        Data::Clear();
        d_gamepath.mkdir(dlg.getProjectPath()+"Backdrop");
        d_gamepath.mkdir(dlg.getProjectPath()+"Panorama");
        d_gamepath.mkdir(dlg.getProjectPath()+"Battle");
        d_gamepath.mkdir(dlg.getProjectPath()+"Battle2");
        d_gamepath.mkdir(dlg.getProjectPath()+"BattleCharSet");
        d_gamepath.mkdir(dlg.getProjectPath()+"BattleWeapon");
        d_gamepath.mkdir(dlg.getProjectPath()+"CharSet");
        d_gamepath.mkdir(dlg.getProjectPath()+"ChipSet");
        d_gamepath.mkdir(dlg.getProjectPath()+"FaceSet");
        d_gamepath.mkdir(dlg.getProjectPath()+"Frame");
        d_gamepath.mkdir(dlg.getProjectPath()+"GameOver");
        d_gamepath.mkdir(dlg.getProjectPath()+"Monster");
        d_gamepath.mkdir(dlg.getProjectPath()+"Movie");
        d_gamepath.mkdir(dlg.getProjectPath()+"Music");
        d_gamepath.mkdir(dlg.getProjectPath()+"Picture");
        d_gamepath.mkdir(dlg.getProjectPath()+"Sound");
        d_gamepath.mkdir(dlg.getProjectPath()+"System");
        d_gamepath.mkdir(dlg.getProjectPath()+"System2");
        d_gamepath.mkdir(dlg.getProjectPath()+"Title");
        m_settings.setValue(DEFAULT_DIR_KEY,dlg.getDefDir());
        setWindowTitle("EasyRPG Editor - " + EasyRPGCore::currentGameTitle());
        m_settings.setValue(CURRENT_PROJECT_KEY, EasyRPGCore::currentGameTitle());
        //TODO:: add a map;
        LDB_Reader::Save(EasyRPGCore::currentProjectPath().toStdString()+"RPG_RT.ldb");
        LMT_Reader::Save(EasyRPGCore::currentProjectPath().toStdString()+"RPG_RT.lmt");
        //TODO:: create ini;
        update_actions();
    }
}

bool MainWindow::removeDir(const QString & dirName, const QString &root)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst))
        {
            if (info.isDir())
                result = removeDir(info.absoluteFilePath(),root);
            else
                result = QFile::remove(info.absoluteFilePath());

            if (!result)
            {
                QMessageBox::warning(this,
                                     "An error ocurred",
                                     QString("Could't delete %1").arg(info.absoluteFilePath()),
                                     QMessageBox::Ok, 0);
                return false;
            }
        }
        if (root != dirName)
            result = dir.rmdir(dirName);
    }
    return result;
}

void MainWindow::on_action_Close_Project_triggered()
{
    const QString CURRENT_PROJECT_KEY("current_project");
    m_settings.setValue(CURRENT_PROJECT_KEY, QString());
    Data::Clear();
    EasyRPGCore::setCurrentGameTitle("");
    EasyRPGCore::setCurrentProjectPath("");
    update_actions();
    setWindowTitle("EasyRPG Editor");
}

void MainWindow::on_action_Open_Project_triggered()
{
    static DialogOpenProject dlg(this);
    dlg.setDefDir(m_defDir);
    if (dlg.exec() == QDialog::Accepted)
        LoadProject(dlg.getProjectPath());
    m_defDir = dlg.getDefDir();
}

void MainWindow::on_actionJukebox_triggered(bool disconnect)
{
      static MusicPlayer player;
    if (disconnect)
    {
        player.disconnect();
       player.deleteLater();
       player.close();
    }
    else
    {
        associateFileTypes(QStringList(".wav,.mp3,.midi"));
        if(!player.isHidden())
        {
            player.playFile("C:\\Users\\Public\\Music\\Sample Music\\Kalimba.mp3");
        }
        player.resize(300, 60);
        player.show();
    }
}

void MainWindow::on_actionChipset_triggered()
{
    if (!EasyRPGCore::debugChipset())
        return;
    EasyRPGCore::debugChipset()->show();
}

void MainWindow::on_action_Lower_Layer_triggered()
{
    ui->action_Lower_Layer->setChecked(true);
    ui->action_Upper_Layer->setChecked(false);
    ui->action_Events->setChecked(false);
    EasyRPGCore::setCurrentLayer(EasyRPGCore::LOWER);
    m_paleteScene->onLayerChange();
}

void MainWindow::on_action_Upper_Layer_triggered()
{
    ui->action_Lower_Layer->setChecked(false);
    ui->action_Upper_Layer->setChecked(true);
    ui->action_Events->setChecked(false);
    EasyRPGCore::setCurrentLayer(EasyRPGCore::UPPER);
    m_paleteScene->onLayerChange();
}

void MainWindow::on_action_Events_triggered()
{
    ui->action_Lower_Layer->setChecked(false);
    ui->action_Upper_Layer->setChecked(false);
    ui->action_Events->setChecked(true);
    EasyRPGCore::setCurrentLayer(EasyRPGCore::EVENT);
    m_paleteScene->onLayerChange();
}

void MainWindow::on_actionOpen_LMU_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open RPG Maker map",
                                                    QString(),
                                                    "LMU file (*.lmu)");
    if (fileName.isEmpty())
        return;
    EasyRPGCore::setCurrentMap(LMU_Reader::Load(fileName.toStdString()).get());
    m_mapWidget->onMapChange();
}

void MainWindow::on_actionZoomIn_triggered()
{
    if (m_mapWidget->scale() != 2.0)
        m_mapWidget->setScale(m_mapWidget->scale()+0.5);
}

void MainWindow::on_actionZoomOut_triggered()
{
    if (m_mapWidget->scale() != 0.5)
        m_mapWidget->setScale(m_mapWidget->scale()-0.5);
}

void MainWindow::on_actionScale_1_1_triggered()
{
    m_mapWidget->setScale(1.0);
}
