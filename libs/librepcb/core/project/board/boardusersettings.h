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

#ifndef LIBREPCB_CORE_BOARDUSERSETTINGS_H
#define LIBREPCB_CORE_BOARDUSERSETTINGS_H

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "../../serialization/serializableobject.h"
#include "../../types/uuid.h"

#include <QtCore>

/*******************************************************************************
 *  Namespace / Forward Declarations
 ******************************************************************************/
namespace librepcb {

class Board;
class GraphicsLayerStackAppearanceSettings;

/*******************************************************************************
 *  Class BoardUserSettings
 ******************************************************************************/

/**
 * @brief The BoardUserSettings class
 */
class BoardUserSettings final : public QObject, public SerializableObject {
  Q_OBJECT

public:
  // Constructors / Destructor
  BoardUserSettings() = delete;
  BoardUserSettings(const BoardUserSettings& other) = delete;
  explicit BoardUserSettings(Board& board) noexcept;
  BoardUserSettings(Board& board, const BoardUserSettings& other) noexcept;
  BoardUserSettings(Board& board, const SExpression& node,
                    const Version& fileFormat);
  ~BoardUserSettings() noexcept;

  // Getters
  bool getPlaneVisibility(const Uuid& uuid) const noexcept {
    return mPlanesVisibility.value(uuid, true);
  }

  // Setters
  void setPlaneVisibility(const Uuid& uuid, bool visible) noexcept {
    mPlanesVisibility[uuid] = visible;
  }

  // General Methods
  void resetPlanesVisibility() noexcept { mPlanesVisibility.clear(); }

  /// @copydoc ::librepcb::SerializableObject::serialize()
  void serialize(SExpression& root) const override;

  // Operator Overloadings
  BoardUserSettings& operator=(const BoardUserSettings& rhs) = delete;

private:  // Methods
  // General
  Board& mBoard;
  QScopedPointer<GraphicsLayerStackAppearanceSettings> mLayerSettings;
  QMap<Uuid, bool> mPlanesVisibility;
};

/*******************************************************************************
 *  End of File
 ******************************************************************************/

}  // namespace librepcb

#endif
