/*
 * LibrePCB - Professional EDA for everyone!
 * Copyright (C) 2013 LibrePCB Developers, see AUTHORS.md for contributors.
 * https://librepcb.org/
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

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "cmdschematicedit.h"

#include "../schematic.h"

#include <QtCore>

/*******************************************************************************
 *  Namespace
 ******************************************************************************/
namespace librepcb {
namespace project {

/*******************************************************************************
 *  Constructors / Destructor
 ******************************************************************************/

CmdSchematicEdit::CmdSchematicEdit(Schematic& schematic) noexcept
  : UndoCommand(tr("Edit sheet properties")),
    mSchematic(schematic),
    mOldName(schematic.getName()),
    mNewName(mOldName) {
}

CmdSchematicEdit::~CmdSchematicEdit() noexcept {
}

/*******************************************************************************
 *  Setters
 ******************************************************************************/

void CmdSchematicEdit::setName(const ElementName& name) noexcept {
  Q_ASSERT(!wasEverExecuted());
  mNewName = name;
}

/*******************************************************************************
 *  Inherited from UndoCommand
 ******************************************************************************/

bool CmdSchematicEdit::performExecute() {
  performRedo();  // can throw

  if (mNewName != mOldName) return true;
  return false;
}

void CmdSchematicEdit::performUndo() {
  mSchematic.setName(mOldName);
}

void CmdSchematicEdit::performRedo() {
  mSchematic.setName(mNewName);
}

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace project
}  // namespace librepcb
