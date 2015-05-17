/*
 * EDA4U - Professional EDA for everyone!
 * Copyright (C) 2013 Urban Bruhin
 * http://eda4u.ubruhin.ch/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*****************************************************************************************
 *  Includes
 ****************************************************************************************/

#include <QtCore>
#include <QtWidgets>
#include "boardeditor.h"
#include "ui_boardeditor.h"
#include "../project.h"
#include "../../workspace/workspace.h"
#include "../../workspace/settings/workspacesettings.h"
#include "../../common/undostack.h"
#include "board.h"
#include "../circuit/circuit.h"
#include "../../common/dialogs/gridsettingsdialog.h"
#include "../dialogs/projectpropertieseditordialog.h"
#include "../settings/projectsettings.h"
#include "../../common/graphics/graphicsview.h"
#include "../../common/gridproperties.h"
#include "cmd/cmdboardadd.h"

namespace project {

/*****************************************************************************************
 *  Constructors / Destructor
 ****************************************************************************************/

BoardEditor::BoardEditor(Project& project, bool readOnly) :
    QMainWindow(0), mProject(project), mUi(new Ui::BoardEditor),
    mGraphicsView(nullptr), mGridProperties(nullptr), mActiveBoardIndex(-1),
    mBoardListActionGroup(this)
{
    mUi->setupUi(this);
    mUi->actionProjectSave->setEnabled(!readOnly);

    // set window title
    QString filenameStr = mProject.getFilepath().getFilename();
    if (readOnly) filenameStr.append(QStringLiteral(" [Read-Only]"));
    setWindowTitle(QString("%1 - Board Editor - EDA4U %2.%3")
                   .arg(filenameStr).arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR));

    // create default grid properties
    mGridProperties = new GridProperties();

    // add graphics view as central widget
    mGraphicsView = new GraphicsView(nullptr, this);
    mGraphicsView->setGridProperties(*mGridProperties);
    //setCentralWidget(mGraphicsView);
    mUi->centralwidget->layout()->addWidget(mGraphicsView);

    // add all boards to the menu and connect to project signals
    for (int i=0; i<mProject.getBoards().count(); i++)
        boardAdded(i);
    connect(&mProject, &Project::boardAdded, this, &BoardEditor::boardAdded);
    connect(&mProject, &Project::boardRemoved, this, &BoardEditor::boardRemoved);
    connect(&mBoardListActionGroup, &QActionGroup::triggered,
            this, &BoardEditor::boardListActionGroupTriggered);

    // connect some actions which are created with the Qt Designer
    connect(mUi->actionProjectSave, &QAction::triggered, &mProject, &Project::saveProject);
    connect(mUi->actionQuit, &QAction::triggered, this, &BoardEditor::close);
    connect(mUi->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(mUi->actionZoomIn, &QAction::triggered, mGraphicsView, &GraphicsView::zoomIn);
    connect(mUi->actionZoomOut, &QAction::triggered, mGraphicsView, &GraphicsView::zoomOut);
    connect(mUi->actionZoomAll, &QAction::triggered, mGraphicsView, &GraphicsView::zoomAll);
    connect(mUi->actionShowControlPanel, &QAction::triggered,
            &Workspace::instance(), &Workspace::showControlPanel);
    connect(mUi->actionShowSchematicEditor, &QAction::triggered,
            &mProject, &Project::showSchematicEditor);
    connect(mUi->actionProjectSettings, &QAction::triggered,
            [this](){mProject.getSettings().showSettingsDialog(this);});

    // connect the undo/redo actions with the UndoStack of the project
    connect(&mProject.getUndoStack(), &UndoStack::undoTextChanged,
            [this](const QString& text){mUi->actionUndo->setText(text);});
    mUi->actionUndo->setText(mProject.getUndoStack().getUndoText());
    connect(&mProject.getUndoStack(), SIGNAL(canUndoChanged(bool)),
            mUi->actionUndo, SLOT(setEnabled(bool)));
    mUi->actionUndo->setEnabled(mProject.getUndoStack().canUndo());
    connect(&mProject.getUndoStack(), &UndoStack::redoTextChanged,
            [this](const QString& text){mUi->actionRedo->setText(text);});
    mUi->actionRedo->setText(mProject.getUndoStack().getRedoText());
    connect(&mProject.getUndoStack(), SIGNAL(canRedoChanged(bool)),
            mUi->actionRedo, SLOT(setEnabled(bool)));
    mUi->actionRedo->setEnabled(mProject.getUndoStack().canRedo());

    // Restore Window Geometry
    QSettings clientSettings;
    restoreGeometry(clientSettings.value("board_editor/window_geometry").toByteArray());
    restoreState(clientSettings.value("board_editor/window_state").toByteArray());

    // Load first board
    if (mProject.getBoards().count() > 0)
        setActiveBoardIndex(0);

    // mUi->graphicsView->zoomAll(); does not work properly here, should be executed later...
    QTimer::singleShot(200, mGraphicsView, &GraphicsView::zoomAll); // ...in the event loop
}

BoardEditor::~BoardEditor()
{
    // Save Window Geometry
    QSettings clientSettings;
    clientSettings.setValue("board_editor/window_geometry", saveGeometry());
    clientSettings.setValue("board_editor/window_state", saveState());

    qDeleteAll(mBoardListActions);  mBoardListActions.clear();

    delete mGraphicsView;           mGraphicsView = nullptr;
    delete mGridProperties;         mGridProperties = nullptr;
    delete mUi;                     mUi = nullptr;
}

/*****************************************************************************************
 *  Getters
 ****************************************************************************************/

Board* BoardEditor::getActiveBoard() const noexcept
{
    return mProject.getBoardByIndex(mActiveBoardIndex);
}

/*****************************************************************************************
 *  Setters
 ****************************************************************************************/

bool BoardEditor::setActiveBoardIndex(int index) noexcept
{
    if (index == mActiveBoardIndex)
        return true;

    Board* board = getActiveBoard();
    if (board)
    {
        // save current view scene rect
        board->saveViewSceneRect(mGraphicsView->getVisibleSceneRect());
        // uncheck QAction
        QAction* action = mBoardListActions.value(mActiveBoardIndex); Q_ASSERT(action);
        if (action) action->setChecked(false);
    }
    board = mProject.getBoardByIndex(index);
    if (board)
    {
        // show scene, restore view scene rect, set grid properties
        board->showInView(*mGraphicsView);
        mGraphicsView->setVisibleSceneRect(board->restoreViewSceneRect());
        mGraphicsView->setGridProperties(board->getGridProperties());
        // check QAction
        QAction* action = mBoardListActions.value(index); Q_ASSERT(action);
        if (action) action->setChecked(true);
    }
    else
    {
        mGraphicsView->setScene(nullptr);
    }

    // active board has changed!
    emit activeBoardChanged(mActiveBoardIndex, index);
    mActiveBoardIndex = index;
    return true;
}

/*****************************************************************************************
 *  General Methods
 ****************************************************************************************/

void BoardEditor::abortAllCommands() noexcept
{
}

/*****************************************************************************************
 *  Inherited Methods
 ****************************************************************************************/

void BoardEditor::closeEvent(QCloseEvent* event)
{
    if (!mProject.windowIsAboutToClose(this))
        event->ignore();
    else
        QMainWindow::closeEvent(event);
}

/*****************************************************************************************
 *  Public Slots
 ****************************************************************************************/

void BoardEditor::boardAdded(int newIndex)
{
    Board* board = mProject.getBoardByIndex(newIndex);
    Q_ASSERT(board); if (!board) return;

    QAction* actionBefore = mBoardListActions.value(newIndex-1);
    //if (!actionBefore) actionBefore = TODO
    QAction* newAction = new QAction(board->getName(), this);
    newAction->setCheckable(true);
    mUi->menuBoard->insertAction(actionBefore, newAction);
    mBoardListActions.insert(newIndex, newAction);
    mBoardListActionGroup.addAction(newAction);
}

void BoardEditor::boardRemoved(int oldIndex)
{
    QAction* action = mBoardListActions.takeAt(oldIndex); Q_ASSERT(action);
    mBoardListActionGroup.removeAction(action);
    delete action;
}

/*****************************************************************************************
 *  Actions
 ****************************************************************************************/

void BoardEditor::on_actionProjectClose_triggered()
{
    mProject.close(this);
}

void BoardEditor::on_actionNewBoard_triggered()
{
    bool ok = false;
    QString name = QInputDialog::getText(this, tr("Add board"),
                       tr("Choose a name:"), QLineEdit::Normal, tr("default"), &ok);
    if (!ok) return;

    try
    {
        CmdBoardAdd* cmd = new CmdBoardAdd(mProject, name);
        mProject.getUndoStack().execCmd(cmd);
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.getUserMsg());
    }
}

void BoardEditor::on_actionUndo_triggered()
{
    try
    {
        mProject.getUndoStack().undo();
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("Undo failed"), e.getUserMsg());
    }
}

void BoardEditor::on_actionRedo_triggered()
{
    try
    {
        mProject.getUndoStack().redo();
    }
    catch (Exception& e)
    {
        QMessageBox::critical(this, tr("Redo failed"), e.getUserMsg());
    }
}

void BoardEditor::on_actionGrid_triggered()
{
    GridSettingsDialog dialog(*mGridProperties, this);
    connect(&dialog, &GridSettingsDialog::gridPropertiesChanged,
            [this](const GridProperties& grid)
            {   *mGridProperties = grid;
                mGraphicsView->setGridProperties(grid);
            });
    if (dialog.exec())
    {
        //foreach (Schematic* schematic, mProject.getSchematics())
        //    schematic->setGridProperties(*mGridProperties);
        mProject.setModifiedFlag();
    }
}

void BoardEditor::on_actionExportAsPdf_triggered()
{
    try
    {
        QString filename = QFileDialog::getSaveFileName(this, tr("PDF Export"),
                                                        QDir::homePath(), "*.pdf");
        if (filename.isEmpty()) return;
        if (!filename.endsWith(".pdf")) filename.append(".pdf");
        //FilePath filepath(filename);
        //mProject.exportSchematicsAsPdf(filepath); // this method can throw an exception
    }
    catch (Exception& e)
    {
        QMessageBox::warning(this, tr("Error"), e.getUserMsg());
    }
}

void BoardEditor::on_actionProjectProperties_triggered()
{
    ProjectPropertiesEditorDialog dialog(mProject, this);
    dialog.exec();
}

void BoardEditor::boardListActionGroupTriggered(QAction* action)
{
    setActiveBoardIndex(mBoardListActions.indexOf(action));
}

/*****************************************************************************************
 *  Private Methods
 ****************************************************************************************/

bool BoardEditor::graphicsViewEventHandler(QEvent* event)
{
    // TODO
    Q_UNUSED(event);
    return false;
}

/*****************************************************************************************
 *  End of File
 ****************************************************************************************/

} // namespace project
